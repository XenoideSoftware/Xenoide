"""Convert a flat-color raster image to SVG via color quantization + contour tracing.

Pipeline: Pillow (load) -> scikit-image median filter -> scipy K-means quantization ->
scikit-image find_contours per palette color -> primitive-promotion classifier
(circle / ellipse / rect / polygon fallback) -> SVG. Shapes are emitted in
descending area order so the painter's algorithm covers holes via overdraw — no
fill-rule="evenodd" needed.
"""

from __future__ import annotations

import argparse
import math
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Optional
from xml.sax.saxutils import escape

import numpy as np
from PIL import Image
from scipy.cluster.vq import kmeans2
from skimage.filters import median as skimage_median
from skimage.measure import find_contours
from skimage.morphology import binary_closing, binary_opening, disk


# Tagged shape tuples emitted by the classifier:
#   ("polygon", pts: np.ndarray)
#   ("circle",  cx, cy, r)
#   ("ellipse", cx, cy, rx, ry)
#   ("rect",    x, y, w, h)
Shape = tuple

# After the (row, col) -> (x, y) column swap in extract_shapes_for_color, outer
# rings from skimage.find_contours have positive signed area, holes negative.
OUTER_SIGN = 1.0


@dataclass(frozen=True)
class PromoteConfig:
    order: tuple[str, ...]
    circle_tol: float
    circle_min_r: float
    ellipse_tol: float
    ellipse_min_r: float
    rect_tol: float
    rect_min_size: float


def load_image(path: Path) -> np.ndarray:
    with Image.open(path) as src:
        return np.asarray(src.convert("RGB"))


def preprocess(img: np.ndarray, blur: int) -> np.ndarray:
    if blur <= 0:
        return img
    if blur % 2 == 0:
        blur += 1
    radius = (blur - 1) // 2
    if radius == 0:
        return img
    footprint = disk(radius)
    out = np.empty_like(img)
    for c in range(3):
        out[..., c] = skimage_median(img[..., c], footprint=footprint)
    return out


def quantize(img: np.ndarray, k: int) -> tuple[np.ndarray, np.ndarray]:
    h, w, _ = img.shape
    samples = img.reshape(-1, 3).astype(np.float64)
    centers, labels = kmeans2(samples, k, minit="++", seed=0)
    palette = np.clip(centers, 0, 255).astype(np.uint8)
    quantized = palette[labels].reshape(h, w, 3)
    return quantized, palette


# --- Geometry helpers ---------------------------------------------------------


def signed_area(pts: np.ndarray) -> float:
    if len(pts) < 3:
        return 0.0
    x = pts[:, 0]
    y = pts[:, 1]
    return 0.5 * float(np.sum(x * np.roll(y, -1) - np.roll(x, -1) * y))


def polygon_area(pts: np.ndarray) -> float:
    return abs(signed_area(pts))


def is_outer(pts: np.ndarray) -> bool:
    return signed_area(pts) * OUTER_SIGN > 0.0


# --- Douglas-Peucker ----------------------------------------------------------


def _dp_open(pts: np.ndarray, epsilon: float) -> np.ndarray:
    n = len(pts)
    if n < 3:
        return pts
    keep = np.zeros(n, dtype=bool)
    keep[0] = keep[-1] = True
    eps2 = float(epsilon) * float(epsilon)
    stack: list[tuple[int, int]] = [(0, n - 1)]
    while stack:
        s, e = stack.pop()
        if e - s < 2:
            continue
        a = pts[s]
        b = pts[e]
        ab = b - a
        ab_len2 = float(ab @ ab)
        seg = pts[s + 1:e]
        if ab_len2 == 0.0:
            d2 = np.sum((seg - a) ** 2, axis=1)
        else:
            t = ((seg - a) @ ab) / ab_len2
            proj = a + np.clip(t, 0.0, 1.0)[:, None] * ab
            d2 = np.sum((seg - proj) ** 2, axis=1)
        idx = int(np.argmax(d2))
        if d2[idx] > eps2:
            k = s + 1 + idx
            keep[k] = True
            stack.append((s, k))
            stack.append((k, e))
    return pts[keep]


def douglas_peucker(pts: np.ndarray, epsilon: float) -> np.ndarray:
    n = len(pts)
    if n < 3:
        return pts
    if np.allclose(pts[0], pts[-1]):
        d2 = np.sum((pts - pts[0]) ** 2, axis=1)
        anchor = int(np.argmax(d2))
        if anchor == 0 or anchor == n - 1:
            return pts
        first = _dp_open(pts[: anchor + 1], epsilon)
        second = _dp_open(pts[anchor:], epsilon)
        return np.vstack([first[:-1], second])
    return _dp_open(pts, epsilon)


# --- Primitive classifiers ----------------------------------------------------


def try_circle(pts: np.ndarray, tolerance: float, min_radius: float) -> Optional[Shape]:
    if len(pts) < 4:
        return None
    x = pts[:, 0]
    y = pts[:, 1]
    A = np.column_stack([x, y, np.ones_like(x)])
    b = -(x * x + y * y)
    sol, *_ = np.linalg.lstsq(A, b, rcond=None)
    D, E, F = sol
    cx = -D / 2.0
    cy = -E / 2.0
    discriminant = cx * cx + cy * cy - F
    if discriminant <= 0.0:
        return None
    r = math.sqrt(discriminant)
    if r < min_radius:
        return None
    dists = np.sqrt((x - cx) ** 2 + (y - cy) ** 2)
    max_dev = float(np.max(np.abs(dists - r)))
    if max_dev / r > tolerance:
        return None
    return ("circle", float(cx), float(cy), float(r))


def try_ellipse(pts: np.ndarray, tolerance: float, min_radius: float) -> Optional[Shape]:
    if len(pts) < 4:
        return None
    xmin, ymin = pts.min(axis=0)
    xmax, ymax = pts.max(axis=0)
    rx = (xmax - xmin) / 2.0
    ry = (ymax - ymin) / 2.0
    if min(rx, ry) < min_radius:
        return None
    cx = (xmin + xmax) / 2.0
    cy = (ymin + ymax) / 2.0
    nx = (pts[:, 0] - cx) / rx
    ny = (pts[:, 1] - cy) / ry
    residual = np.abs(nx * nx + ny * ny - 1.0)
    if float(np.max(residual)) > tolerance:
        return None
    return ("ellipse", float(cx), float(cy), float(rx), float(ry))


def try_rect(pts: np.ndarray, tolerance: float, min_size: float) -> Optional[Shape]:
    if len(pts) < 4:
        return None
    xmin, ymin = pts.min(axis=0)
    xmax, ymax = pts.max(axis=0)
    w = xmax - xmin
    h = ymax - ymin
    if min(w, h) < min_size:
        return None
    dx_min = pts[:, 0] - xmin
    dx_max = xmax - pts[:, 0]
    dy_min = pts[:, 1] - ymin
    dy_max = ymax - pts[:, 1]
    edge_dist = np.minimum(np.minimum(dx_min, dx_max), np.minimum(dy_min, dy_max))
    if float(np.max(edge_dist)) / min(w, h) > tolerance:
        return None
    return ("rect", float(xmin), float(ymin), float(w), float(h))


_PROMOTERS: dict[str, Callable[[np.ndarray, float, float], Optional[Shape]]] = {
    "circle": try_circle,
    "ellipse": try_ellipse,
    "rect": try_rect,
}


def _promoter_args(name: str, cfg: PromoteConfig) -> tuple[float, float]:
    if name == "circle":
        return cfg.circle_tol, cfg.circle_min_r
    if name == "ellipse":
        return cfg.ellipse_tol, cfg.ellipse_min_r
    if name == "rect":
        return cfg.rect_tol, cfg.rect_min_size
    raise ValueError(f"unknown promoter: {name}")


def classify_outer(pts: np.ndarray, cfg: PromoteConfig, epsilon: float) -> Shape:
    for name in cfg.order:
        promoter = _PROMOTERS[name]
        result = promoter(pts, *_promoter_args(name, cfg))
        if result is not None:
            return result
    return ("polygon", douglas_peucker(pts, epsilon))


# --- Pipeline -----------------------------------------------------------------


def extract_shapes_for_color(
    quantized: np.ndarray,
    color: np.ndarray,
    min_area: float,
    epsilon: float,
    smooth: bool,
    cfg: PromoteConfig,
) -> list[tuple[float, Shape]]:
    mask = np.all(quantized == color, axis=-1)
    if smooth:
        mask = binary_opening(mask, footprint=disk(1))
        mask = binary_closing(mask, footprint=disk(1))

    padded = np.pad(mask.astype(np.float32), 1, constant_values=0.0)
    raw_contours = find_contours(padded, level=0.5)

    shapes: list[tuple[float, Shape]] = []
    for contour in raw_contours:
        # find_contours returns (row, col); convert to (x, y) and undo padding.
        pts = np.column_stack([contour[:, 1] - 1.0, contour[:, 0] - 1.0])
        # Holes are skipped: the painter's algorithm covers them via overdraw of
        # smaller, on-top shapes from other layers.
        if not is_outer(pts):
            continue
        area = polygon_area(pts)
        if area < min_area:
            continue
        shapes.append((area, classify_outer(pts, cfg, epsilon)))
    return shapes


# --- SVG emission -------------------------------------------------------------


def contour_to_path_d(pts: np.ndarray) -> str:
    if len(pts) >= 2 and np.allclose(pts[0], pts[-1]):
        pts = pts[:-1]
    if len(pts) < 3:
        return ""
    parts = [f"M{pts[0, 0]:.1f} {pts[0, 1]:.1f}"]
    for p in pts[1:]:
        parts.append(f"L{p[0]:.1f} {p[1]:.1f}")
    parts.append("Z")
    return " ".join(parts)


def rgb_to_hex(color: np.ndarray) -> str:
    r, g, b = (int(c) for c in color)
    return f"#{r:02x}{g:02x}{b:02x}"


def emit_shape(shape: Shape, fill: str) -> str:
    tag = shape[0]
    if tag == "polygon":
        d = contour_to_path_d(shape[1])
        if not d:
            return ""
        return f'<path fill="{fill}" d="{d}"/>'
    if tag == "circle":
        _, cx, cy, r = shape
        return f'<circle cx="{cx:.1f}" cy="{cy:.1f}" r="{r:.1f}" fill="{fill}"/>'
    if tag == "ellipse":
        _, cx, cy, rx, ry = shape
        return f'<ellipse cx="{cx:.1f}" cy="{cy:.1f}" rx="{rx:.1f}" ry="{ry:.1f}" fill="{fill}"/>'
    if tag == "rect":
        _, x, y, w, h = shape
        return f'<rect x="{x:.1f}" y="{y:.1f}" width="{w:.1f}" height="{h:.1f}" fill="{fill}"/>'
    raise ValueError(f"unknown shape tag: {tag}")


def build_svg(
    width: int,
    height: int,
    shapes: list[tuple[float, np.ndarray, Shape]],
) -> str:
    parts: list[str] = [
        f'<svg xmlns="http://www.w3.org/2000/svg" '
        f'viewBox="0 0 {width} {height}" width="{width}" height="{height}" '
        f'shape-rendering="geometricPrecision">'
    ]
    for _area, color, shape in shapes:
        fill = escape(rgb_to_hex(color))
        element = emit_shape(shape, fill)
        if element:
            parts.append(element)
    parts.append("</svg>")
    return "\n".join(parts) + "\n"


def convert(
    input_path: Path,
    output_path: Path,
    colors: int,
    epsilon: float,
    min_area: float,
    blur: int,
    smooth: bool,
    cfg: PromoteConfig,
) -> None:
    img = load_image(input_path)
    img = preprocess(img, blur)
    quantized, palette = quantize(img, colors)

    height, width = img.shape[:2]
    all_shapes: list[tuple[float, np.ndarray, Shape]] = []
    for color in palette:
        for area, shape in extract_shapes_for_color(
            quantized, color, min_area, epsilon, smooth, cfg
        ):
            all_shapes.append((area, color, shape))

    # Painter's algorithm: largest first (back), smallest last (front).
    all_shapes.sort(key=lambda t: t[0], reverse=True)

    svg = build_svg(width, height, all_shapes)
    output_path.write_text(svg, encoding="utf-8")


# --- CLI ----------------------------------------------------------------------


def parse_promote(spec: str) -> tuple[str, ...]:
    spec = spec.strip().lower()
    if spec in ("", "none"):
        return ()
    parts = tuple(p.strip() for p in spec.split(",") if p.strip())
    unknown = [p for p in parts if p not in _PROMOTERS]
    if unknown:
        allowed = ", ".join(_PROMOTERS.keys())
        raise argparse.ArgumentTypeError(
            f"unknown primitive(s) in --promote: {', '.join(unknown)}. "
            f"Allowed: {allowed}, or 'none' to disable."
        )
    return parts


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Convert a flat-color raster image to SVG.")
    parser.add_argument("input", type=Path, help="Input image (PNG/JPEG/BMP).")
    parser.add_argument(
        "-o", "--output", type=Path, default=None,
        help="Output SVG path (default: <input>.svg).",
    )
    parser.add_argument("--colors", type=int, default=8, help="Palette size for K-means.")
    parser.add_argument("--epsilon", type=float, default=1.5, help="Douglas-Peucker tolerance (px).")
    parser.add_argument("--min-area", type=float, default=10.0, help="Drop contours below this area (px^2).")
    parser.add_argument("--blur", type=int, default=3, help="Median filter kernel size (0 disables).")
    parser.add_argument("--smooth", action="store_true", help="Apply morphological open+close per color mask.")

    parser.add_argument("--circle-tolerance", type=float, default=0.05,
                        help="Max radial deviation / radius for <circle> promotion.")
    parser.add_argument("--circle-min-radius", type=float, default=2.0,
                        help="Skip <circle> promotion below this radius (px).")
    parser.add_argument("--ellipse-tolerance", type=float, default=0.05,
                        help="Max normalized residual for axis-aligned <ellipse> promotion.")
    parser.add_argument("--ellipse-min-radius", type=float, default=2.0,
                        help="Skip <ellipse> promotion when min(rx, ry) is below this (px).")
    parser.add_argument("--rect-tolerance", type=float, default=0.05,
                        help="Max bbox-edge deviation / min(w, h) for <rect> promotion.")
    parser.add_argument("--rect-min-size", type=float, default=4.0,
                        help="Skip <rect> promotion when min(w, h) is below this (px).")
    parser.add_argument("--promote", type=parse_promote,
                        default=("circle", "ellipse", "rect"),
                        help='Comma-separated primitives to promote, in priority order. '
                             'Allowed: circle, ellipse, rect. "none" disables promotion.')
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    output = args.output or args.input.with_suffix(".svg")

    if args.colors < 2:
        print("error: --colors must be >= 2", file=sys.stderr)
        return 2

    cfg = PromoteConfig(
        order=args.promote,
        circle_tol=args.circle_tolerance,
        circle_min_r=args.circle_min_radius,
        ellipse_tol=args.ellipse_tolerance,
        ellipse_min_r=args.ellipse_min_radius,
        rect_tol=args.rect_tolerance,
        rect_min_size=args.rect_min_size,
    )

    convert(
        input_path=args.input,
        output_path=output,
        colors=args.colors,
        epsilon=args.epsilon,
        min_area=args.min_area,
        blur=args.blur,
        smooth=args.smooth,
        cfg=cfg,
    )
    print(f"wrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

# img2svg

Convert a flat-color raster image (PNG/JPEG/BMP) to SVG using scikit-image and Pillow.

Designed for **simple colored contour designs** — logos, flat icons, coloring-book art,
diagrams. Not suitable for photographs, gradients, or images with anti-aliased noisy
edges.

## Files

- `img2svg.py` — CLI entry point and conversion pipeline.
- `make_sample.py` — generates `sample.png`, a small synthetic flat-color test image.
- `sample.png` — committed test input (re-runnable via `make_sample.py`).
- `requirements.txt` — Python dependencies (Pillow, numpy, scipy, scikit-image).

## Setup

### Windows (PowerShell)

```powershell
cd tools/img2svg
python -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install -r requirements.txt
```

### Unix (bash)

```bash
cd tools/img2svg
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## Run

Basic conversion:

```bash
python img2svg.py sample.png -o sample.svg
```

Open `sample.svg` in any browser to view it.

### Useful flags

```bash
# Smaller palette (fewer color regions)
python img2svg.py sample.png --colors 4

# Looser polygon approximation (chunkier paths, smaller file)
python img2svg.py sample.png --epsilon 5

# Drop small specks below 200 px^2
python img2svg.py sample.png --min-area 200

# Smooth color-region edges before tracing
python img2svg.py sample.png --smooth

# Disable median blur on input
python img2svg.py sample.png --blur 0
```

### Primitive promotion

Outer contours are tested against a list of native-SVG primitive classifiers;
the first match wins, otherwise the contour falls through to a polygon `<path>`.
Default priority order is `circle,ellipse,rect`. Detection is **axis-aligned only**
for `<rect>` and `<ellipse>` (rotated shapes fall through to `<path>`).

```bash
# Disable promotion entirely (everything becomes a polygon path)
python img2svg.py sample.png --promote none

# Only attempt circles
python img2svg.py sample.png --promote circle

# Reorder priority (rect first; circles still preferred where they fit)
python img2svg.py sample.png --promote circle,rect
```

### Full flag list

| Flag | Default | Purpose |
|---|---|---|
| `INPUT` | — | Path to PNG/JPEG/BMP. |
| `-o, --output` | `<input>.svg` | Output SVG path. |
| `--colors` | `8` | K for K-means color quantization (must be >= 2). |
| `--epsilon` | `1.5` | Douglas-Peucker tolerance in pixels. Higher = simpler paths. |
| `--min-area` | `10` | Drop contours with area below this (px^2). |
| `--blur` | `3` | Median filter kernel size before quantization (`0` disables). |
| `--smooth` | off | Apply morphological open+close on each color mask. |
| `--circle-tolerance` | `0.05` | Max radial deviation / radius for `<circle>` promotion. |
| `--circle-min-radius` | `2.0` | Skip `<circle>` promotion below this radius (px). |
| `--ellipse-tolerance` | `0.05` | Max normalized residual for axis-aligned `<ellipse>` promotion. |
| `--ellipse-min-radius` | `2.0` | Skip `<ellipse>` promotion when `min(rx, ry)` is below this (px). |
| `--rect-tolerance` | `0.05` | Max bbox-edge deviation / `min(w, h)` for `<rect>` promotion. |
| `--rect-min-size` | `4.0` | Skip `<rect>` promotion when `min(w, h)` is below this (px). |
| `--promote` | `circle,ellipse,rect` | Comma-separated primitives in priority order. `none` (or empty) disables promotion. |

## Pipeline

1. **Preprocess** — optional per-channel median filter (`skimage.filters.median`).
2. **Quantize** — `scipy.cluster.vq.kmeans2` (k-means++ init) reduces the image to
   a small palette.
3. **Trace** — for each palette color: build a binary mask, optional morphological
   open+close (`skimage.morphology`), then `skimage.measure.find_contours` extracts
   subpixel iso-contours. Hole contours (negative signed area after axis swap) are
   discarded; the painter's algorithm in step 5 handles them via overdraw.
4. **Classify** — each outer contour is tested against `try_circle`, `try_ellipse`,
   `try_rect` in `--promote` order. First match wins; otherwise the contour is
   simplified with Douglas-Peucker and emitted as a polygon path.
5. **Emit** — every shape is emitted as its own SVG element, in **descending area
   order across all colors** (painter's algorithm). A larger background shape is
   drawn first; smaller shapes that should appear "inside" it are drawn later and
   cover what they need to. No `fill-rule="evenodd"` needed.

Adding a new classifier (e.g. rotated rect, rounded rect) is a one-function edit:
write `try_<shape>(pts, tol, min_size) -> Shape | None`, register it in the
`_PROMOTERS` dict, and extend `emit_shape` and `_promoter_args`.

## Limitations / future work

- Output uses straight polygon segments only (`M`/`L`/`Z`) for the polygon
  fallback. Bezier-curve smoothing is not implemented yet.
- `<rect>` and `<ellipse>` detection are axis-aligned only; rotated variants
  fall through to `<path>`.
- Photographs and gradient regions will produce a faceted/posterized result
  rather than a faithful vectorization — that is by design.
- Colors are picked by K-means on raw RGB, with no perceptual color-space
  conversion.

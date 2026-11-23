"""Generate a small synthetic flat-color test image for img2svg."""

from pathlib import Path

from PIL import Image, ImageDraw


def make_sample(output: Path = Path(__file__).with_name("sample.png"), size: int = 256) -> Path:
    bg = (245, 240, 230)  # cream
    img = Image.new("RGB", (size, size), bg)
    draw = ImageDraw.Draw(img)

    draw.rectangle([20, 20, 110, 110], fill=(200, 80, 60))            # red square
    draw.ellipse([130, 20, 230, 120], fill=(80, 170, 60))              # green disc
    draw.ellipse([162, 52, 198, 88], fill=bg)                          # hole in disc
    draw.polygon([(40, 230), (120, 140), (200, 230)], fill=(40, 120, 200))  # blue triangle
    draw.rectangle([150, 160, 235, 220], fill=(230, 200, 40))          # yellow box
    draw.ellipse([180, 178, 204, 202], fill=(130, 60, 90))             # purple dot

    output.parent.mkdir(parents=True, exist_ok=True)
    img.save(output)
    return output


if __name__ == "__main__":
    path = make_sample()
    print(f"wrote {path}")

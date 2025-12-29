#!/usr/bin/env python3
"""
Generate a fixed-cell bitmap for digits/colon and convert to a CFB font.

Usage:
  python3 tools/gen_lato_digits_font.py --font fonts/Lato-Bold.ttf
  python3 tools/gen_lato_digits_font.py --font fonts/Lato-Bold.ttf --size 60
  python3 tools/gen_lato_digits_font.py --font fonts/Lato-Bold.ttf --zephyr-base /path/to/zephyr
example:
  python3 tools/gen_lato_digits_font.py --font fonts/BarlowCondensed-Regular.ttf --size 70 --zephyr-base /opt/nordic/ncs/v3.2.1/zephyr

Requirements:
  - PIL/Pillow (python3 -m pip install pillow)
  - Zephyr SDK scripts in place:
      $ZEPHYR_BASE/scripts/build/gen_cfb_font_header.py
  - Input font provided via --font

Outputs:
  - src/cfb_font_digits_<width><height>.c
"""

import argparse
import math
import os
import subprocess

from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate digits/colon CFB font from a TTF/OTF file."
    )
    parser.add_argument(
        "--font",
        required=True,
        help="Path to TTF/OTF font file (e.g., fonts/Lato-Bold.ttf)",
    )
    parser.add_argument(
        "--size",
        type=int,
        default=60,
        help="Point size for font rendering (default: 60)",
    )
    parser.add_argument(
        "--zephyr-base",
        help="Path to Zephyr base (default: $ZEPHYR_BASE)",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    font_path = args.font
    size = args.size
    chars = "0123456789:"

    font = ImageFont.truetype(font_path, size)

    bboxes = [font.getbbox(ch) for ch in chars]
    min_top = min(b[1] for b in bboxes)
    max_bottom = max(b[3] for b in bboxes)
    max_width = max(b[2] - b[0] for b in bboxes)

    height = int(math.ceil((max_bottom - min_top) / 8.0) * 8)
    width = max_width

    out_png = f"src/digits_{width}x{height}.png"
    out_c = f"src/cfb_font_digits_{width}{height}.c"

    img = Image.new("1", (width * len(chars), height), 1)
    draw = ImageDraw.Draw(img)

    for idx, ch in enumerate(chars):
        left, top, right, bottom = font.getbbox(ch)
        glyph_w = right - left
        x = idx * width + (width - glyph_w) // 2 - left
        y = -min_top
        draw.text((x, y), ch, font=font, fill=0)

    img.save(out_png)

    zephyr_base = args.zephyr_base or os.environ.get("ZEPHYR_BASE")
    if not zephyr_base:
        raise SystemExit("ZEPHYR_BASE is not set (use --zephyr-base)")

    gen_script = os.path.join(zephyr_base, "scripts", "build", "gen_cfb_font_header.py")
    cmd = [
        gen_script,
        "-i", out_png,
        "-t", "image",
        "-x", str(width),
        "-y", str(height),
        "--first", "48",
        "--last", "58",
        "-n", "lato_bold_digits",
        "-o", out_c,
    ]
    subprocess.check_call(cmd)
    print(f"Generated: {out_c}")


if __name__ == "__main__":
    main()

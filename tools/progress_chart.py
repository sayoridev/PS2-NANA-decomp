#!/usr/bin/env python3
"""Render progress.yaml to a cute dark-mode PNG dashboard."""

from pathlib import Path
import math
import sys

import yaml
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
PROGRESS_YAML = ROOT / "progress.yaml"
OUTPUT = ROOT / "docs" / "progress.png"

W, H = 1200, 760
BG = "#17111f"
CARD = "#211a2d"
CARD_2 = "#2a2138"
TEXT = "#f8fafc"
MUTED = "#aaa4ba"
PINK = "#f9a8d4"
LAV = "#c4b5fd"
MINT = "#86efac"
GOLD = "#fde68a"
RED = "#fda4af"


def load_font(size, bold=False, mono=False):
    candidates = []
    if mono:
        candidates += [
            "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",
        ]
    elif bold:
        candidates.append("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf")
    candidates.append("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")
    for path in candidates:
        try:
            return ImageFont.truetype(path, size)
        except OSError:
            pass
    return ImageFont.load_default()


def fit_text(draw, text, font, max_width):
    if draw.textlength(text, font=font) <= max_width:
        return text
    ell = "…"
    while text and draw.textlength(text + ell, font=font) > max_width:
        text = text[:-1]
    return text + ell


def draw_cat(draw, x, y):
    # Tiny pixel-cat face, matching the README tone.
    draw.rounded_rectangle((x, y + 18, x + 78, y + 76), radius=22, fill=CARD_2, outline=PINK, width=3)
    draw.polygon([(x + 10, y + 28), (x + 18, y), (x + 32, y + 24)], fill=CARD_2, outline=PINK)
    draw.polygon([(x + 46, y + 24), (x + 60, y), (x + 68, y + 28)], fill=CARD_2, outline=PINK)
    draw.ellipse((x + 22, y + 40, x + 30, y + 48), fill=TEXT)
    draw.ellipse((x + 50, y + 40, x + 58, y + 48), fill=TEXT)
    draw.polygon([(x + 38, y + 51), (x + 43, y + 51), (x + 40, y + 55)], fill=PINK)
    draw.arc((x + 28, y + 49, x + 39, y + 61), 15, 165, fill=MUTED, width=2)
    draw.arc((x + 42, y + 49, x + 53, y + 61), 15, 165, fill=MUTED, width=2)


def bar(draw, x, y, w, h, label, value, maximum, color):
    draw.text((x, y), label, font=load_font(20), fill=MUTED)
    draw.rounded_rectangle((x, y + 30, x + w, y + 30 + h), radius=h // 2, fill="#15101c")
    pct = 0 if maximum <= 0 else max(0, min(1, value / maximum))
    fill_w = max(h, int((w - 4) * pct))
    draw.rounded_rectangle((x + 2, y + 32, x + 2 + fill_w, y + 28 + h), radius=max(1, h // 2 - 2), fill=color)
    draw.text((x + w + 14, y + 22), f"{value:,} / {maximum:,}", font=load_font(20, mono=True), fill=TEXT)


def status_color(status):
    status = str(status).lower()
    if status in ("completed", "complete", "done"):
        return MINT
    if status in ("in_progress", "ongoing", "active"):
        return GOLD
    return LAV


def main():
    data = yaml.safe_load(PROGRESS_YAML.read_text(encoding="utf-8"))
    funcs = data.get("functions", {})
    total = int(funcs.get("total", 0))
    identified = int(funcs.get("identified", 0))
    decompiled = int(funcs.get("decompiled", 0))
    matched = int(funcs.get("matched", 0))
    remaining = int(funcs.get("remaining", total - decompiled))

    img = Image.new("RGB", (W, H), BG)
    d = ImageDraw.Draw(img)

    # Background ornaments.
    d.ellipse((900, -90, 1250, 260), fill="#22162d")
    d.ellipse((-120, 570, 250, 930), fill="#1d1527")

    # Header.
    d.text((52, 42), "PS2-NANA-decomp", font=load_font(44, bold=True), fill=TEXT)
    d.text((54, 96), "progress dashboard · generated from progress.yaml", font=load_font(20), fill=MUTED)
    d.text((54, 128), f"updated {data.get('updated', 'unknown')} · phase {data.get('phase', '?')}", font=load_font(18, mono=True), fill=PINK)
    draw_cat(d, 1010, 42)

    # Main cards.
    d.rounded_rectangle((52, 178, 574, 470), radius=28, fill=CARD, outline="#372946", width=2)
    d.rounded_rectangle((598, 178, 1148, 470), radius=28, fill=CARD, outline="#372946", width=2)

    d.text((82, 206), "functions", font=load_font(26, bold=True), fill=TEXT)
    bar(d, 82, 248, 330, 22, "identified", identified, total, LAV)
    bar(d, 82, 316, 330, 22, "reconstructed", decompiled, total, PINK)
    bar(d, 82, 384, 330, 22, "matched", matched, total, MINT)

    d.text((628, 206), "summary", font=load_font(26, bold=True), fill=TEXT)
    summary = [
        ("estimated total", f"{total:,}"),
        ("clean reconstructed", f"{decompiled:,}"),
        ("binary matched", f"{matched:,}"),
        ("remaining", f"{remaining:,}"),
    ]
    yy = 250
    for key, val in summary:
        d.text((628, yy), key, font=load_font(20), fill=MUTED)
        d.text((900, yy - 3), val, font=load_font(24, bold=True, mono=True), fill=TEXT)
        yy += 48

    # Milestone card.
    d.rounded_rectangle((52, 500, 1148, 704), radius=28, fill=CARD_2, outline="#3b2d4b", width=2)
    d.text((82, 526), "milestones", font=load_font(26, bold=True), fill=TEXT)
    milestones = data.get("milestones", [])
    x = 82
    y = 572
    max_x = 1080
    chip_font = load_font(17)
    for ms in milestones:
        name = fit_text(d, str(ms.get("name", "unknown")), chip_font, 260)
        status = str(ms.get("status", "pending"))
        label = f"{name} · {status}"
        tw = int(d.textlength(label, font=chip_font)) + 28
        if x + tw > max_x:
            x = 82
            y += 44
        color = status_color(status)
        d.rounded_rectangle((x, y, x + tw, y + 30), radius=15, fill="#181221", outline=color, width=2)
        d.text((x + 14, y + 5), label, font=chip_font, fill=color)
        x += tw + 12

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    img.save(OUTPUT, optimize=True)
    print(f"wrote {OUTPUT.relative_to(ROOT)}")


if __name__ == "__main__":
    main()

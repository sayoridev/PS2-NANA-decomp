#!/usr/bin/env python3
"""Generate the decompilation progress chart (docs/progress.svg) from progress.yaml.

Usage:
    python tools/progress_chart.py

Reads progress.yaml from the repository root and writes docs/progress.svg.
No third-party dependencies beyond PyYAML (pacman: python-yaml).
The SVG is committed to the repo; a GitHub Action regenerates it automatically
on every push that touches progress.yaml or this script.
"""

import datetime
import sys
from pathlib import Path

try:
    import yaml
except ImportError:
    sys.exit("PyYAML is required: sudo pacman -S python-yaml")

ROOT = Path(__file__).resolve().parent.parent
PROGRESS = ROOT / "progress.yaml"
OUT = ROOT / "docs" / "progress.svg"

STATUS_COLORS = {
    "completed": "#4caf50",
    "in_progress": "#ffc107",
    "pending": "#9e9e9e",
    "ongoing": "#2196f3",
    "blocked": "#f44336",
}


def bar_row(label, value, maximum, color, y):
    width = 340
    fill = int(width * value / maximum) if maximum else 0
    return f"""
  <text x='16' y='{y + 13}' class='lbl'>{label}</text>
  <rect x='150' y='{y}' width='{width}' height='18' rx='3' fill='#2b313c'/>
  <rect x='150' y='{y}' width='{fill}' height='18' rx='3' fill='{color}'/>
  <text x='{150 + width + 8}' y='{y + 13}' class='val'>{value}</text>"""


def main():
    data = yaml.safe_load(PROGRESS.read_text())
    f = data["functions"]
    total = max(f["total"], 1)
    a = data["assets"]

    funcs = [
        ("Identified", f["identified"], "#8bc34a"),
        ("Decompiled", f["decompiled"], "#2196f3"),
        ("Matched", f["matched"], "#4caf50"),
        ("Remaining", f["remaining"], "#616161"),
    ]
    rows = "".join(bar_row(lbl, val, total, col, 90 + i * 26) for i, (lbl, val, col) in enumerate(funcs))
    pct = 100.0 * f["matched"] / total

    miles = []
    for i, m in enumerate(data["milestones"]):
        c = STATUS_COLORS.get(m["status"], "#9e9e9e")
        miles.append(
            f"<circle cx='22' cy='{244 + i * 20}' r='5' fill='{c}'/>"
            f"<text x='36' y='{249 + i * 20}' class='ms'>{m['name']} — {m['status'].replace('_', ' ')}</text>"
        )

    today = datetime.date.today().isoformat()
    svg = f"""<svg xmlns='http://www.w3.org/2000/svg' width='560' height='{260 + 20 * len(miles)}' viewBox='0 0 560 {260 + 20 * len(miles)}' font-family='monospace'>
  <style>
    .t {{ font-size: 17px; font-weight: bold; fill: #e6edf3; }}
    .s {{ font-size: 12px; fill: #8b949e; }}
    .lbl {{ font-size: 12px; fill: #c9d1d9; }}
    .val {{ font-size: 12px; fill: #e6edf3; }}
    .ms {{ font-size: 12px; fill: #c9d1d9; }}
    .pct {{ font-size: 26px; font-weight: bold; fill: #4caf50; }}
  </style>
  <rect width='100%' height='100%' rx='8' fill='#0d1117'/>
  <text x='16' y='30' class='t'>PS2-NANA-decomp — Progress</text>
  <text x='16' y='50' class='s'>Phase {data['phase']} · updated {data.get('updated', today)}</text>
  <text x='530' y='40' text-anchor='end' class='pct'>{pct:.1f}%</text>
  <text x='16' y='74' class='s'>Functions: {f['matched']}/{f['total']} matched</text>
  {rows}
  <text x='16' y='216' class='s'>Assets: {a['identified']} identified · {a['documented']} documented</text>
  <text x='16' y='238' class='s'>Milestones</text>
  {''.join(miles)}
  <text x='16' y='{252 + 20 * len(miles)}' class='s'>Generated from progress.yaml — do not edit by hand.</text>
</svg>"""
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(svg)
    print(f"wrote {OUT.relative_to(ROOT)} "
          f"({f['matched']}/{f['total']} matched, {pct:.1f}%)")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""PS2-NANA-decomp — AnyPS5-style per-function progress map.

Generates docs/progress_map.html: one square per function, colored by
decompilation status, grouped per binary module (main ELF + OVL/*.BIN).

Inputs (repo-relative):
  functions_*.json   rizin 'aflj' output, one file per module
                     (generate: see docs below / Makefile target)
  symbols/functions.txt  status source of truth (TSV v2)

Usage:
  python3 tools/progress_map.py [functions_dir] [out_html]

Module display config: MODULES dict below (base vaddr for raw overlays).
"""
import json
import os
import sys
import html

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SYMBOLS = os.path.join(REPO, "symbols", "functions.txt")

# module key -> (title, base_vaddr or None for ELF-embedded vaddrs)
MODULES = {
    "functions_main":   ("SLPM_659.14 (main)", None),
    "functions_CLOTHE2R": ("OVL clothe2R", 0x00B7F800),
    "functions_ENDINGR":  ("OVL endingR",  0x00B7F800),
    "functions_JOBFND2R": ("OVL jobfnd2R", 0x00B7F800),
    "functions_MYROOMR":  ("OVL myroomR",  0x00B7F800),
    "functions_SHOP2R":   ("OVL shop2R",   0x00B7F800),
    "functions_STATUSR":  ("OVL statusR",  0x00B7F800),
    "functions_WORKINGR": ("OVL workingR", 0x00B7F800),
}

STATUS_COLOR = {
    "VERIFIED":      "#39d353",   # green
    "RECONSTRUCTED": "#26a641",   # green (slightly darker)
    "PARTIAL":       "#e3b341",   # yellow
    "hypothesis":    "#58a6ff",   # blue
}
DEFAULT_COLOR = "#30363d"         # unidentified / not in symbol map

def load_statuses(path):
    """symbols/functions.txt -> {vaddr:int -> (status, name, comment)}"""
    out = {}
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            line = line.rstrip("\n")
            if not line or line.startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) < 4:
                continue
            try:
                addr = int(parts[0], 16)
            except ValueError:
                continue
            status = parts[2].strip()
            if status not in ("RECONSTRUCTED", "PARTIAL", "VERIFIED",
                              "hypothesis"):
                continue
            out[addr] = (status, parts[3], parts[4] if len(parts) > 4 else "")
    return out

def load_module(path, base):
    with open(path, encoding="utf-8") as fh:
        fns = json.load(fh)
    rows = []
    for f in fns:
        off = int(f["offset"])
        vaddr = (off + base) if base is not None else off
        rows.append({"vaddr": vaddr, "name": f["name"],
                     "size": int(f.get("size", 0))})
    rows.sort(key=lambda r: r["vaddr"])
    return rows

def render(module_rows):
    secs = []
    totals = {"all": 0, "done": 0}
    for key, (title, base) in MODULES.items():
        rows = module_rows.get(key)
        if rows is None:
            continue
        n = len(rows)
        done = sum(1 for r in rows if r["status"] != "unidentified")
        totals["all"] += n
        totals["done"] += done
        pct = (100.0 * done / n) if n else 0.0
        cells = []
        for r in rows:
            st = r["status"]
            col = STATUS_COLOR.get(st, DEFAULT_COLOR)
            tip = ("%s\n0x%08X  size 0x%X\n%s%s"
                   % (r["name"], r["vaddr"], r["size"], st,
                      (" — " + r["comment"]) if r["comment"] else ""))
            cells.append('  <div class="cell" style="background:%s" '
                         'title="%s"></div>' % (col, html.escape(tip, True)))
        secs.append(
            '<section><h2>%s <span class="pct">%.2f%% (%d/%d)</span></h2>'
            '<div class="grid">%s</div></section>'
            % (html.escape(title), pct, done, n, "\n".join(cells)))
    allpct = (100.0 * totals["done"] / totals["all"]) if totals["all"] else 0
    return PAGE % {
        "total_pct": "%.2f" % allpct,
        "total_done": totals["done"],
        "total_all": totals["all"],
        "sections": "\n".join(secs),
    }

PAGE = """<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8"/>
<title>PS2-NANA-decomp — function map</title>
<style>
 body{background:#0d1117;color:#c9d1d9;font:14px/1.4 system-ui,sans-serif;margin:24px}
 h1{font-size:22px}
 .legend span{display:inline-block;width:12px;height:12px;margin:0 6px 0 18px;vertical-align:middle}
 section{margin-top:28px}
 h2{font-size:15px;font-weight:600}
 .pct{color:#8b949e;font-weight:400;margin-left:10px}
 .grid{display:flex;flex-wrap:wrap;gap:1px;max-width:1280px}
 .cell{width:10px;height:10px}
</style></head><body>
<h1>PS2-NANA-decomp — per-function progress <span class="pct">%(total_pct)s%% (%(total_done)d/%(total_all)d functions)</span></h1>
<p class="legend">VERIFIED/RECONSTRUCTED<span style="background:#39d353"></span>
PARTIAL<span style="background:#e3b341"></span>
hypothesis<span style="background:#58a6ff"></span>
unidentified<span style="background:#30363d"></span></p>
%(sections)s
</body></html>
"""

# --- static PNG render (for README embedding) -------------------------------
COLORS = {k: tuple(int(c[i:i+2], 16) for i in (1, 3, 5))
          for k, c in (("VERIFIED", "#39d353"), ("RECONSTRUCTED", "#26a641"),
                       ("PARTIAL", "#e3b341"), ("hypothesis", "#58a6ff"))}
PNG_UNKNOWN = (48, 54, 61)
PNG_BG = (13, 17, 23)
PNG_FG = (201, 209, 217)
PNG_SUB = (139, 148, 158)
CELL = 10
GAP = 1
COLS = 96

def _font(sz):
    try:
        from PIL import ImageFont
        for cand in ("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
                     "/usr/share/fonts/dejavu/DejaVuSans.ttf"):
            if os.path.exists(cand):
                return ImageFont.truetype(cand, sz)
    except Exception:
        pass
    return None

def render_png(module_rows, out_png):
    from PIL import Image, ImageDraw
    mods = []
    for k in MODULES:
        if k in module_rows:
            mods.append((k, MODULES[k][0], module_rows[k]))
    H = 40  # header
    for _, _, rows in mods:
        H += 24 + ((len(rows) + COLS - 1) // COLS) * (CELL + GAP) + 10
    W = COLS * (CELL + GAP) + 20
    img = Image.new("RGB", (W, max(H, 60)), PNG_BG)
    dr = ImageDraw.Draw(img)
    f_t, f_s = _font(13), _font(11)
    y = 10
    tot = sum(len(r) for _, _, r in mods)
    done = sum(1 for _, _, r in mods for x in r if x["status"] != "unidentified")
    pct = (100.0 * done / tot) if tot else 0.0
    dr.text((10, y), "PS2-NANA-decomp — function map  %.2f%% (%d/%d)"
            % (pct, done, tot), fill=PNG_FG, font=f_t)
    y += 30
    for key, title, rows in mods:
        n = len(rows)
        dn = sum(1 for x in rows if x["status"] != "unidentified")
        p = (100.0 * dn / n) if n else 0.0
        dr.text((10, y), "%s  %.2f%% (%d/%d)" % (title, p, dn, n),
                fill=PNG_FG, font=f_s)
        y += 20
        x0, yy = 10, y
        for i, x in enumerate(rows):
            col = COLORS.get(x["status"], PNG_UNKNOWN)
            cx = x0 + (i % COLS) * (CELL + GAP)
            cy = yy + (i // COLS) * (CELL + GAP)
            dr.rectangle([cx, cy, cx + CELL - 1, cy + CELL - 1], fill=col)
        y = yy + ((n + COLS - 1) // COLS) * (CELL + GAP) + 10
    img.save(out_png)

def main():
    fdir = sys.argv[1] if len(sys.argv) > 1 else os.path.join(REPO, "docs", "functions")
    out_html = sys.argv[2] if len(sys.argv) > 2 else os.path.join(REPO, "docs", "progress_map.html")
    statuses = load_statuses(SYMBOLS)
    module_rows = {}
    for key in MODULES:
        path = os.path.join(fdir, key + ".json")
        if not os.path.exists(path):
            alt = None
            for cand in os.listdir(fdir):
                if cand.lower() == (key + ".json").lower():
                    alt = os.path.join(fdir, cand)
                    break
            if alt is None:
                print("skip (missing):", path)
                continue
            path = alt
        rows = load_module(path, MODULES[key][1])
        for r in rows:
            st = statuses.get(r["vaddr"], ("unidentified", r["name"], ""))
            r["status"], r["sym_name"], r["comment"] = st
        module_rows[key] = rows
        print("%-20s %5d functions" % (key, len(rows)))
    with open(out_html, "w", encoding="utf-8") as fh:
        fh.write(render(module_rows))
    print("wrote", out_html)
    out_png = os.path.splitext(out_html)[0] + ".png"
    try:
        render_png(module_rows, out_png)
        print("wrote", out_png)
    except ImportError:
        print("PIL not available, PNG skipped")

if __name__ == "__main__":
    main()

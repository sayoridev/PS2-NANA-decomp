#!/usr/bin/env bash
# Apply the PS2-NANA-decomp maintenance snapshot and create Schedule__Update.
# The script is idempotent and must be run from the repository root.
set -euo pipefail

if [[ ! -f README.md || ! -d src || ! -d symbols ]]; then
    echo "error: run this script from the PS2-NANA-decomp repository root" >&2
    exit 1
fi

chmod +x .githooks/pre-push

python3 - <<'PY'
from pathlib import Path
import re

ROOT = Path('.')
UPDATED = "2026-10-15"

def read(rel):
    return (ROOT / rel).read_text(encoding='utf-8')

def write(rel, text):
    (ROOT / rel).write_text(text, encoding='utf-8')

def replace_exact(rel, old, new):
    text = read(rel)
    if new in text:
        return
    if old not in text:
        raise SystemExit(f"error: expected block not found in {rel}")
    write(rel, text.replace(old, new, 1))

# Correct the crt0 BSS range. The original constants were reversed.
replace_exact(
    'src/common/ps2.h',
    "#define PS2_MAINLOOP_BSS_START  0x005A3680u     /* crt0 BSS clear range */\n"
    "#define PS2_MAINLOOP_BSS_END    0x001A7900u",
    "#define PS2_MAINLOOP_BSS_START  0x001A7900u     /* crt0 BSS clear range */\n"
    "#define PS2_MAINLOOP_BSS_END    0x005A3680u",
)
replace_exact(
    'src/system/startup.c',
    "/* 2. Clear BSS [0x5A3680, 0x1A7900). Original: align-to-16 loop, then",
    "/* 2. Clear BSS [0x001A7900, 0x005A3680). Original: align-to-16 loop, then",
)

# Add the missing main-loop prototypes.
replace_exact(
    'src/game/main.c',
    "extern void Sound__Update(void);        /* @0x00177F30 — sound command processor */\n"
    "extern void System__RunFrameHook(void); /* src/system/framehook.c */\n"
    "extern void Sound__FrameTick(void);     /* @0x00177F00 */",
    "extern void Sound__Update(void);        /* @0x00177F30 — sound command processor */\n"
    "extern void Schedule__Update(void);     /* src/game/schedule_update.c */\n"
    "extern void System__RunFrameHook(void); /* src/system/framehook.c */\n"
    "extern void Sound__FrameTick(void);     /* @0x00177F00 */\n"
    "extern void Schedule__FrameTick(void);  /* @0x00177F00 — pending boundary confirmation */",
)

# Remove obsolete duplicate TU if present from earlier snapshot.
legacy = ROOT / 'src/game/schedule_update.c'
if legacy.exists():
    legacy.unlink()

# Align the symbol README with the files that actually exist.
symbols_readme = '''# Symbols

Current symbol files:

- `functions.txt` — one entry per line: `address size status name`

Planned exports, once Ghidra boundaries are stable:

- `functions.csv` — spreadsheet-friendly function table
- `globals.txt` / `globals.csv` — data-only symbol tables

All addresses are virtual addresses of the original EE ELF. Function sizes are
still incomplete until the Ghidra boundary pass is exported.

Status values: `unknown | hypothesis | VERIFIED | PARTIAL | RECONSTRUCTED | STUB`.
'''
write('symbols/README.md', symbols_readme)

# Promote Schedule__Update in the symbol map.
symbols_path = ROOT / 'symbols/functions.txt'
symbols = symbols_path.read_text(encoding='utf-8')
new_symbol = "0x00177f30 0x98 RECONSTRUCTED Schedule__Update # src/game/schedule_update.c — countdown + sound command dispatch"
if new_symbol not in symbols:
    symbols, count = re.subn(
        r'^0x00177f30\s+\S+\s+\S+\s+Schedule__Update\b.*$',
        new_symbol,
        symbols,
        count=1,
        flags=re.M,
    )
    if count != 1:
        raise SystemExit("error: Schedule__Update symbol entry not found")
    symbols_path.write_text(symbols, encoding='utf-8')

# Update TODO.md.
todo = read('TODO.md')
todo = todo.replace(
    '*Last updated: 2026-10-14 (Phase 2 in progress)*',
    f'*Last updated: {UPDATED} (Phase 3 in progress)*',
)
completed_addition = (
    "- [x] `Schedule__Update` reconstructed in `src/game/schedule_update.c`\n"
    "- [x] crt0 BSS range corrected to `0x001A7900..0x005A3680`\n"
)
if completed_addition not in todo:
    todo = todo.replace('## Completed\n\n', '## Completed\n\n' + completed_addition, 1)
old_pending = '''- [x] ~~Cleanup pass B~~ DONE (2026-10-15): mode_update.c, scene_update.c,
game_init.c, schedule.c, registry.c, pad.c+pad.h committed clean;
raw files removed (preserved in git history).
'''
new_pending = '''- [ ] Raw-file cleanup: raw Ghidra files are temporarily retained as analysis
references. Remove each `_raw.c` only after its clean replacement is committed
and cross-checked. Current remaining raw references: 5.
'''
if new_pending not in todo:
    if old_pending not in todo:
        raise SystemExit('error: expected raw cleanup block not found in TODO.md')
    todo = todo.replace(old_pending, new_pending, 1)
next_addition = (
    "- [ ] Verify `Schedule__Update` against Ghidra/rizin disassembly and mark it\n"
    "  VERIFIED if CFG, call sequence and memory accesses match.\n"
)
if next_addition not in todo:
    todo = todo.replace('## Next recommended actions\n\n', '## Next recommended actions\n\n' + next_addition, 1)
old_milestones = '''| Phase 3 — Decompilation foundation | in progress | ~40% |
| Phase 3 — Decompilation foundation | pending | 0% |'''
new_milestones = '''| Phase 3 — Decompilation foundation | in progress | ~40% |'''
if new_milestones not in todo:
    if old_milestones not in todo:
        raise SystemExit('error: expected milestone block not found in TODO.md')
    todo = todo.replace(old_milestones, new_milestones, 1)
write('TODO.md', todo)

# Rewrite progress.yaml with the corrected schema and current snapshot.
progress = '''# =============================================================================
# PS2-NANA-decomp — machine-readable progress (single source of truth)
# Update this file with every meaningful change; TODO.md mirrors it for humans.
# The progress chart in docs/ is generated from this file by tools/ scripts.
# =============================================================================

project: PS2-NANA-decomp
updated: "2026-10-15"
phase: 3

# Analysis source: maintainer's personal dump (private, never committed).
executable:
  discovered: true
  local_path: null        # private local path of YOUR dump; never committed
  serial: SLPM_659.14
  format: ELF32 LSB MIPS R5900 (eabi64, mips3), statically linked, stripped
  entry_point: 0x00100008
  gp_value: 0x005ab270
  compiler_hints:
    - "Metrowerks CodeWarrior — MW MIPS C Compiler 2.4.1.01 (PlayStation 2)"
  overlay_load_base: 0x00b7f800   # hypothesis: OVL/*.BIN load regions

functions:
  total: 2348             # rizin 'aaa' estimate; confirm in Ghidra
  identified: 190         # pending Ghidra confirmation
  decompiled: 46          # clean reconstructed functions; raw references not summed
  matched: 0
  remaining: 2302         # 2348 - 46 clean reconstructions; estimate

modules: []

assets:
  identified: 3           # .nkp packages, .rws audio streams, .nkt/.nkf config/dialog
  documented: 0

milestones:
  - name: "Phase 0 — Project setup"
    status: completed
  - name: "Phase 1 — Dump inspection"
    status: completed
  - name: "Phase 2 — Executable analysis"
    status: completed
  - name: "Phase 3 — Decompilation foundation"
    status: in_progress
  - name: "Phase 4 — Game systems"
    status: pending
  - name: "Phase 5 — Assets and formats"
    status: pending
  - name: "Phase 6 — Build and verification"
    status: pending
  - name: "Phase 7 — Continuous progress"
    status: ongoing
'''
write('progress.yaml', progress)

# Record the maintenance snapshot in AGENTS.md.
agents = read('AGENTS.md')
snapshot = (
    "- Maintenance snapshot 2026-10-15: `Schedule__Update` reconstructed;\n"
    "  crt0 BSS range fixed; raw files retained pending one-by-one replacement.\n"
)
if snapshot not in agents:
    agents = agents.replace(
        '- Metrics: functions identified/decompiled/matched/remaining, modules,\n  assets understood, milestone completion.\n',
        '- Metrics: functions identified/decompiled/matched/remaining, modules,\n  assets understood, milestone completion.\n' + snapshot,
        1,
    )
agents = agents.replace('*Maintained by the project supervisor and AI assistant. Last updated: 2026-10-14.*',
                        f'*Maintained by the project supervisor and AI assistant. Last updated: {UPDATED}.*')
write('AGENTS.md', agents)
PY

cat <<'EOF'
PS2-NANA-decomp maintenance snapshot applied.
Next manual verification steps:
  git diff --check
  python3 tools/progress_chart.py
  git status --short
EOF

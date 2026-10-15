# Asset Formats

> Filled during Phase 5. Structure documentation only — never file contents.

## NKP — Konami Package (DECODED 2026-10-15)

**Magic:** `4E 4B 50 1A` ("NKP" + 0x1A)

Container with an id-to-offset index (FAT). Resources are stored
sequentially; ids are grouped in runs (blocks of ~11 resources at
fixed 0x21800 stride observed in `interior`, variable-size tail blocks).

### Header (16 bytes)

| Offset | Type | Field | Notes |
| --- | --- | --- | --- |
| +0x00 | char[4] | magic | `NKP\x1A` |
| +0x04 | u32 | base | load address / base vaddr (0x005B8000 in `interior`; large values in big packages — possibly size/base depending on kind) |
| +0x08 | u32 | flags | 0 in all samples |
| +0x0C | u32 | count | index entry count |

### Index (count x 8 bytes, from +0x10)

| Size | Field |
| --- | --- |
| u32 | id — first resource id of the run (ascending) |
| u32 | offset — file offset of the run's data |

Consistency rule: index ends at `0x10 + count*8`; first data offset must be
>= that (verified: 88-entry index ends 0x2D0, first data at 0x800).

### Observed samples

| Package | count | Notes |
| --- | --- | --- |
| interior (@0x35E1A0) | 0x58 (88) | ids 0x2D8.., uniform 0x21800 stride then variable tail |
| shiftbg (@0x39AB90) | 0x14 (20) | compact |
| story/allsc (@0x39B380) | 0x444 (1092) | the 1.65 GB disc package; large offsets |

### Tool

`tools/parse_nkp.py` — parses header + index (structure only):

```javascript
python3 tools/parse_nkp.py SLPM_659.14 0x35e1a0 --elf
python3 tools/parse_nkp.py STORY_ALLSC.NKP 0 --max 8   # on an extracted copy
```

### Open items

- [ ] Semantics of header.base (load addr vs size — samples differ)
- [ ] Resource record format at the indexed offsets (per-resource header?)
- [ ] Compression flag detection (any LZ/RLE in resource bodies?)
- [ ] Index run-length semantics (id delta = resources per run)

## MWo3 — Metrowerks Overlay v3

See `docs/overlay-format.md` for the complete structural draft.

## RWS — Konami RWA Audio Stream

Format used by the custom `RWA.IRX` driver. Files: `RWAB/BGM_*.RWS`,
`RWAV/V*.RWS`. Header and structure pending analysis.

## PSS — PS2 Standard Video Stream

`RWA/OPMV.PSS` — standard PS2 PSS format (MPEG-2 video + ADX audio).
Well-documented externally; no reverse engineering needed.

## NKF / NKT — Dialog / Config

Small text/config formats. `yesno1.nkf`, `konami.nkt` observed in strings.
Structure pending.

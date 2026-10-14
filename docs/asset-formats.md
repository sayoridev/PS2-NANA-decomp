# Asset Formats

> Filled during Phase 5. Structure documentation only — never file contents.

## NKP — Konami Package (confirmed 2026-10-14)

**Magic:** `4E 4B 50 1A` (bytes: `N K P 0x1A`)

Observed headers (from maintainer's personal dump, structural analysis only):

| File | Vaddr | Bytes 0–15 |
| --- | --- | --- |
| interior | 0x35E1A0 | `4E 4B 50 1A 00 80 5B 00 00 00 00 00 58 00 00 00` |
| shiftbg | 0x39AB90 | `4E 4B 50 1A 00 48 51 00 00 00 00 00 14 00 00 00` |
| story/allsc | 0x39B380 | `4E 4B 50 1A 00 78 5D 62 00 00 00 00 44 04 00 00` |

**Header hypothesis (16 bytes):**

```javascript
+0x00  u32  magic   = 0x1A504B4E (LE: 4E4B501A)
+0x04  u32  field1  = ? (0x005B8000, 0x00514800, 0x625D7800)
+0x08  u32  field2  = 0x00000000 in all samples
+0x0C  u32  count   = ? (0x00000058, 0x00000014, 0x00000444)
```

The `count` field (0x0C) varies significantly: 0x58 (88), 0x14 (20), 0x444
(1092). Likely number of entries in an index table following the header.

**Open items:**

- [ ] Decode field1 semantics (size? base address? compression?)
- [ ] Verify count = number of sub-files / entries
- [ ] Map the index table structure (entry size, fields)
- [ ] Check if NKP is a flat concatenation or has offset table
- [ ] Compare with root-level .NKP packages (CHMAKE.NKP, CLOTHES2.NKP, etc.)

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

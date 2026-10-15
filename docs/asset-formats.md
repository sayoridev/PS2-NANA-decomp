# Asset Formats

> Filled during Phase 5. Structure documentation only — never file contents.

## NKP — Konami Package (VERIFIED 2026-10-15)

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
| u32 | id — first resource id of the RUN (ascending) |
| u32 | offset — file offset of the run's data |

**Run-length semantics (verified on 3 samples):** entry `i` covers resource
ids `[id_i, id_{i+1})` — the id delta IS the resource count of the run.

- interior: runs of 11-12 resources, uniform 0x21800 stride (fixed-size
blocks) then variable-size tail runs
- story/allsc: PERFECTLY uniform 0x800 stride, exactly 12 ids per run

Consistency rule: index ends at `0x10 + count*8`; first data offset must be
>= that (verified: 88-entry index ends 0x2D0, first data at 0x800).

**Global id space:** ids do not restart per package (interior starts at
0x2D8, story/allsc at 0x2238) — the engine numbers resources globally.

**Embedded header vs disc payload:** the story/allsc sample claims 1092
runs (~2.3 MB payload) but the ELF cannot hold it (image ends 0x5A3680).
The embedded copy at 0x39B380 is therefore the HEADER+INDEX of the disc
package STORY/ALLSC.NKP (name matches the on-disc file exactly); the
flag==0 "embedded" path for big packages means "header embedded, payload
from disc". `base` = 0x625D7800 (~1.57 GB) is consistent with the disc
package size (1,650,292,736 B) — likely payload size / load extent.

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

- [x] ~~Semantics of header.base~~ RESOLVED: RAM load base for small packages (0x5B8000), payload size/extent for disc-backed ones (0x625D7800)
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

## RWS — RWA Audio Stream (bank model VERIFIED 2026-10-15)

RWS files are VOICE/BANK containers addressed by code-side tables:

| Stream set | Files | Table | Contents |
| --- | --- | --- | --- |
| `RWAV/V000–V024.RWS` | 25 | `g_VoiceBankTable` @0x001F6070 (25 x {min,max}) | voice lines: bank i = ids [i*500, i*500+499]; **12,311 lines total** (last bank 311) |
| `RWAB/BGM_001–030.RWS` | 30 | BGM table @0x001F5E5C (30 x {name, path, param}) | background music, one file per entry |

- Voice IDs are global across banks (bank = id / 500, line = id % 500).
- `TaskCmd__04/05` (Voice__Play) resolve id -> bank via the range table,
then stream the corresponding `.RWS` through the RWA.IRX driver.
- RWS internal record format (line header inside a bank file): pending —
first bytes of V000.RWS needed (extract from personal dump when in Phase 5).

## Audio address-space summary (code <-> disc cross-reference)

```javascript
Bgm__Play(idx)      -> BGM table[30]        -> RWAB/BGM_{idx+1:03d}.RWS
Voice__Play(id)     -> VoiceBankTable[25]   -> RWAV/V{id/500:03d}.RWS
VoiceBank2 (cmd 07) -> gp table 0x5A3388    -> system SE bank (RWA_SE1?)
Pad__SetOutputs     -> RWA.IRX via SIF DMA  -> DualShock 2 actuators
```

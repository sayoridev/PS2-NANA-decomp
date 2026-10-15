# Overlay Format — MWo3 (Metrowerks CodeWarrior Overlay v3) — VERIFIED 2026-10-15

> Documented from structural analysis of the maintainer's personal dump.
> Structure only; no file contents are stored in this repo.
> Status: DRAFT — header field semantics marked (?) are hypotheses to verify
> against additional overlays and runtime behavior.

All files in `OVL/` on the disc use this format. They are loaded at runtime
into the reserved regions at **0x00b7f800** (see `docs/executable.md` §memory map).

## Sample files analyzed

| File | Size | Header field @0x04 | Code size @0x0C | Tail field @0x14 |
| --- | --- | --- | --- | --- |
| `SHOP2R.BIN` | 15,872 (0x3e00) | 8 | 0x38c0 | 0x97 (151) |
| `JOBFND2R.BIN` | 9,984 (0x2700) | 7 | 0x1dc0 | 0x02 |

## Header layout (draft)

| Offset | Size | Field | Notes |
| --- | --- | --- | --- |
| 0x00 | 4 | Magic | ASCII `MWo3` = Metrowerks Overlay v3 |
| 0x04 | 4 | u32 | (?) unknown — values 8 (SHOP2), 7 (JOBFND). NOT header size: code starts at 0x80 in both samples, JOBFND pads 0x70–0x7F with zeros |
| 0x08 | 4 | u32 | Load address — `0x00b7f800` in all samples (matches ELF reserved regions) |
| 0x0C | 4 | u32 | Code size in bytes |
| 0x10 | 4 | u32 | (?) small value (5, 9) — unknown; possibly BSS-related or section count |
| 0x14 | 4 | u32 | (?) **relocation count** — 4-byte records. SHOP2: 151 relocs ≈ 604 B fits 0x3940–0x3CC0 region; JOBFND: 2 relocs (tiny wrapper, consistent) |
| 0x18 | 4 | u32 | Image end vaddr = load addr + file size (0x00b83600) |
| 0x1C | 4 | u32 | Same as 0x18 in both samples |
| 0x20 | 16 | Name | ASCII filename, NUL-padded (`shop2R.bin`, `jobfnd2R.bin`) |
| 0x30.. | — | (?) padding / additional entries up to header end |  |

## File regions (SHOP2R.BIN reference)

| Range | Content |
| --- | --- |
| 0x000–0x07F | Header — **fixed 0x80 bytes** in both samples |
| 0x080–0x393F | MIPS R5900 code (0x38c0 bytes) — starts with CW prologue `addiu sp,sp,-N` + `sq $sX, off($sp)` |
| ~0x3940–~0x3CBF | (?) relocation data — target addresses into main ELF and overlay region |
| ~0x3CC0–0x3DFF | Embedded string/asset manifest: `*.nkf` names, `*.nkt` names, `shop2/shop2_m.nkp` package ref, Shift-JIS dialog text |

## Small overlays are thin wrappers

JOBFND2R.BIN code begins: `addiu a1, zero, 0x50; j 0x0143e230; …; jr ra; nop`
— sets one parameter and jumps straight into a main-ELF function.
Hypothesis: the three small overlays (JOBFND 10 KB, SHOP2 16 KB, CLOTHE 38 KB)
are parameter+stub shims; substantial logic lives in the main ELF or in the
four large overlays (ENDINGR 4.4 MB, STATUSR 1.9 MB, WORKINGR 1.4 MB,
MYROOMR 1.1 MB).

## Observed code characteristics

- Prologue: `addiu $sp, $sp, -N` followed by `sq $sX, off($sp)` (128-bit saves)
- References to main-ELF globals via `lui`/`addiu` pairs (e.g. data at 0x00b9bbe0)
- **Cross-calls into the main executable**: `jal 0x0165dc40` observed — overlays
call functions in `SLPM_659.14`; relocation of these call targets is required
at load time (hence relocation area)

## Open items

- [ ] Verify header-size hypothesis (0x04 field × 16) on all 7 overlays
- [ ] Decode relocation record format (offset + type + target?)
- [ ] Decode 0x10 / 0x14 field semantics
- [ ] Confirm runtime loader location in main ELF (who parses MWo3? main code or RTFSSIOP.IRX?)
- [ ] Check whether strings region is referenced by pointer table inside code

## Live analysis — SHOP2R.BIN first function (0x00B7F880)

`Shop__FrameUpdate` (XREF[7] within overlay): iterates 19 shop item slots
(idx 1..19); per slot with cooldown > 0: decrements, computes visual scale
`(t/2)*32` and fade `t*255/25` (25-tick fade-out), calls main-ELF render
helpers 0x001A8DE0 and 0x001444AF0 with overlay globals.
Overlay runtime areas observed:

- 0xB8BBE4: pointer table (slot state), +0xD4 = cooldown counters
- 0xB8BD70: item info pointer table
- 0xB8BC50: base index global; 0xB8C024/0xB8C028: render globals
- 0xB83300/0xB83610: fade/scale structs passed to render helpers

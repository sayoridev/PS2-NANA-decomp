# Executable Analysis — Nana (PS2, SLPM_659.14)

> Source: maintainer's personal dump, boot ELF `SLPM_659.14` (4,864,752 bytes).
> All analysis is local; nothing from the binary is stored in this repo.

## ELF Header

| Field | Value |
| --- | --- |
| Class / Data | ELF32 / 2's complement, **little endian** |
| Type | EXEC (statically linked executable) |
| Machine | MIPS (flags: `0x20924000` — **R5900**, eabi64, mips3) |
| Entry point | **0x00100008** |
| Program headers | 10 (offset 52) |
| Section headers | 16 (offset 0x4a3870) |
| OS/ABI | UNIX System V |

## Compiler / Toolchain

```javascript
.comment: "MW MIPS C Compiler (2.4.1.01)" + "PlayStation2"
```

**Metrowerks CodeWarrior 2.4.1.01 for PlayStation 2.**

Consequences for reconstruction:

- Codegen idioms differ from GCC/ps2sdk (expect `bl`/`b` call forms, CW-specific
prologue/epilogue and delay-slot habits). Catalog them empirically in
`notes/analysis/` before bulk decompilation.
- Verification in Phase 6 must tolerate CW-vs-GCC structural differences;
behavioral + CFG comparison is primary, instruction-level diff secondary.

## Register info (.reginfo)

| Field | Value |
| --- | --- |
| `ri_gprmask` | 0xfefffff7 |
| `ri_cprmask[0]` (FPU) | 0xf7ffffff (FPU used) |
| `ri_gp_value` | **0x005ab270** ($gp anchor for global addressing) |
| `ri_cprmask[1..3]` | 0 (no other coprocessors) |

## Sections (16 total — merged layout, linker custom/SN-style)

| Nr | Name | Type | Addr | Size | Notes |
| --- | --- | --- | --- | --- | --- |
| 1 | .shstrtab | STRTAB | — | 0x2d |  |
| 2 | .strtab | STRTAB | — | **0** | present but **empty** |
| 3 | .symtab | SYMTAB | — | **0** | present but **empty** → effectively **stripped** |
| 4 | (anon) | PROGBITS | 0x00100000 | 0x4a3680 | **all code+rodata+data merged**, WAX |
| 5–12 | (anon) | PROGBITS | 0x00b7f800 | 0 | empty, WAX |
| 13 | (anon) | PROGBITS | 0x01a7f900 | 0 | empty, WA |
| 14 | .comment | PROGBITS | — | 0x2b | compiler ID |
| 15 | .reginfo | MIPS_REGINFO | — | 0x18 | see above |

No `.text/.rodata/.data/.bss` separation — everything lives in one section.
Static analysis must separate code from data heuristically (rizin/Ghidra + manual).

## Program headers / memory map

| Seg | VirtAddr | FileSiz | MemSiz | Role |
| --- | --- | --- | --- | --- |
| LOAD 0 | 0x00100000 | 0x4a3680 | 0xa7f800 | main image: ~4.86 MB file, ~6 MB additional BSS (mem extends to 0x00b7f800) |
| LOAD 1–8 | 0x00b7f800 | 0 | 0xf00100 / 0x42e000 / 0x1d7580 / 0x15b000 / 0x12b180 / 0x28180 / 0x02900 / 0x0d500 | **reserved OVL slots** — 8 overlay load regions reusing base 0x00b7f800 |
| LOAD 9 | 0x01a7f900 | 0 | 0 | RW marker |

**Hypothesis (strong): OVL/*.BIN modules are loaded at 0x00b7f800**, each
overlay claiming one of the reserved regions. Verify when analyzing OVL format.

## Function inventory (rizin 0.8.2, `aaa`)

- Detected functions: **2348** (initial estimate; single merged section makes
auto-analysis noisy — data blocks probed as code produce "Invalid instruction"
errors; true count to be confirmed in Ghidra).
- Symbol table empty → all names to be recovered (see AGENTS.md §8 conventions).

## Runtime library usage (from strings)

- **Konami "PsII" wrappers**: `PsIIlibcdvd 3000`, `PsIIlibpad 3010` (internal
wrappers over Sony CDVD and pad libraries)
- **IOP modules loaded by path** (lowercase `/iop/*.irx`): cdvdstm, libsd,
mcman, mcserv, padman, rtfssiop, rwa, sdrdrv, sio2man
- **Sony kernel/API surface observed**: `SceSifrpc*`, `SceKernel*` (threads,
libc, stdio drivers), `SceCd*` (semaphores + N/S/R cmd paths), `sceMpeg*`,
`libipu` (video playback), `libpad`
- Error strings: `Libcdvd bind err ...`, `Need to re-setup libipu since
sceMpegGetPicture was aborted`, `libpad: Module version mismatch`

## Asset references embedded in code

- **`.nkp` story packages named `A-XX-YY.nkp`** (e.g. `A-00-00.nkp`,
`A-01-01.nkp` … `A-02-04.nkp`): letter / episode / scene indexing scheme
- Format strings: `%s.nkf`, `%s%s%c.bin` — runtime-composed asset paths
- Config: `konami.nkt` (runtime config file); `yesno1.nkf` / `yesno10004`
(dialog resources)

## Entry-point region (0x100000–0x10007F) — system pointer table

Raw bytes at file offset 0x180 (vaddr 0x100000):

- 8 zero bytes, then **30 little-endian pointers** into the `0x7000xxxx`
region (stride exactly 0x800): `0x70000c28, 0x70001428, … 0x7000fc28`,
terminated by `0x00000011`.
- The ELF entry point (0x00100008) points **inside this table** — the words
are data, not instructions (this explains rizin's "invalid" spam and why
hand-decode of the first words is nonsensical).
- XREF evidence: `fcn.00118478` reads table entries (DATA XREF ×3);
`syscall.0.555070` (a syscall stub) has a CODE XREF to 0x100018;
`fcn.00555d40` references 0x100018/0x10003c.
- Interpretation: **system/interface dispatch table** set up for runtime
service calls (kernel-side targets at 0x7000xxxx). The true executable
startup code begins at **0x00100080**.

## Overlay loading (see overlay-format.md)

All `OVL/*.BIN` use the **MWo3** format (Metrowerks CodeWarrior Overlay v3):
magic `MWo3`, load address 0x00b7f800 (matches reserved ELF regions),
code + relocations + embedded asset-name strings. Overlays call back into
the main executable (`jal 0x0165dc40` observed in SHOP2R).

## Startup chain (Phase 2, rizin-assisted)

| Address | Hypothesis | Evidence |
| --- | --- | --- |
| 0x00100080 | `Startup__Entry` (crt0) | FPU zeroing (`mtc1 f0..f31`), `ctc1 zero,31`, BSS clear loop 0x5A3680–0x1A7900 |
| 0x001000F0 | `Fpu__ClearAll` | callable `mtc1 f24..f31` block; CALL XREF from fcn.0044e1f0 (reset path) |
| 0x00118478 | `System__InstallHandlers` | installs 0x10-byte entries from table @0x1C5078 (cap 49), IRQs masked via `mtc0` |
| 0x00118398 | `System__AddHandler` | **kernel syscall 0x56 wrapper** (`v1=0x56; syscall`); real install done by kernel |
| 0x00111688 | `System__CalcTableEnd` | end-1 arithmetic before install loop |
| 0x00119098 | `System__Fatal` | called with a0=1 after error string setup |
| 0x001C5078 | `g_SystemHandlerTable` | header {13,18,8}; {id, handler} pairs; code stubs embedded @0x1C50B8 |
| 0x00118C98 | `System__BootInit1` | first boot-phase call from crt0 |
| 0x0010EBA0 | `System__Syscall64` | syscall-0x64 wrapper at head of stub table 0x10EBA0–0x10ECDC (IDs 0x64–0x76, 46+ callers); NOT a boot phase |
| 0x0012B968 | `Random__Next` | Knuth LCG (a=6364136223846793005, c=1), 44 xrefs — src/system/random.c |
| 0x0014E8E0 | `Time__Tick` | frame counter @0x71FDE8 (cap 0x0CDFD7F0) — src/system/time.c |
| 0x00123C18 | `Random__MulStep` | 64×64 multiply helper for the LCG |
| 0x00141EE0 | `Game__Main` | init then **infinite main loop** (0x141F04), ~13 fixed calls per iteration |
| 0x00127DE0 | `System__Exit` | exit-callback lists (ctx+0x148), returns to crt0 @0x100218; 7 external callers |

crt0 verified: $gp = 0x5AB270 set in code (matches .reginfo); syscall 0x3C
returns sp; syscall 0x3D secondary setup; syscall ID families 0x56..0x59
(add/remove handler pairs) around 0x1183A8.

Note: `syscall.0.555070` (rizin label) decodes as **data** — likely a dispatch
table with embedded syscall opcodes. Confirm in Ghidra.

## Open items

- [ ] Confirm true function count + boundaries in Ghidra 12.1.2
- [ ] Entry-point trace: `pd 30 @ 0x100008` → locate `main` and init chain
- [x] OVL format identified: MWo3 (Metrowerks overlay) — see overlay-format.md
- [ ] Verify MWo3 header fields on all 7 overlays
- [ ] Import stub table → full list of Sony SDK functions referenced
- [ ] Locate and document the `A-XX-YY.nkp` dispatch table (file name → loader call)

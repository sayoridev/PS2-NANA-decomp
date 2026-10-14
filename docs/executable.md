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

## Open items

- [ ] Confirm true function count + boundaries in Ghidra 12.1.2
- [ ] Entry-point trace: `pd 30 @ 0x100008` → locate `main` and init chain
- [ ] OVL .BIN load format check (expect raw MIPS code for 0x00b7f800)
- [ ] Import stub table → full list of Sony SDK functions referenced
- [ ] Locate and document the `A-XX-YY.nkp` dispatch table (file name → loader call)

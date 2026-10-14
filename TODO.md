# TODO — Project State

> This file must always reflect the **real** state of the project.
> Update it with every meaningful change. Machine-readable mirror: `progress.yaml`.

*Last updated: 2026-10-14 (Phase 2 in progress)*

## Current phase

**Phase 3 — Decompilation Foundation**

## Completed

- [x] Repository created: `sayoridev/PS2-NANA-decomp` (public), initial docs pushed
- [x] Arch Linux environment prepared (git, gh, python, binutils, Ghidra 12.1.2,
rizin 0.8.2 + rz-ghidra, 7-Zip, fuseiso)
- [x] Phase 1 — Dump inspection:
- [x] Serial identified: **SLPM_659.14** (Japan, Konami/KCEJ, master 2005-02-02)
- [x] Boot confirmed via SYSTEM.CNF (`BOOT2 = cdrom0:\SLPM_659.14;1`, NTSC)
- [x] SHA-256 of dump recorded: `44919209…120419`
- [x] Disc mapped: 120 files, 7 dirs (see `docs/disc-map.md`)
- [x] Dominant asset identified: `STORY/ALLSC.NKP` (1.65 GB)
- [x] Phase 2 (partial) — Executable identification:
- [x] ELF32 LSB MIPS **R5900**, entry 0x00100008, statically linked, stripped
- [x] **Compiler: Metrowerks CodeWarrior (MW MIPS C Compiler 2.4.1.01)**
- [x] $gp = 0x005ab270 (from .reginfo)
- [x] Merged single-section layout; OVL slots at 0x00b7f800
- [x] Function inventory estimate: **2348** (rizin)
- [x] Loader/SDK strings mapped (PsII wrappers, /iop/*.irx, A-XX-YY.nkp scheme)

## In progress

- [x] Entry region mapped: pointer table 0x100000–0x10007F; code starts 0x100080
- [x] crt0 identified at 0x100080 (FPU init + BSS clear loop)
- [x] System handler-install chain mapped (0x118478 ← table @0x1C5078, cap 49)
- [x] OVL format identified: MWo3 (see docs/overlay-format.md); small OVLs are thin wrappers
- [x] First symbols seeded: symbols/functions.txt (10 functions + 2 data symbols)
- [x] $gp verified in crt0 code (0x5AB270, matches .reginfo)
- [x] Boot chain mapped: crt0 → syscall 0x3C/0x3D → BootInit1/2/3 → BootTail
- [x] rizin raw mode confirmed for overlay disassembly
- [x] MAIN LOOP located: Game__MainLoop @0x141F04 (13 body calls)
- [x] First source files: src/system/startup.c (Startup__Entry, Fpu__ClearAll — RECONSTRUCTED)
- [x] Time__Tick + Random__Next reconstructed (Knuth LCG identified, 44 xrefs)
- [x] g_SystemContext identified @0x1C7FD0 (central system struct)
- [x] System__Exit resolved (exit-callback lists, back to crt0)
- [x] Arena allocator init found in BootInit1 tail (0x5AB8C8, 0x10-byte slots)
- [x] Codegen idiom catalog started: notes/analysis/codegen.md

## Pending

- [ ] Confirm function count/boundaries in Ghidra; export to `symbols/`
- [ ] Catalog Metrowerks CodeWarrior codegen idioms (`notes/analysis/codegen.md`)
- [ ] Import stub table → Sony SDK function list for `src/common/` shims
- [ ] Locate `.nkp` dispatch table(s) in code
- [ ] Progress chart generator in `tools/` (renders `progress.yaml` → `docs/progress.svg`)

## Blocked

- [ ] PCSX2 emulator (verification, Phase 6) — AUR; deferred, no action needed now
- [ ] ps2dev toolchain (Phase 6) — build from source when reconstruction begins

## Discovered issues

- `file` reports "not stripped" but `.symtab`/`.strtab` are empty → treat as
stripped (documented in `docs/executable.md`).
- Merged code+data section makes rizin auto-analysis noisy (data probed as
code → "Invalid instruction" spam). Ghidra confirmation needed for the count.

## Next recommended actions

1. Commit updated docs (disc-map, executable, overlay-format, AGENTS, TODO, progress).
2. Classify remaining 9 main-loop body functions (notes/analysis/mainloop.md — 5 of 14 done).
3. Find MWo3 loader → candidates Game__Main inits (0x141D90, 0x13BB80).
4. Start Ghidra project; confirm function boundaries for the 2348 estimate.

## Milestones

| Milestone | Status | Progress |
| --- | --- | --- |
| Phase 0 — Project setup | completed | 100% |
| Phase 1 — Dump inspection | completed | 100% |
| Phase 2 — Executable analysis | completed | 100% |
| Phase 3 — Decompilation foundation | in progress | ~20% |
| Phase 3 — Decompilation foundation | pending | 0% |
| Phase 4 — Game systems | pending | 0% |
| Phase 5 — Assets and formats | pending | 0% |
| Phase 6 — Build and verification | pending | 0% |
| Phase 7 — Continuous progress | ongoing | — |

*Percentages are estimates until measurable units stabilize (function count pending Ghidra confirmation).*

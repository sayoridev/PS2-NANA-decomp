# AGENTS.md — Technical Project Documentation

Complete technical reference for the **PS2-NANA-decomp** project. Read this
before touching any code, tool, or binary. Update it when workflows change.

---

## 1. Project goal, legal constraints, non-goals

**Goal.** Produce a readable, documented, progressively buildable C/C++
reconstruction of the PlayStation 2 game *Nana* through reverse engineering,
for education, preservation, and technical research.

**Legal constraints.**
- Work exclusively from a **personal, legally obtained dump** (owned by the
  maintainer/contributor). Never share it.
- The repository contains **zero copyrighted game data**: no ISO, no extracted
  files, no assets. Only reconstructed code, docs, symbol maps, metadata.
- Documenting structural facts (offsets, sizes, headers, control flow) is
  fine; reproducing content is not.
- MIT license applies to reconstructed code/docs only (see `LICENSE` note).

**Non-goals.**
- No asset redistribution, no "full game" builds shipping data, no piracy
  facilitation, no claims on the original IP.

---

## 2. Required tools and versions

Verified on Arch Linux (2026-10-14):

| Tool | Version | Role |
|------|---------|------|
| git | 2.56.0 | version control |
| github-cli (`gh`) | 2.102.0 | repo management |
| python + pip | 3.14.8 / 26.2.1 | analysis & progress tooling |
| binutils (readelf/objdump) | 2.47 | ELF inspection, MIPS disassembly |
| file | 5.48 | format identification |
| Ghidra | 12.1.2 | interactive decompilation (EE MIPS-R5900) |
| rizin + rz-ghidra | 0.8.2 / 0.8.0 | scriptable disasm/decomp, batch analysis |
| 7-Zip (`7z`) | 26.04 | ISO listing/extraction |
| fuseiso + fusermount | 2.9.9 | read-only ISO mount |
| PCSX2 | pending (AUR) | behavioral verification (Phase 6) |
| ps2dev toolchain | pending | EE/MIPS rebuild (Phase 6, open source) |

If versions drift, update this table and `docs/environment.md`.

---

## 3. Arch Linux environment setup

Already executed (kept here as reproducible reference):

```bash
sudo pacman -Syu
sudo pacman -S --needed base-devel git github-cli python python-pip
sudo pacman -S --needed binutils file          # readelf-less does not exist; readelf is in binutils
sudo pacman -S --needed ghidra
sudo pacman -S --needed rizin rz-ghidra
sudo pacman -S --needed 7zip fuseiso
# Pending (Phase 6): PCSX2 via AUR; ps2dev toolchain from source.
```

Verification commands:

```bash
git --version && gh --version && python --version
readelf --version | head -1 && objdump --version | head -1
7z | head -3 && fusermount --version
```

---

## 4. ISO/dump extraction workflow (Phase 1)

Principles: **read-only by default**, analyze in place, extract only what a
specific task needs, keep everything **outside the repository**.

1. Record integrity of your personal dump (stays local, never committed):
   ```bash
   sha256sum /path/to/your/nana.iso | tee ~/nana_sha256.txt
   ```
2. List contents without extracting:
   ```bash
   7z l /path/to/your/nana.iso
   ```
3. Mount read-only for inspection:
   ```bash
   mkdir -p ~/mnt/nana && fuseiso /path/to/your/nana.iso ~/mnt/nana
   # ... inspect ...
   fusermount -u ~/mnt/nana
   ```
4. Expected layout on a PS2 disc: `SYSTEM.CNF` (boot config, points to the
   main ELF, e.g. `BOOT2 = cdrom0:\SLUS_XXX.XX;1`), a `MODULES/` or similar
   directory with IOP `.irx` drivers, and game data files.
5. Document findings in `docs/disc-map.md` (paths, sizes, hashes, hypotheses).

---

## 5. PS2 executable identification and loading

- PS2 main executables are **32-bit big-endian MIPS ELF** files targeting the
  Emotion Engine (R5900, MIPS-III + custom MMI extensions). IOP modules
  (`.irx`) are MIPS R3000A (MIPS-I, little... note: EE is big-endian; IOP
  IRX are little-endian MIPS-I) — verify per-file.
- Identify with:
  ```bash
  file SLUS_XXX.XX          # expect: ELF 32-bit MSB executable, MIPS R5900
  readelf -h SLUS_XXX.XX    # class, data encoding, machine, entry point
  ```
- `SYSTEM.CNF` resolves the boot ELF path and may reveal the serial
  (SLUS/SCES/SLPS/SCKA...) used for identification and cross-referencing
  documented builds.

---

## 6. ELF analysis (Phase 2)

Standard pass, always from the **local** dump copy:

```bash
readelf -h <elf>     # header: entry, machine, flags
readelf -S <elf>     # sections: .text .data .bss .rodata sizes → function/data inventory scale
readelf -s <elf>     # symbols (retail games are usually stripped; check anyway)
readelf -r <elf>     # relocations (usually none/few for fixed-address EE ELFs)
readelf -l <elf>     # program headers, load addresses
```

- EE ELFs typically load at `0x00100000` region; record the actual base —
  every address map in `symbols/` is relative to it.
- Extract strings for SDK/compiler hints:
  ```bash
  strings -n 8 <elf> | grep -iE 'gcc|gnu|sn systems|ee|sdk|version' | sort -u
  ```
  Compiler identification narrows the reconstruction dialect (prologue
  patterns, register conventions, division/multiply idioms).
- Build the **function inventory**: use rizin for batch disassembly stats
  (`rizin -q -c 'afl' <elf>` after analysis) or Ghidra's function boundaries.
  Record count in `progress.yaml`.

---

## 7. MIPS instruction analysis (R5900 specifics)

- 64-bit GPRs (MIPS-III), 128-bit multimedia registers (`$vf*`), FPU is the
  EE "FPU" coprocessor — expect heavy use of `lqc2/sqc2`, MMI opcodes
  (`pmaddw`, `pmulth`, `mfsa/mtsa`, etc.) in rendering/math code.
- Prologue pattern: stack frame via `addiu $sp, $sp, -N`, saves to
  `($sp+offset)`; epilogue restores `$ra`, `jr $ra; nop` (branch delay slot).
- Leaf functions often skip the frame; recognize tail calls
  (`j func; nop` as pseudo `jr`-less return).
- Common idioms to catalog in `notes/analysis/`: 64-bit mul/div via
  `mfhi/mflo` pairs, unaligned loads (`lwl/lwr/swl/swr`), inline memcpy via
  `lq/sq`.

---

## 8. Symbol recovery and naming conventions

Retail ELF is expected stripped; symbols are recovered progressively.

- **Known SDK functions**: match against PS2SDK/open-source equivalents
  (` sce* ` kernel stubs often remain as import stubs — check `.iopmod` /
  import tables for module names like `sceCd`, `sceGs`, `scePad`).
- **Naming scheme** (enforced):
  - Known purpose: `ModuleName__FunctionName` (PascalCase module, camelCase fn),
    e.g. `Render__DrawSprite`, `Save__WriteBlock`.
  - Unknown: `func_00123ABC` (8 hex digits = original virtual address).
  - Data: `g_CamelCase` for globals, `s_CamelCase` for statics,
    `D_00123ABC` for unlabeled data at address.
  - Structs: `ModuleName__StructName` typedef'd to `ModuleNameStructName`.
  - Enums: `ModuleName__EnumName`, members `MODULE_ENUM_MEMBER`.
- Maintain the mapping in `symbols/` (text map + CSV); every rename is a
  commit (`symbols: rename func_0019A3F0 → Audio__InitSpu2 (VERIFIED)`).

---

## 9. Function matching and decompilation workflow

Per function, in order:

1. **Inventory** — address, size, callers/callees (rizin `axt`/`axff`).
2. **Hypothesis** — name + one-line purpose, added to symbol map (status: unknown).
3. **Decompile** — Ghidra interactive or rz-ghidra batch; reconstruct in `src/`
   preserving boundaries; label status RECONSTRUCTED.
4. **Understand** — document args/return/globals/side effects in the function
   header comment; promote to PARTIAL or VERIFIED as understanding firms.
5. **Verify** — structural comparison (control flow graph, stack frame, reg
   usage) against the original disassembly; behavioral checks under PCSX2 in
   Phase 6. Only then count as `matched` in `progress.yaml`.
6. Track everything in `progress.yaml` → chart in `docs/`.

Status header template (top of every reconstructed function):

```c
/* STATUS: VERIFIED | PARTIAL | RECONSTRUCTED | STUB
 * ADDRESS: 0x00123ABC  SIZE: 0x58
 * PURPOSE: one-line description */
```

---

## 10. Data structures, globals, memory layout

- Map `.data`/`.bss` from `readelf -S`; identify vtables/pointer tables by
  scanning for address-range constants (`0x001xxxxx`) — scriptable in
  `tools/`.
- Document global layout in `docs/` as `symbol name, address, size, inferred
  type, evidence`. Evidence = disassembly references, never raw dumps.
- Watch for SDK globals (`_gp` value from ELF header → $gp-relative
  addressing base).

---

## 11. Asset format identification and extraction methodology (Phase 5)

- First check **known PS2 formats**: TIM2 (textures, magic `TIM2`), VTF/VU
  micro-programs, ADX (audio, magic `0x80 0x00 'A' 'D' 'X'` header), PSS/SS2
  (video), VAG (audio), standard-ish containers (PACK/ARC with size tables).
- Methodology per format: locate magic → map header fields → determine
  offset/size table → write a parser in `tools/` → document in
  `docs/asset-formats.md` with **structure only**.
- Extraction scripts must read from the user's local dump path (passed as an
  argument or env var, defaulting to nothing) and write outside the repo.
- **Never** commit extracted assets or full-file fixtures.

---

## 12. Reconstructed source organization (defined fully in Phase 3)

Preview:

```
src/
  common/        # shared types, SDK shims, macros
  system/        # init, main loop, memory
  render/        # GS/VU drawing
  audio/         # SPU2/ADSR, streams
  input/         # pad/remote
  scene/         # state machine, script VM (if any)
  ui/            # menus, HUD
  save/          # memory-card (PS2 MC) handling
  asset/         # loaders per documented format
```

One directory per subsystem; file names mirror module names.

---

## 13. Build and verification workflow (Phase 6)

- Build with the open-source **ps2dev** toolchain (GCC MIPS + ps2sdk) —
  no proprietary Sony SDK. Install only when reconstruction is far enough to
  compile.
- Link script must reproduce original section layout (addresses from Phase 2)
  so structural comparison stays meaningful.
- Verification (non-infringing):
  - Compare per-function CFG and stack frames against original disassembly.
  - Run under PCSX2 with logging hooks at reconstructed function addresses.
  - Record mismatches/assumptions in `docs/build.md` and `TODO.md`.

---

## 14. Git workflow, commits, branches

- See `CONTRIBUTING.md` for full rules. Summary:
  - Conventional Commits, English, imperative.
  - `main` protected; branches `decomp/<area>`, `docs/<topic>`, `tools/<name>`.
  - No binaries, no force-push to `main`, no destructive history rewrites
    without explicit maintainer confirmation.

---

## 15. Progress tracking rules

- `progress.yaml` is the **single source of truth**; `TODO.md` mirrors it.
- Update on every merged unit of work (function, module, format, milestone).
- `tools/` hosts the generator that renders the chart from `progress.yaml`
  into `docs/progress.svg` (or interactive HTML). Regenerate on each update;
  never hand-edit the chart.
- Progress metrics: functions identified/decompiled/matched/remaining,
  modules completed, assets understood, milestone completion.

---

## 16. Known issues, assumptions, unresolved questions

- **Dump not yet inspected.** All executable-level numbers (function counts,
  sizes, compiler) are pending Phase 1/2.
- **PCSX2 pending** — AUR install deferred to Phase 6 (verification).
- **ps2dev toolchain pending** — install from source in Phase 6.
- **Endianness note** — EE ELF is big-endian; IRX modules are little-endian
  MIPS-I. Confirm per-file with `readelf -h` before analysis.
- **Assumption**: retail build is stripped; if symbols survive, Phase 2 is
  dramatically easier — check `readelf -s` output size early.
- **Open question**: which engine/framework the game uses (in-house vs.
  licensed middleware) — strings analysis in Phase 2 will answer.

---

*Maintained by the project supervisor and AI assistant. Last updated: 2026-10-14.*

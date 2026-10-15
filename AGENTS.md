# AGENTS.md — Technical Project Documentation

Complete technical reference for the **PS2-NANA-decomp** project. Read this
before touching any code, tool, or binary. Update it when workflows change.

---

## 1. Project goal, legal constraints, non-goals

**Goal.** Produce a readable, documented, progressively buildable C/C++
reconstruction of the PlayStation 2 game *Nana* (SLPM_659.14, Japan, Konami)
through reverse engineering, for education, preservation, and research.

**Legal constraints.**

- Work exclusively from a **personal, legally obtained dump**. Never share it.
- The repository contains **zero copyrighted game data**: no ISO, no extracted
files, no assets. Only reconstructed code, docs, symbol maps, metadata.
- Documenting structural facts (offsets, sizes, headers, control flow) is
fine; reproducing content is not.
- MIT license applies to reconstructed code/docs only (see `LICENSE` note).

**Non-goals.** No asset redistribution, no full-game data builds, no piracy
facilitation, no claims on the original IP.

---

## 2. Required tools and versions

Verified on Arch Linux (2026-10-14):

| Tool | Version | Role |
| --- | --- | --- |
| git | 2.56.0 | version control |
| github-cli (`gh`) | 2.102.0 | repo management |
| python + pip | 3.14.8 / 26.2.1 | analysis & progress tooling |
| binutils (readelf/objdump) | 2.47 | ELF inspection, MIPS disassembly |
| file | 5.48 | format identification (note: can misreport stripped state) |
| Ghidra | 12.1.2 | interactive decompilation (EE MIPS-R5900) |
| rizin + rz-ghidra | 0.8.2 / 0.8.0 | scriptable disasm/decomp, batch analysis |
| 7-Zip (`7z`) | 26.04 | ISO listing/extraction |
| fuseiso + fusermount | 2.9.9 | read-only ISO mount |
| PCSX2 | pending (AUR) | behavioral verification (Phase 6) |
| ps2dev toolchain | pending | EE/MIPS rebuild (Phase 6, open source) |

If versions drift, update this table and `docs/environment.md`.

---

## 3. Arch Linux environment setup

Executed baseline (reproducible reference):

```bash
sudo pacman -Syu
sudo pacman -S --needed base-devel git github-cli python python-pip
sudo pacman -S --needed binutils file          # readelf-less does not exist; readelf is in binutils
sudo pacman -S --needed ghidra
sudo pacman -S --needed rizin rz-ghidra
sudo pacman -S --needed 7zip fuseiso
# Pending (Phase 6): PCSX2 via AUR; ps2dev toolchain from source.
```

Verification:

```bash
git --version && gh --version && python --version
readelf --version | head -1 && objdump --version | head -1
7z | head -3 && fusermount --version
```

---

## 4. ISO/dump extraction workflow (Phase 1)

Principles: **read-only by default**, analyze in place, extract only what a
specific task needs, keep everything **outside the repository** (use `~/work/nana`).

1. Record integrity (stays local, never committed):

```bash
   sha256sum /path/to/your/nana.iso | tee ~/nana_sha256.txt
```

2. List without extracting: `7z l /path/to/your/nana.iso`
3. Mount read-only: `fuseiso /path/to/your/nana.iso ~/mnt/nana` /
`fusermount -u ~/mnt/nana`
4. Extract selected files to the work dir:

```bash
   7z e /path/to/your/nana.iso SYSTEM.CNF SLPM_659.14 OVL/SHOP2R.BIN -o~/work/nana
```

5. Document findings in `docs/disc-map.md` (paths, sizes, hashes, hypotheses).

Dump facts (this project): serial **SLPM_659.14**, Japan, KCEJ, master
2005-02-02; boot `BOOT2 = cdrom0:\SLPM_659.14;1`, NTSC, VER 1.01; ISO
SHA-256 `4491920966f6cc5412ab5e58d01e4ba7eabb1fc4e677c7d58254d3fae8120419`.

---

## 5. PS2 executable identification and loading

Verified facts for `SLPM_659.14`:

- ELF32, **little endian**, MIPS R5900 (`flags 0x20924000`, eabi64, mips3),
statically linked, **effectively stripped** (`.symtab`/`.strtab` present but
zero-sized — `file`'s "not stripped" report is misleading).
- Entry point **0x00100008**; `$gp = 0x005ab270` (from `.reginfo`).
- **Merged single PROGBITS section** at 0x00100000 (file 0x4a3680, mem
0xa7f800 → ~6 MB BSS). No .text/.rodata/.data/.bss split — code/data
separation is heuristic work for rizin/Ghidra.
- 8 zero-fill LOAD segments at **0x00b7f800** with different mem sizes =
reserved **overlay (OVL/*.BIN) load regions**; RW marker at 0x01a7f900.
- Identify any new binary with:

```bash
  file <bin> && readelf -h <bin>
```

---

## 6. ELF analysis (Phase 2)

Standard pass (local dump copy only):

```bash
readelf -h <elf>     # header
readelf -S <elf>     # sections
readelf -s <elf>     # symbols (empty for the main ELF — see §8)
readelf -l <elf>     # segments / load map
readelf -p .comment <elf>   # compiler ID → "MW MIPS C Compiler (2.4.1.01)"
readelf -x .reginfo <elf>   # $gp and coprocessor masks
```

- Inventory via rizin: `rizin -q -c 'aaa; aflc' <elf>` → **2348 functions**
(estimate; merged section makes auto-analysis noisy — confirm in Ghidra).
- Export function list: `rizin -q -c 'aaa; afl' <elf> > functions_rizin.txt`.
- Strings pass for SDK/asset hints:

```bash
  strings -n 6 <elf> | grep -E '\.(nkt|nkf|nkp|rws|pss|irx|bin|cnf)' | sort -u
  strings -n 6 <elf> | grep -iE 'sce[A-Z]|libpad|libsnd|mcman|cdvd' | sort -u
```

Known surface (see `docs/executable.md`): Konami "PsII" wrappers
(`PsIIlibcdvd 3000`, `PsIIlibpad 3010`); `/iop/*.irx` loaded by path;
`sceMpeg`/`libipu` video; `SceSifrpc*`, `SceKernel*`, `SceCd*`; story
packages named `A-XX-YY.nkp`.

---

## 7. MIPS instruction analysis (R5900 + Metrowerks CodeWarrior)

Target CPU: R5900 (MIPS-III + MMI), 64-bit GPRs, 128-bit `$vf*`, EE FPU.

**Compiler: Metrowerks CodeWarrior 2.4.1.01** — NOT GCC. Implications:

- Expect CW codegen idioms: `bl`/`b` branch-long forms in addition to
`jal`/`j`; CW-specific prologue/epilogue habits; different delay-slot and
`li`/`move` pseudo-op lowering than GCC.
- Catalog these idioms empirically in `notes/analysis/codegen.md` with
disassembly excerpts **before** starting bulk decompilation.
- Phase 6 verification must tolerate CW-vs-GCC structural divergence:
behavioral equivalence and CFG comparison are primary; instruction-level
diff is secondary.
- Build will use GCC (ps2dev); matched = same behavior/structure, not same
bytes.

General R5900 notes:

- Prologue: `addiu $sp,$sp,-N`, saves via `sq`/`sw` to `($sp+off)`; epilogue
restores `$ra`, `jr $ra` + delay-slot `nop` (or `ld`/`lq` tail).
- MMI in render/math code: `lqc2/sqc2`, `pmaddw`, `pmulth`, `mfsa/mtsa`.
- Idioms to catalog: 64-bit mul/div `mfhi/mflo` pairs, unaligned `lwl/lwr`,
inline `lq/sq` copy loops, $gp-relative global access from 0x005ab270.

---

## 8. Symbol recovery and naming conventions

Effectively stripped → all symbols recovered during analysis.

- **SDK/kernel functions**: match against PS2SDK/open-source equivalents
(imports visible via `Sce*` strings and stubs; IOP modules listed in §6).
- **Naming scheme** (enforced):
- Known: `ModuleName__FunctionName` (PascalCase module, camelCase fn),
e.g. `Render__DrawSprite`, `Save__WriteBlock`.
- Unknown: `func_00123ABC` (8 hex digits = virtual address).
- Data: `g_CamelCase` globals, `s_CamelCase` statics, `D_00123ABC` unlabeled.
- Structs: `ModuleName__StructName` → typedef `ModuleNameStructName`.
- Enums: `ModuleName__EnumName`, members `MODULE_ENUM_MEMBER`.
- Mapping lives in `symbols/` (see `symbols/README.md`); every rename is a
commit: `symbols: rename func_0019A3F0 → Audio__InitSpu2 (VERIFIED)`.

---

## 9. Function matching and decompilation workflow

Per function, in order:

1. **Inventory** — address, size, callers/callees (rizin `afl`, `axt`).
2. **Hypothesis** — name + one-line purpose into symbol map (status: unknown).
3. **Decompile** — Ghidra interactive / rz-ghidra batch; reconstruct in
`src/` preserving boundaries; label RECONSTRUCTED.
4. **Understand** — document args/return/globals/side effects in header
comment; promote to PARTIAL / VERIFIED.
5. **Verify** — CFG + stack-frame comparison vs original disassembly;
behavioral checks under PCSX2 (Phase 6). Only then increment `matched`.
6. Update `progress.yaml` → chart in `docs/`.

Status header template for every reconstructed function:

```c
/* STATUS: VERIFIED | PARTIAL | RECONSTRUCTED | STUB
 * ADDRESS: 0x00123ABC  SIZE: 0x58
 * PURPOSE: one-line description */
```

---

## 10. Data structures, globals, memory layout

- Merged section → locate data by scanning for pointers into the image range
(0x00100000–0x00b7f800) and $gp-relative references (base 0x005ab270).
- Document globals in `docs/` as: name, address, size, inferred type,
evidence (disasm refs). Evidence, never raw dumps.
- Overlay regions at 0x00b7f800: document per-OVL usage when analyzed.

---

## 11. Asset format identification and extraction methodology (Phase 5)

- Check **known PS2 formats** first: TIM2 (`TIM2`), VAG, ADX, PSS/SS2 video
(`OPMV.PSS` confirmed), VU micro-programs, PACK/ARC-style containers.
- Project-specific formats to document: `.NKP` packages (root + `A-XX-YY.nkp`
story scheme + `ALLSC.NKP` 1.65 GB), `.RWS` audio streams (Konami `RWA.IRX`
driver), `.NKT` config, `.NKF` dialogs, OVL `.BIN` load format.
- Methodology per format: magic → header fields → offset/size table → parser
in `tools/` → document structure in `docs/asset-formats.md`.
- Parsers read the local dump path via argument/env var; outputs stay outside
the repo. Never commit extracted assets or full-file fixtures.

---

## 12. Reconstructed source organization (defined fully in Phase 3)

Preview:

```javascript
src/
  common/        # shared types, Sony/Konami shims, macros
  system/        # init, main loop, memory
  render/        # GS/VU drawing
  audio/         # RWA driver client, streams
  input/         # pad (PsIIlibpad)
  scene/         # state machine, story scripting
  ui/            # menus, HUD, dialogs (.nkf)
  save/          # memory-card (MCMAN/MCSERV)
  asset/         # .nkp/.rws/.nkt loaders
  overlay/       # OVL modules (loaded at 0x00b7f800)
```

---

## 13. Build and verification workflow (Phase 6)

- Build with open-source **ps2dev** toolchain (GCC MIPS + ps2sdk) — no
proprietary Sony SDK. Install only when reconstruction compiles.
- Link script reproduces original layout (0x00100000 base, $gp 0x005ab270)
so address-based comparison stays meaningful.
- Verification (non-infringing): per-function CFG/stack-frame comparison,
PCSX2 runtime logging at reconstructed addresses, mismatch log in
`docs/build.md`. Remember CW-vs-GCC divergence (§7).

---

## 14. Git workflow, commits, branches

See `CONTRIBUTING.md`. Summary: Conventional Commits in English; `main`
protected; branches `decomp/<area>`, `docs/<topic>`, `tools/<name>`; no
binaries; no force-push to `main`.

---

## 15. Progress tracking rules

- `progress.yaml` is the **single source of truth**; `TODO.md` mirrors it.
- Update on every merged unit of work.
- `tools/` hosts the generator rendering the chart from `progress.yaml` into
`docs/progress.svg`; regenerate on update, never hand-edit.
- Metrics: functions identified/decompiled/matched/remaining, modules,
assets understood, milestone completion.

- Maintenance snapshot 2026-10-15: `Schedule__WalkSlots` reconstructed;
  scheduler symbols renamed per Ghidra; duplicate `schedule_update.c` removed.
- Maintenance snapshot 2026-10-15: `Schedule__Update` reconstructed;
  crt0 BSS range fixed; raw files retained pending one-by-one replacement.

---

## 16. Known issues, assumptions, resolved questions

**Resolved.**

- Endianness: ELF is **little endian** (confirmed via `readelf -h`).
- Symbols: `.symtab`/`.strtab` empty → effectively stripped (despite `file`).
- Compiler: **Metrowerks CodeWarrior (MW MIPS C Compiler 2.4.1.01)** via
`.comment`.
- Engine: Konami in-house (KCEJ copyright; `konami.nkt`; PsII wrappers).
- Boot: SYSTEM.CNF → `SLPM_659.14`; entry 0x00100008; $gp 0x005ab270.
- Video via `sceMpeg`/`libipu`; audio via custom Konami `RWA.IRX`.

**Open / pending.**

- True function count (Ghidra confirmation of rizin's 2348 estimate).
- OVL format: MWo3 identified (see docs/overlay-format.md); header field semantics pending verification.
- `RTFSSIOP.IRX` purpose (strings pass in Phase 5).
- `ALLSC.NKP` self-contained vs indexing external `.RWS` voice files.
- CW codegen idioms catalog (`notes/analysis/codegen.md`).

**Assumptions to validate.**

- OVL slots at 0x00b7f800 reused per overlay (CONFIRMED: MWo3 load address field = 0x00b7f800).
- MWo3 header @0x04 = header size in 16-byte units (8→0x80, 7→0x70) — strong hypothesis, verify on all overlays.
- `A-XX-YY.nkp` = episode/scene package indexing (from embedded strings).
- ELF entry region 0x100000–0x10007F = pointer table into 0x7000xxxx (system interface), NOT code; real startup code at 0x100080.

---

*Maintained by the project supervisor and AI assistant. Last updated: 2026-10-15.*

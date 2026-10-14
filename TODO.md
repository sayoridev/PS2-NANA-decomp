# TODO — Project State

> This file must always reflect the **real** state of the project.
> Update it with every meaningful change. Machine-readable mirror: `progress.yaml`.

_Last updated: 2026-10-14 (Phase 0)_

## Current phase

**Phase 0 — Project Setup** (in progress)

## Completed

- [x] Repository created: `sayoridev/PS2-NANA-decomp` (public)
- [x] Arch Linux environment prepared:
  - git 2.56.0, github-cli 2.102.0, python 3.14.8 + pip 26.2.1
  - binutils 2.47 (readelf/objdump), file 5.48
  - Ghidra 12.1.2
  - rizin 0.8.2 + rz-ghidra 0.8.0
  - 7-Zip 26.04, fuseiso 20070708 (fusermount 2.9.9)
- [x] Initial documentation committed (README, AGENTS, CONTRIBUTING, LICENSE, .gitignore, progress.yaml)

## In progress

- [ ] Initial git history + push of documentation baseline

## Pending

- [ ] Verify local dump location and integrity (sha256, personal copy — never committed) — Phase 1
- [ ] Dump inspection: ISO structure, SYSTEM.CNF, main executable — Phase 1
- [ ] ELF analysis: sections, symbols, compiler/SDK hints — Phase 2
- [ ] Function inventory and address map — Phase 2
- [ ] Progress chart generator in `tools/` — Phase 0/7

## Blocked

- [ ] PCSX2 emulator (verification, Phase 6) — not in official Arch repos; install
      from AUR when the verification phase approaches. No action needed now.

## Discovered issues

- None yet.

## Next recommended actions

1. Copy the delivered initial file set into the repo, review, commit, push.
2. Locate your personal dump; report its path, size, and sha256 (stays local).
3. Begin Phase 1: dump inspection with `7z l` / `fuseiso` read-only mount.

## Milestones

| Milestone | Status | Progress |
|-----------|--------|----------|
| Phase 0 — Project setup | in progress | ~80% |
| Phase 1 — Dump inspection | pending | 0% |
| Phase 2 — Executable analysis | pending | 0% |
| Phase 3 — Decompilation foundation | pending | 0% |
| Phase 4 — Game systems | pending | 0% |
| Phase 5 — Assets and formats | pending | 0% |
| Phase 6 — Build and verification | pending | 0% |
| Phase 7 — Continuous progress | ongoing | — |

_Percentages are meaningful only once measurable units exist (functions identified, assets documented, etc.)._

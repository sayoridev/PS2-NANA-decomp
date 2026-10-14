# PS2-NANA-decomp

Decompilation of the PlayStation 2 game **Nana** (Bandai, 2005), for
computer science education, video game preservation, and technical research.

> **LEGAL DISCLAIMER**
>
> This repository contains **no original game data**. It hosts only
> independently reconstructed source code, technical documentation, build
> scripts, symbol maps, and non-copyrightable metadata produced by reverse
> engineering.
>
> **You must own a physical copy of the game and produce your own personal
> dump to work with or verify this project.** Do not request, upload, share,
> or redistribute game binaries, ISO images, assets, music, or videos.
> See [CONTRIBUTING.md](CONTRIBUTING.md) for binding rules.

## Requirements

- A legally owned personal dump of the PS2 game (never committed to this repo)
- Arch Linux (other distros work with equivalent packages)
- Tools (see [docs/environment.md](docs/environment.md) for exact versions):
  - `git`, `github-cli`
  - `python` + `pip` (analysis scripts)
  - `binutils` (`readelf`, `objdump`), `file`
  - Ghidra (interactive decompilation)
  - rizin + rz-ghidra (scriptable disassembly/decompilation)
  - `7zip`, `fuseiso` (ISO inspection, read-only mount)
  - PCSX2 (verification — AUR, set up in a later phase)

## Repository layout

| Path | Purpose |
|------|---------|
| `docs/` | Technical documentation: environment, disc map, executable analysis, conventions, asset formats, build |
| `notes/` | Raw reverse-engineering notes, one file per area/module |
| `src/` | Reconstructed source code (created in Phase 3) |
| `symbols/` | Symbol maps and address-to-function mappings |
| `tools/` | Python analysis/progress tooling |
| `progress.yaml` | Machine-readable single source of truth for progress |
| `AGENTS.md` | Full technical project documentation |
| `TODO.md` | Live project state |

## Progress

See [TODO.md](TODO.md) and `progress.yaml`. A generated progress chart is
maintained in `docs/` as the project advances.

## License

Reconstructed source code and original documentation are licensed under the
MIT License ([LICENSE](LICENSE)). This license covers **only** the original
work in this repository — it grants no rights over the game, its assets, or
any copyrighted material belonging to the original rights holders.

## Non-goals

- Distributing the game or any of its assets in any form
- Facilitating piracy in any way
- Claiming ownership of the original game or its intellectual property

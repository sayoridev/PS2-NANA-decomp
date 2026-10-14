# Contributing

Thank you for your interest in this project. Before anything else, read and
accept the rules below — they are **binding** and PRs that violate them will
be closed without discussion.

## Legal rules (hard requirements)

1. **You must own a physical copy of the game** and use only your own personal
   dump for analysis or verification.
2. **Never** commit, upload, link, or redistribute:
   - ISO images, disc dumps, or any part thereof
   - Extracted game files (executables, modules, textures, models, audio, video, scripts)
   - Proprietary SDK files or leaked material
3. Commit only: reconstructed source code, technical documentation, build
   scripts, symbol maps, analysis notes, and non-copyrightable metadata
   (sizes, hashes, offsets, structure descriptions).
4. Small structural facts discovered by analysis (offsets, sizes, magic bytes,
   control flow) may be documented. Complete asset data may not.
5. If you are unsure whether something is safe to commit, **ask before
   committing**.

## Code rules

- All code, identifiers, comments, docs, and commit messages in **English**.
- Preserve original function boundaries and behavior where practical.
- Every reconstructed function must be labeled with its verification status:
  - `VERIFIED` — confirmed to match the original (structure + behavior)
  - `PARTIAL` — understood in part; clearly marked unknown regions
  - `RECONSTRUCTED` — faithful rewrite not yet verified against the original
  - `STUB` — placeholder, minimal or empty body
- Never commit unverified or placeholder code without its status marker.
- Prefer readable, well-commented C/C++ over clever one-liners.

## Commit conventions

Follow [Conventional Commits](https://www.conventionalcommits.org/):

    <type>(<scope>): <short summary in English, imperative mood>

- Types: `docs`, `tools`, `decomp`, `analysis`, `symbols`, `build`, `fix`, `chore`
- Examples:
  - `decomp(main): reconstruct InitSystem startup path (PARTIAL)`
  - `docs(disc): add volume descriptor map from personal dump analysis`
  - `tools(progress): add chart generator from progress.yaml`

Keep commits small, descriptive, and reviewable. One logical unit per commit.

## Git workflow

- `main` is protected: work happens on short-lived branches
  (`decomp/<area>`, `docs/<topic>`, `tools/<name>`).
- Never force-push to `main`. Never commit binary blobs (see `.gitignore`).
- Open a PR for review before merging anything non-trivial.

## Progress tracking

- Update `progress.yaml` with every merged change, then mirror the summary in
  `TODO.md`.
- The progress chart in `docs/` is generated from `progress.yaml` — never edit
  the chart by hand.

## Conduct

Respectful, technical discussion only. This is a preservation and education
project, not a piracy channel.

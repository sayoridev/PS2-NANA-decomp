# Tools

Python analysis and progress tooling. All scripts:

- take the local dump/ELF path as an argument (never hardcode, never commit),
- write outputs outside the repo or as structural metadata only,
- are documented with a module docstring and `--help`.

Planned: `inventory.py` (function stats), `progress_chart.py`
(renders `progress.yaml` → `docs/progress.svg`).

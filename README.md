# Tools

Current scripts:

- `progress_chart.py` — renders `progress.yaml` to `docs/progress.svg`.
- `hexdump.py` — local binary inspection helper.
- `parse_nkp.py` — parser for NKP asset archives when a local dump is supplied.
- `read_strings.py` — printable-string extraction helper for local binaries.

Rules for all scripts:

- take the local dump/ELF path as an argument (never hardcode, never commit),
- write outputs outside the repo or as structural metadata only,
- are documented with a module docstring and `--help` where applicable.

Planned: `inventory.py` (function stats).

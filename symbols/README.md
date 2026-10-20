# Symbols

Current symbol file:

- `functions.txt` — TSV table with columns:
`address`, `size`, `status`, `name`, `comment`

Planned exports, once Ghidra boundaries are stable:

- `functions.csv` — spreadsheet-friendly function table
- `globals.txt` / `globals.csv` — data-only symbol tables

All addresses are virtual addresses of the original EE ELF. Function sizes are
still incomplete until the Ghidra boundary pass is exported.

Status values: `unknown | hypothesis | VERIFIED | PARTIAL | RECONSTRUCTED | STUB`.
# Environment — Arch Linux

Verified 2026-10-14 on Arch Linux. See `AGENTS.md` §3 for the authoritative
tool/version table and setup commands.

## Notes

- `readelf-less` does **not** exist as an Arch package; `readelf` ships with
  `binutils`.
- PCSX2 is **not** in the official repositories — install from AUR when
  Phase 6 (verification) begins.
- The ps2dev open-source toolchain is built from source in Phase 6; do not
  install the proprietary Sony SDK.
- Verify tool availability at any time with:

```bash
git --version && gh --version && python --version
readelf --version | head -1 && objdump --version | head -1
7z | head -3 && fusermount --version
```

#!/usr/bin/env python3
"""Parse an NKP (Konami Package) header + index from a raw file or ELF vaddr.

Usage:
    python3 parse_nkp.py <file> <offset> [--elf] [--max N]

With --elf, <offset> is a vaddr in SLPM_659.14 (delta 0xFFE80 applied).
Without --elf, <offset> is a plain file offset (e.g. inside an extracted .NKP).
--max N limits printed entries (default 16; use a huge N for all).
"""
import sys
import struct

DELTA = 0xFFE80

def parse(data, off, max_entries=16):
    if data[off:off+3] != b"NKP" or data[off+3] != 0x1A:
        print(f"!! no NKP magic at 0x{off:X}")
        return
    base    = struct.unpack_from("<I", data, off + 4)[0]
    flags   = struct.unpack_from("<I", data, off + 8)[0]
    count   = struct.unpack_from("<I", data, off + 0xC)[0]
    print(f"magic   : NKP 0x1A")
    print(f"base    : 0x{base:08X}")
    print(f"flags   : 0x{flags:08X}")
    print(f"entries : {count} (index ends at 0x{0x10 + count*8:X})")
    print()
    print("idx | id     | offset")
    prev_off = 0
    for i in range(min(count, max_entries)):
        eid, eoff = struct.unpack_from("<II", data, off + 0x10 + i * 8)
        stride = f" (+0x{eoff - prev_off:X})" if i else ""
        print(f"{i:3} | 0x{eid:04X} | 0x{eoff:08X}{stride}")
        prev_off = eoff
    if count > max_entries:
        print(f"... ({count - max_entries} more entries)")

def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    path = args[0]
    off = int(args[1], 16)
    max_entries = 16
    for i, a in enumerate(sys.argv[1:]):
        if a == "--max":
            max_entries = int(sys.argv[1:][i + 1])
    if "--elf" in sys.argv:
        off -= DELTA
    data = open(path, "rb").read()
    parse(data, off, max_entries)

if __name__ == "__main__":
    main()

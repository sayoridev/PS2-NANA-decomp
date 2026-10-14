#!/usr/bin/env python3
"""Read null-terminated ASCII strings at given vaddrs from the ELF.
Usage: python3 read_strings.py <binary> <vaddr1> <vaddr2> ..."""
import sys

DELTA = 0xFFE80  # vaddr = file_off + DELTA

def read_string(data, vaddr):
    off = vaddr - DELTA
    end = off
    while end < len(data) and data[end] != 0:
        end += 1
    return data[off:end].decode("ascii", errors="replace")

def main():
    data = open(sys.argv[1], "rb").read()
    for a in sys.argv[2:]:
        v = int(a, 16)
        s = read_string(data, v)
        print(f"0x{v:08X}: {s!r}")

if __name__ == "__main__":
    main()

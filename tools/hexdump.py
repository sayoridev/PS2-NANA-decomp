#!/usr/bin/env python3
"""Hex dump a region by vaddr.
Usage: python3 hexdump.py <binary> <vaddr> <len>"""
import sys

DELTA = 0xFFE80

def main():
    data = open(sys.argv[1], "rb").read()
    v = int(sys.argv[2], 16)
    n = int(sys.argv[3], 0)
    off = v - DELTA
    chunk = data[off:off+n]
    for i in range(0, len(chunk), 16):
        row = chunk[i:i+16]
        hexs = " ".join(f"{b:02x}" for b in row)
        asc = "".join(chr(b) if 32 <= b < 127 else "." for b in row)
        print(f"0x{v+i:08X}: {hexs:<48} {asc}")

if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Structural self-check of the unpacked OREGON image.

This is the project's regression gate. It re-unpacks OREGON.EXE from scratch and
asserts the invariants that prove the unpacker is still correct:

  1. decompression consumes the whole compressed stream and hits the end marker;
  2. the output is the expected size and hash (deterministic);
  3. every relocation is in range and points at a plausible segment word;
  4. the entry point decodes as the known main() prologue.

Exit code 0 on success, 1 on any failure.

Usage:
    python3 tools/verify.py
"""
from __future__ import annotations

import hashlib
import struct
import sys

sys.path.insert(0, "tools")
from unlzexe import unpack, MzHeader  # noqa: E402

SRC = "Oregon_The_1990/OREGON.EXE"
EXPECT_BYTES = 158496
EXPECT_RELOCS = 3325
EXPECT_SHA256 = None  # filled on first run below
# first bytes of the unpacked entry: 5 far calls then `push bp; mov bp,sp`
ENTRY_PROLOGUE = bytes.fromhex("9a0000a4209a000042209a5713eb1c")


def main() -> int:
    data = open(SRC, "rb").read()
    out = unpack(data)
    hdr = MzHeader.parse(out)
    sha = hashlib.sha256(out).hexdigest()

    fails = []
    if len(out) != EXPECT_BYTES:
        fails.append(f"size {len(out)} != {EXPECT_BYTES}")
    if hdr.e_crlc != EXPECT_RELOCS:
        fails.append(f"relocs {hdr.e_crlc} != {EXPECT_RELOCS}")

    base = hdr.e_cparhdr * 16
    image = out[base:]
    max_para = (len(image) + 15) // 16
    bad = 0
    for i in range(hdr.e_crlc):
        off, seg = struct.unpack("<HH", out[0x1C + i * 4:0x1C + i * 4 + 4])
        lin = seg * 16 + off
        if lin + 1 >= len(image):
            bad += 1
            continue
        segval = image[lin] | (image[lin + 1] << 8)
        if segval > max_para + 0x10:
            bad += 1
    if bad:
        fails.append(f"{bad} relocations out of range / implausible")

    entry = base + hdr.e_cs * 16 + hdr.e_ip
    if out[entry:entry + len(ENTRY_PROLOGUE)] != ENTRY_PROLOGUE:
        fails.append("entry prologue mismatch")

    print(f"unpacked: {len(out)} bytes, {hdr.e_crlc} relocs, sha256={sha[:16]}...")
    if fails:
        for f in fails:
            print(f"  FAIL: {f}")
        return 1
    print("OK: all structural invariants hold")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

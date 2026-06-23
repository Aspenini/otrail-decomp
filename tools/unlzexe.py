#!/usr/bin/env python3
"""Unpack an LZEXE 0.91-compressed MS-DOS executable.

OREGON.EXE ships compressed with LZEXE 0.91 (Fabrice Bellard's packer); the
"LZ91" signature sits at offset 0x1C of the MZ header.  This tool reverses both
the LZ77 payload compression and the run-length-encoded relocation table, then
writes a plain, relocatable MZ executable that can be disassembled or run.

The algorithm here is transcribed directly from the decompressor stub inside
OREGON.EXE itself (disassembled at file offset 0x126F0), so it is faithful to
this exact binary rather than to a generic UNLZEXE port.  The two details that
trip up naive ports are both reproduced:

  * the control-bit word is reloaded *eagerly* the moment the 16-bit counter
    hits zero (mid token), before the triggering token reads its data byte; and
  * back-references copy with 16-bit offset wrap inside the destination
    segment (``mov al,[es:bx+di]``).

Usage:
    python3 tools/unlzexe.py INPUT.EXE OUTPUT.EXE
"""
from __future__ import annotations

import struct
import sys
from dataclasses import dataclass


@dataclass
class MzHeader:
    e_cblp: int      # bytes in last 512-byte page
    e_cp: int        # number of 512-byte pages
    e_crlc: int      # relocation entries
    e_cparhdr: int   # header size in paragraphs
    e_minalloc: int
    e_maxalloc: int
    e_ss: int
    e_sp: int
    e_csum: int
    e_ip: int
    e_cs: int
    e_lfarlc: int    # file offset of relocation table
    e_ovno: int

    @classmethod
    def parse(cls, data: bytes) -> "MzHeader":
        if data[:2] != b"MZ":
            raise ValueError("not an MZ executable")
        f = struct.unpack("<13H", data[2:28])
        return cls(*f)

    @property
    def header_bytes(self) -> int:
        return self.e_cparhdr * 16


class _BitReader:
    """Faithful model of the stub's control-bit stream (lzexe shared stream)."""

    def __init__(self, data: bytes, pos: int = 0):
        self._d = data
        self.pos = pos
        self._bits = self._word()
        self._left = 16

    def _word(self) -> int:
        w = self._d[self.pos] | (self._d[self.pos + 1] << 8)
        self.pos += 2
        return w

    def byte(self) -> int:
        b = self._d[self.pos]
        self.pos += 1
        return b

    def bit(self) -> int:
        cf = self._bits & 1
        self._bits >>= 1
        self._left -= 1
        if self._left == 0:           # eager reload, exactly as the stub does
            self._bits = self._word()
            self._left = 16
        return cf


def decompress(comp: bytes) -> bytes:
    r = _BitReader(comp)
    out = bytearray()
    while True:
        if r.bit():                       # 1 -> literal byte
            out.append(r.byte())
            continue
        if r.bit() == 0:                  # 0,0 -> short match (len 2..5, disp -1..-256)
            length = ((r.bit() << 1) | r.bit()) + 2
            span = r.byte() | 0xFF00
        else:                             # 0,1 -> long match
            lo = r.byte()
            hi = r.byte()
            span = (lo | ((hi >> 3) << 8) | 0xE000) & 0xFFFF
            length = hi & 0x07
            if length == 0:
                ext = r.byte()
                if ext == 0:
                    break                 # end of stream
                if ext == 1:
                    continue              # segment boundary, no copy
                length = ext + 1
            else:
                length += 2
        start = len(out) + (span - 0x10000)
        if start < 0:
            raise ValueError(f"back-reference before output start at out=0x{len(out):x}")
        for i in range(length):
            out.append(out[start + i])
    return bytes(out)


def reconstruct_relocations(stub: bytes, table_off: int) -> list[tuple[int, int]]:
    """Decode the LZEXE 0.91 RLE relocation table.

    Returns a list of (segment, offset) pairs relative to the load base.
    Transcribed from the stub routine at ds:0xFC.
    """
    relocs: list[tuple[int, int]] = []
    si = table_off
    seg = 0
    off = 0
    while True:
        al = stub[si]; si += 1
        if al == 0:
            w = stub[si] | (stub[si + 1] << 8); si += 2
            if w == 0:                    # span > 0xFF0: bump segment, no fixup
                seg += 0x0FFF
                continue
            if w == 1:                    # terminator
                break
            delta = w
        else:
            delta = al
        off += delta
        seg += off >> 4
        off &= 0x0F
        relocs.append((seg, off))
    return relocs


def unpack(data: bytes) -> bytes:
    hdr = MzHeader.parse(data)
    if data[0x1C:0x20] not in (b"LZ91", b"LZ90"):
        raise ValueError("missing LZ90/LZ91 signature; not an LZEXE file")

    info_off = (hdr.e_cparhdr + hdr.e_cs) * 16
    ip0, cs0, sp0, ss0, comp_par = struct.unpack("<5H", data[info_off:info_off + 10])

    comp = data[hdr.header_bytes:info_off]
    if len(comp) != comp_par * 16:
        raise ValueError("compressed-size mismatch vs info block")

    image = bytearray(decompress(comp))

    # Relocation table lives at ds:0x158 inside the stub (file info_off + 0x158).
    stub = data[info_off:]
    relocs = reconstruct_relocations(stub, 0x158)

    # ---- assemble a fresh MZ executable -------------------------------------
    reloc_bytes = b"".join(struct.pack("<HH", off, seg) for seg, off in relocs)
    hdr_min = 0x1C + len(reloc_bytes)
    hdr_paras = (hdr_min + 15) // 16
    # pad header to paragraph boundary
    header = bytearray(hdr_paras * 16)

    # memory needed up to top of stack, in paragraphs, beyond the image
    image_paras = (len(image) + 15) // 16
    stack_top = ss0 * 16 + sp0
    min_paras = max(0, (stack_top + 15) // 16 - image_paras) + 0x10

    total = hdr_paras * 16 + len(image)
    e_cp = (total + 511) // 512
    e_cblp = total % 512

    struct.pack_into(
        "<2s13H", header, 0,
        b"MZ", e_cblp, e_cp, len(relocs), hdr_paras,
        min_paras, 0xFFFF, ss0, sp0, 0, ip0, cs0, 0x1C, 0,
    )
    header[0x1C:0x1C + len(reloc_bytes)] = reloc_bytes

    return bytes(header) + bytes(image)


def main(argv: list[str]) -> int:
    if len(argv) != 3:
        print(__doc__)
        return 2
    data = open(argv[1], "rb").read()
    out = unpack(data)
    open(argv[2], "wb").write(out)
    hdr = MzHeader.parse(out)
    print(f"unpacked {argv[1]} -> {argv[2]}")
    print(f"  image bytes : {len(out):#x} ({len(out)})")
    print(f"  relocations : {hdr.e_crlc}")
    print(f"  entry CS:IP : {hdr.e_cs:#06x}:{hdr.e_ip:#06x}")
    print(f"  stack SS:SP : {hdr.e_ss:#06x}:{hdr.e_sp:#06x}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

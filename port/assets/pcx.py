#!/usr/bin/env python3
"""Decode ZSoft PCX images (8-bit, RLE) — the format behind the game's `.256`,
`.pcc`, and `.PCL`-contained images.

Standalone (stdlib only). Used by the asset pipeline to convert the original art
into something the port can bundle, and to verify our understanding of the data.

    python3 port/assets/pcx.py INPUT.pcx OUTPUT.png
"""
from __future__ import annotations

import struct
import sys
import zlib


def decode_pcx(data: bytes):
    """Return (width, height, indexed_bytes, palette) for an 8-bit PCX.

    palette is a list of 256 (r, g, b) tuples.
    """
    if data[0] != 0x0A:
        raise ValueError("not a PCX (bad manufacturer byte)")
    enc = data[2]
    bpp = data[3]
    xmin, ymin, xmax, ymax = struct.unpack_from("<4H", data, 4)
    nplanes = data[65]
    bytes_per_line = struct.unpack_from("<H", data, 66)[0]
    if bpp != 8 or nplanes != 1:
        raise ValueError(f"only 8-bit single-plane PCX supported (bpp={bpp}, planes={nplanes})")
    w = xmax - xmin + 1
    h = ymax - ymin + 1

    # RLE-decode the pixel data (file offset 128 .. palette).
    out = bytearray()
    total = bytes_per_line * h
    p = 128
    if enc == 1:
        while len(out) < total:
            b = data[p]; p += 1
            if (b & 0xC0) == 0xC0:           # run: count in low 6 bits
                count = b & 0x3F
                val = data[p]; p += 1
                out.extend([val] * count)
            else:
                out.append(b)
    else:                                    # uncompressed
        out = bytearray(data[p:p + total]); p += total

    # 256-colour palette: trailing 0x0C marker + 768 bytes at end of file.
    pal = [(0, 0, 0)] * 256
    if len(data) >= 769 and data[-769] == 0x0C:
        praw = data[-768:]
        pal = [(praw[i * 3], praw[i * 3 + 1], praw[i * 3 + 2]) for i in range(256)]

    # Trim each scanline's padding (bytes_per_line may exceed width).
    pixels = bytearray(w * h)
    for y in range(h):
        row = out[y * bytes_per_line: y * bytes_per_line + w]
        pixels[y * w: y * w + len(row)] = row
    return w, h, bytes(pixels), pal


def write_png(path: str, w: int, h: int, rgb: bytes):
    """Write a minimal RGB PNG (stdlib zlib only)."""
    def chunk(tag, payload):
        c = tag + payload
        return struct.pack(">I", len(payload)) + c + struct.pack(">I", zlib.crc32(c) & 0xFFFFFFFF)

    raw = bytearray()
    for y in range(h):
        raw.append(0)                                   # filter: none
        raw.extend(rgb[y * w * 3:(y + 1) * w * 3])
    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(raw), 9))
    png += chunk(b"IEND", b"")
    open(path, "wb").write(png)


def to_rgb(w: int, h: int, indexed: bytes, pal) -> bytes:
    out = bytearray(w * h * 3)
    for i, idx in enumerate(indexed):
        r, g, b = pal[idx]
        out[i * 3], out[i * 3 + 1], out[i * 3 + 2] = r, g, b
    return bytes(out)


def main(argv):
    if len(argv) != 3:
        print(__doc__)
        return 2
    w, h, idx, pal = decode_pcx(open(argv[1], "rb").read())
    write_png(argv[2], w, h, to_rgb(w, h, idx, pal))
    print(f"{argv[1]}: {w}x{h} 8-bit PCX -> {argv[2]}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

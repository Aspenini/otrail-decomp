#!/usr/bin/env python3
"""Extract images from a Genus PCXLIB archive (the game's `.PCL` files).

OTMCGA.PCL / OTCGA.PCL are "pcxLib" archives (Genus Microprogramming, 1988-89)
holding the named `.pcc` images the game loads (FAMILY, SUPPLIES, TERRAIN,
ANIMALS, TRAVELOX, SCENERY, …). Each directory entry is:

    name : 12 bytes  ("NAME    .EXT", 8.3, space-padded)
    +12  : 1 byte    flag
    +13  : uint32    size of the PCX data
    ...  : metadata  (entry header is 83 bytes total)
    +83  : PCX image data (size bytes)

This extracts every entry and (with pcx.py) can convert them to PNG.

    python3 port/assets/pcxlib.py LIB.PCL OUTDIR     # dump all entries as PNG
    python3 port/assets/pcxlib.py LIB.PCL --list     # just list the directory
"""
from __future__ import annotations

import os
import re
import struct
import sys

from pcx import decode_pcx, to_rgb, write_png

_PCX_SIG = bytes.fromhex("0a050108")
_NAME = re.compile(rb"[A-Z0-9_ ]{8}\.[A-Z]{3}")


def entries(data: bytes):
    """Yield (name, data_offset, size) for each valid image in the archive."""
    for m in _NAME.finditer(data):
        off = m.start()
        data_off = off + 83
        if data[data_off:data_off + 4] != _PCX_SIG:      # real entries only
            continue
        size = struct.unpack_from("<I", data, off + 13)[0]
        name = m.group().decode("latin1").replace(" ", "").lower()
        yield name, data_off, size


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    data = open(argv[1], "rb").read()
    if data[:6] != b"pcxLib":
        print("warning: missing 'pcxLib' signature", file=sys.stderr)

    items = list(entries(data))
    if len(argv) >= 3 and argv[2] == "--list":
        for name, off, size in items:
            print(f"  {name:16} @0x{off:06x}  {size} bytes")
        print(f"{len(items)} images")
        return 0

    # The archived images share one global palette (PAL.256) rather than each
    # embedding its own; load it as a fallback from the game directory.
    global_pal = None
    pal_path = os.path.join(os.path.dirname(argv[1]), "PAL.256")
    if os.path.exists(pal_path):
        global_pal = decode_pcx(open(pal_path, "rb").read())[3]

    outdir = argv[2] if len(argv) >= 3 else "build/pcl"
    os.makedirs(outdir, exist_ok=True)
    for name, off, size in items:
        blob = data[off:off + size]
        w, h, idx, pal = decode_pcx(blob)
        if blob[-769] != 0x0C and global_pal:        # no embedded palette -> shared
            pal = global_pal
        stem = os.path.splitext(name)[0]
        write_png(os.path.join(outdir, stem + ".png"), w, h, to_rgb(w, h, idx, pal))
    print(f"extracted {len(items)} images from {argv[1]} -> {outdir}")
    return 0


if __name__ == "__main__":
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    raise SystemExit(main(sys.argv))

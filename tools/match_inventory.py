#!/usr/bin/env python3
"""Build the matching "answer key": per-function byte ranges in the unpacked image.

For byte-accurate (matching) decompilation we need, for every function, the exact
original bytes it must compile to. This carves the unpacked image into functions
using the Borland stack-frame prologue (the same detector as map_segments) and
records each function's offset, size, and hash. The result is the denominator for
a "% matched" metric and the reference the diff harness (match_compare.py) checks
compiled output against.

Output: config/match_targets.json

Usage: python3 tools/match_inventory.py [build/OREGON_unpacked.exe]
"""
from __future__ import annotations

import hashlib
import json
import re
import struct
import sys

# push bp; mov bp,sp; <ax setup 0..5 bytes>; call __stkcheck (0x20a4:0x244)
_PROLOGUE = re.compile(rb"\x55\x89\xe5.{0,5}\x9a\x44\x02\xa4\x20", re.S)


def load(path):
    d = open(path, "rb").read()
    hdr = struct.unpack("<13H", d[2:28])
    ncrlc, cparhdr = hdr[2], hdr[3]
    base = cparhdr * 16
    relocs = [struct.unpack("<HH", d[0x1C + i * 4:0x1C + i * 4 + 4]) for i in range(ncrlc)]
    return d[base:], relocs


def main(argv):
    path = argv[1] if len(argv) > 1 else "build/OREGON_unpacked.exe"
    image, relocs = load(path)

    # named functions: map "SSSS:OOOO" -> (name, lifted) from the symbol table
    names = {}
    try:
        sym = json.load(open("config/symbols.json", encoding="utf-8"))
        for k, v in sym["functions"].items():
            seg, off = (int(x, 16) for x in k.split(":"))
            names[seg * 16 + off] = (v["name"], bool(v.get("file")))
    except FileNotFoundError:
        pass

    # function entries = prologue matches, sorted; size = gap to the next entry
    entries = sorted(m.start() for m in _PROLOGUE.finditer(image))
    funcs = []
    total = 0
    lifted = 0
    for i, off in enumerate(entries):
        end = entries[i + 1] if i + 1 < len(entries) else min(off + 0x2000, len(image))
        size = end - off
        blob = image[off:end]
        name, is_lifted = names.get(off, (f"sub_{off:05x}", False))
        funcs.append(dict(
            name=name, offset=off, segment=off >> 4, size=size,
            sha8=hashlib.sha256(blob).hexdigest()[:8], lifted=is_lifted,
        ))
        total += size
        lifted += is_lifted

    out = dict(
        source=path,
        function_count=len(funcs),
        total_bytes=total,
        lifted_functions=lifted,
        note="framed (Borland stack-frame) functions only; size = bytes to the "
             "next function entry, so includes any trailing CONST data.",
        functions=funcs,
    )
    json.dump(out, open("config/match_targets.json", "w", encoding="utf-8"), indent=1)
    print(f"functions={len(funcs)} total_code_bytes={total} "
          f"({total/1024:.1f} KB) lifted={lifted}")
    print("wrote config/match_targets.json")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

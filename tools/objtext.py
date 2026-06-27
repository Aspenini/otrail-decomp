#!/usr/bin/env python3
"""objtext.py - extract the _TEXT (code) bytes from a Turbo C OMF .OBJ file.

Turbo C 2.0's `TCC -c` emits an Intel OMF object. For matching we want just the
code a function compiled to, so we can diff it against the original image. With
one function per object, the object's `_TEXT` segment *is* that function's code.

    python3 tools/objtext.py OUT/RAND.OBJ            # -> hex/length to stdout
    python3 tools/objtext.py OUT/RAND.OBJ rand.bin   # -> write the code bytes

Fixups (segment-reference words) are left as the compiler emitted them; the match
harness masks those positions anyway, so their value does not matter.
"""
from __future__ import annotations

import sys
from pathlib import Path

# OMF record types we care about.
LNAMES, LNAMES32 = 0x96, 0x96
SEGDEF, SEGDEF32 = 0x98, 0x99
LEDATA, LEDATA32 = 0xA0, 0xA1


def _index(buf, i):
    """Read an OMF index (1 or 2 bytes, 1-based). Returns (value, next_i)."""
    b = buf[i]
    if b & 0x80:
        return ((b & 0x7F) << 8) | buf[i + 1], i + 2
    return b, i + 1


def extract_text(data: bytes) -> bytes:
    """Concatenate the LEDATA of the segment named _TEXT, in offset order."""
    names = []                 # LNAMES, 1-based
    segnames = []              # SEGDEF order, 1-based -> name-index
    text_seg = None            # 1-based SEGDEF index of _TEXT
    chunks = {}                # enumerated offset -> bytes

    i = 0
    n = len(data)
    while i < n:
        rectype = data[i]
        rlen = data[i + 1] | (data[i + 2] << 8)
        body = data[i + 3:i + 3 + rlen - 1]   # minus the trailing checksum byte
        i += 3 + rlen

        if rectype == LNAMES:
            j = 0
            while j < len(body):
                ln = body[j]; j += 1
                names.append(body[j:j + ln].decode("latin1")); j += ln
        elif rectype in (SEGDEF, SEGDEF32):
            j = 0
            acbp = body[j]; j += 1
            if (acbp >> 5) & 7 == 0:            # absolute segment: frame+offset
                j += 3
            j += 4 if rectype == SEGDEF32 else 2   # segment length
            name_idx, j = _index(body, j)          # 1-based into LNAMES
            segnames.append(name_idx)
            if 1 <= name_idx <= len(names) and names[name_idx - 1] == "_TEXT":
                text_seg = len(segnames)           # this SEGDEF's 1-based index
        elif rectype in (LEDATA, LEDATA32):
            j = 0
            seg_idx, j = _index(body, j)
            if rectype == LEDATA32:
                off = int.from_bytes(body[j:j + 4], "little"); j += 4
            else:
                off = body[j] | (body[j + 1] << 8); j += 2
            if text_seg is not None and seg_idx == text_seg:
                chunks[off] = body[j:]

    out = bytearray()
    for off in sorted(chunks):
        if off > len(out):
            out.extend(b"\x00" * (off - len(out)))   # gap (uncommon)
        out[off:off + len(chunks[off])] = chunks[off]
    return bytes(out)


def main(argv) -> int:
    if not argv:
        print(__doc__)
        return 1
    data = Path(argv[0]).read_bytes()
    code = extract_text(data)
    if len(argv) > 1:
        Path(argv[1]).write_bytes(code)
        print(f"{argv[0]}: _TEXT = {len(code)} bytes -> {argv[1]}")
    else:
        print(f"{argv[0]}: _TEXT = {len(code)} bytes")
        print(code[:64].hex(" "))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))

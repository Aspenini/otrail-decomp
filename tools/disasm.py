#!/usr/bin/env python3
"""Disassemble one function from the unpacked image, by segment:offset.

    python tools/disasm.py 0032:0655          # one function (to next boundary)
    python tools/disasm.py 0032:0655 0x200    # force a byte length

Boundaries come from the known function offsets in config/symbols.json (plus
the requested address), so a function is shown up to wherever the next known
function in the same segment begins. Addresses are printed as segment offsets,
matching how the lifted C annotates them. Requires ndisasm on PATH.
"""
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
IMG = ROOT / "build" / "OREGON_unpacked.exe"


def load_base():
    # load image starts at e_cparhdr*16; read it straight from the MZ header
    hdr = IMG.read_bytes()[:0x40]
    e_cparhdr = int.from_bytes(hdr[0x08:0x0A], "little")
    return e_cparhdr * 16


def seg_offsets(seg):
    """All known function offsets within a segment, from symbols.json."""
    sym = json.loads((ROOT / "config" / "symbols.json").read_text())
    offs = set()
    for key in sym["functions"]:
        if ":" not in key:
            continue
        s, o = key.split(":")
        if int(s, 16) == seg:
            offs.add(int(o, 16))
    return offs


def main(argv):
    if not argv:
        print(__doc__)
        return 1
    seg_s, off_s = argv[0].split(":")
    seg, off = int(seg_s, 16), int(off_s, 16)
    base = load_base()
    file_off = base + seg * 16 + off

    if len(argv) > 1:
        length = int(argv[1], 0)
    else:
        later = sorted(o for o in seg_offsets(seg) if o > off)
        length = (later[0] - off) if later else 0x400

    print(f"; {seg:04x}:{off:04x}  file 0x{file_off:x}  ({length} bytes)\n")
    cmd = ["ndisasm", "-b16", "-e", str(file_off), "-o", hex(off), str(IMG)]
    out = subprocess.run(cmd, capture_output=True, text=True).stdout
    shown = 0
    for line in out.splitlines():
        # ndisasm columns: addr  hexbytes  mnemonic
        try:
            addr = int(line.split()[0], 16)
        except (ValueError, IndexError):
            continue
        if addr >= off + length:
            break
        print(line)
        shown += 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

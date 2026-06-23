#!/usr/bin/env python3
"""Build a segment / function map of the unpacked OREGON image.

Real decompilation of a 16-bit segmented DOS program needs a map of which
segments hold code, which hold data, and where the functions are. We recover
that structure from the relocation table without a full disassembler:

  * Every far call is encoded ``9A off16 seg16`` and its ``seg16`` word is
    relocated. So a relocation whose target word sits exactly 3 bytes after a
    ``0x9A`` byte marks a far call, and (off16 : seg16) is a function entry.
    The set of distinct targets is the cross-segment function table.

  * Relocated words that are *not* far-call segment operands are data/segment
    loads (``mov ax,seg`` / ``push seg`` / far ``jmp``). The segment values they
    carry, weighted by frequency, reveal DGROUP and other data groups.

Outputs:
    config/segments.json   machine-readable map
    docs/segment_map.md     human-readable summary

Usage:
    python3 tools/map_segments.py build/OREGON_unpacked.exe
"""
from __future__ import annotations

import bisect
import json
import re
import struct
import sys
from collections import defaultdict


def load(path):
    d = open(path, "rb").read()
    hdr = struct.unpack("<13H", d[2:28])
    ncrlc, cparhdr = hdr[2], hdr[3]
    e_ip, e_cs, e_ss, e_sp = hdr[9], hdr[10], hdr[6], hdr[7]
    base = cparhdr * 16
    relocs = [struct.unpack("<HH", d[0x1C + i * 4:0x1C + i * 4 + 4]) for i in range(ncrlc)]
    image = d[base:]
    return image, relocs, dict(ip=e_ip, cs=e_cs, ss=e_ss, sp=e_sp, base=base)


# Borland function prologue + the universal __stkcheck far call (0x20a4:0x244):
#   55 89 E5            push bp ; mov bp,sp
#   (B8 ll hh | 31 C0)  mov ax,frame  /  xor ax,xor   (0..5 bytes)
#   9A 44 02 A4 20      call 0x20a4:0x244
# Every framed application function opens with this, so it is a precise,
# false-positive-free function detector for the parts of the program that use
# stack frames. (Hand-written graphics/runtime routines omit it; those are
# recovered from far-call targets instead.)
_PROLOGUE = re.compile(rb"\x55\x89\xe5.{0,5}\x9a\x44\x02\xa4\x20", re.S)


def build_map(image, relocs):
    far_targets = set()                 # absolute image offsets of far-call targets
    callers = defaultdict(int)          # target_seg -> number of call sites
    data_seg_refs = defaultdict(int)    # seg value -> times loaded as data/seg
    seg_bases = set()                   # segment paragraph values that start code
    for off, seg in relocs:
        L = seg * 16 + off              # linear address of the relocated word
        if L + 1 >= len(image):
            continue
        target_seg = image[L] | (image[L + 1] << 8)
        if L >= 3 and image[L - 3] == 0x9A:        # far call
            target_off = image[L - 2] | (image[L - 1] << 8)
            far_targets.add(target_seg * 16 + target_off)
            callers[target_seg] += 1
            seg_bases.add(target_seg)
        else:
            data_seg_refs[target_seg] += 1

    # Prologue-detected functions (catches near-called internals the far-call
    # scan misses); union with far-call targets is the full function set.
    prologue = set(m.start() for m in _PROLOGUE.finditer(image))
    funcs = far_targets | prologue

    # Partition the image into segments using the code-segment bases as
    # boundaries, then assign each function entry to its containing segment.
    bases = sorted(seg_bases)
    bpar = [b * 16 for b in bases]
    by_seg = defaultdict(lambda: {"funcs": 0, "framed": 0, "min": 1 << 30, "max": 0})
    for e in funcs:
        i = bisect.bisect_right(bpar, e) - 1
        if i < 0:
            continue
        s = bases[i]
        rec = by_seg[s]
        rec["funcs"] += 1
        if e in prologue:
            rec["framed"] += 1
        off = e - s * 16
        rec["min"] = min(rec["min"], off)
        rec["max"] = max(rec["max"], off)

    code_segments = []
    for s in bases:
        rec = by_seg[s]
        if rec["funcs"] == 0:
            continue
        code_segments.append(dict(
            segment=s,
            func_count=rec["funcs"],
            framed=rec["framed"],          # functions with a stack frame
            call_sites=callers.get(s, 0),
            entry_min=rec["min"],
            entry_max=rec["max"],
        ))
    data_segments = [
        dict(segment=s, data_refs=c)
        for s, c in sorted(data_seg_refs.items(), key=lambda kv: -kv[1])
    ]
    return code_segments, data_segments, funcs


def main(argv):
    path = argv[1] if len(argv) > 1 else "build/OREGON_unpacked.exe"
    image, relocs, hdr = load(path)
    code_segments, data_segments, funcs = build_map(image, relocs)

    out = dict(
        source=path,
        image_bytes=len(image),
        image_paragraphs=(len(image) + 15) // 16,
        entry=dict(cs=hdr["cs"], ip=hdr["ip"]),
        stack=dict(ss=hdr["ss"], sp=hdr["sp"]),
        relocations=len(relocs),
        function_count=len(funcs),
        code_segments=code_segments,
        data_segments=data_segments[:20],
    )
    json.dump(out, open("config/segments.json", "w", encoding="utf-8"), indent=2)

    lines = []
    lines.append("# OREGON.EXE unpacked — segment & function map\n")
    lines.append(f"_Generated by `tools/map_segments.py` from `{path}`._\n")
    lines.append(f"- image: {len(image):#x} bytes "
                 f"({(len(image)+15)//16:#x} paragraphs)")
    lines.append(f"- entry: `CS:IP = {hdr['cs']:#06x}:{hdr['ip']:#06x}`  "
                 f"stack `SS:SP = {hdr['ss']:#06x}:{hdr['sp']:#06x}`")
    lines.append(f"- relocations: {len(relocs)}")
    lines.append(f"- functions (far-call targets + framed prologues): {len(funcs)}\n")

    lines.append("Functions = union of far-call targets and Borland stack-frame "
                 "prologues (`55 89 E5 ... call 0x20a4:0x244`). 'framed' counts "
                 "the prologue-detected ones; graphics/runtime segments use "
                 "hand-written routines without that prologue.\n")
    fbase = hdr["base"]
    lines.append("## Code segments\n")
    lines.append("File off = byte offset of segment:0 in build/OREGON_unpacked.exe.\n")
    lines.append("| segment | file off | functions | framed | call sites | entry range |")
    lines.append("|---------|----------|-----------|--------|------------|-------------|")
    for c in sorted(code_segments, key=lambda c: -c["func_count"]):
        lines.append(
            f"| `{c['segment']:#06x}` | `{fbase + c['segment']*16:#08x}` | "
            f"{c['func_count']} | {c['framed']} | {c['call_sites']} | "
            f"`{c['entry_min']:#06x}`..`{c['entry_max']:#06x}` |")
    lines.append("")

    lines.append("\n## Top data/segment references (candidate data groups)\n")
    lines.append("| segment | file off | data refs |")
    lines.append("|---------|----------|-----------|")
    for dseg in data_segments[:12]:
        s = dseg["segment"]
        lines.append(f"| `{s:#06x}` | `{fbase + s*16:#08x}` | {dseg['data_refs']} |")
    lines.append("")
    open("docs/segment_map.md", "w", encoding="utf-8").write("\n".join(lines))

    print(f"functions={len(funcs)} across {len(code_segments)} code segments; "
          f"{len(data_segments)} distinct data-seg values")
    print("wrote config/segments.json and docs/segment_map.md")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

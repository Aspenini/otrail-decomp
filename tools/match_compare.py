#!/usr/bin/env python3
"""Compare compiled function bytes against the original, and report % matched.

This is the matching harness. For each function we have produced compiled bytes
for (dropped as build/match/<name>.bin), it diffs them against the original bytes
from the unpacked image -- masking the relocated segment words, which depend on
link layout and aren't part of "did the code compile the same".

  python3 tools/match_compare.py --self-test   # validate the harness (-> 100%)
  python3 tools/match_compare.py               # report progress vs build/match/*

A function "matches" when every non-relocated byte is identical. The metric is
matched bytes / total code bytes (from config/match_targets.json).
"""
from __future__ import annotations

import json
import os
import struct
import sys


def load(path):
    d = open(path, "rb").read()
    hdr = struct.unpack("<13H", d[2:28])
    ncrlc, base = hdr[2], hdr[3] * 16
    relocs = [struct.unpack("<HH", d[0x1C + i * 4:0x1C + i * 4 + 4]) for i in range(ncrlc)]
    image = d[base:]
    # byte offsets covered by a relocation (segment word = 2 bytes)
    masked = set()
    for off, seg in relocs:
        L = seg * 16 + off
        masked.add(L)
        masked.add(L + 1)
    return image, masked


def compare(image, masked, off, size, compiled):
    """Return (matched_bytes, comparable_bytes) for a function range."""
    matched = comparable = 0
    n = min(size, len(compiled))
    for i in range(size):
        if (off + i) in masked:
            continue                       # relocated word: not part of the match
        comparable += 1
        if i < n and compiled[i] == image[off + i]:
            matched += 1
    return matched, comparable


def main(argv):
    self_test = "--self-test" in argv
    image, masked = load("build/OREGON_unpacked.exe")
    targets = json.load(open("config/match_targets.json", encoding="utf-8"))

    total = matched_total = exact = present = 0
    rows = []
    for f in targets["functions"]:
        off, size = f["offset"], f["size"]
        total += size
        if self_test:
            blob = image[off:off + size]            # feed each function its own bytes
        else:
            p = os.path.join("build", "match", f["name"] + ".bin")
            if not os.path.exists(p):
                continue
            blob = open(p, "rb").read()
        present += 1
        m, c = compare(image, masked, off, size, blob)
        matched_total += m
        if m == c:
            exact += 1
        rows.append((f["name"], m, c))

    pct = 100.0 * matched_total / total if total else 0.0
    if self_test:
        bad = [r for r in rows if r[1] != r[2]]
        print(f"self-test: {len(rows)} functions, {exact} exact, "
              f"{matched_total}/{total} bytes ({pct:.2f}%)")
        print("HARNESS OK" if not bad else f"HARNESS BUG: {bad[:3]}")
        return 0 if not bad else 1

    print(f"matching progress: {exact}/{targets['function_count']} functions exact, "
          f"{present} attempted, {matched_total}/{total} bytes ({pct:.2f}%)")
    if present == 0:
        print("(no compiled functions in build/match/ yet - see docs/matching.md)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

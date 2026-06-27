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


def compute(match_dir=os.path.join("build", "match"),
            image_path="build/OREGON_unpacked.exe", self_test=False):
    """Compute matching stats. Returns a dict the dashboard and CLI both use.

    Safe to call when nothing has been matched yet (or even before the image is
    unpacked): on a missing image it falls back to the target metadata so the
    denominator is still reported and the dashboard renders 0 / N.
    """
    targets = json.load(open("config/match_targets.json", encoding="utf-8"))
    stats = {
        "function_count": targets["function_count"],
        "total_bytes": targets.get("total_bytes")
                       or sum(f["size"] for f in targets["functions"]),
        "exact": 0, "present": 0, "matched_bytes": 0, "rows": [],
    }
    if not os.path.exists(image_path):
        return stats

    image, masked = load(image_path)
    total = stats["total_bytes"]
    matched_total = exact = present = 0
    rows = []
    for f in targets["functions"]:
        off, size = f["offset"], f["size"]
        if self_test:
            blob = image[off:off + size]            # feed each function its own bytes
        else:
            p = os.path.join(match_dir, f["name"] + ".bin")
            if not os.path.exists(p):
                continue
            blob = open(p, "rb").read()
        present += 1
        m, c = compare(image, masked, off, size, blob)
        matched_total += m
        if m == c:
            exact += 1
        rows.append((f["name"], m, c))

    stats.update(exact=exact, present=present, matched_bytes=matched_total, rows=rows)
    return stats


def main(argv):
    self_test = "--self-test" in argv
    stats = compute(self_test=self_test)
    total = stats["total_bytes"]
    matched_total = stats["matched_bytes"]
    exact = stats["exact"]
    present = stats["present"]
    rows = stats["rows"]
    targets = {"function_count": stats["function_count"]}

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

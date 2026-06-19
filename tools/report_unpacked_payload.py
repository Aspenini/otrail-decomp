#!/usr/bin/env python3
"""Generate a compact analysis report for the stable unpacked payload image."""

from __future__ import annotations

import argparse
import hashlib
import math
import shutil
import subprocess
from collections import Counter
from pathlib import Path


def shannon_entropy(data: bytes) -> float:
    if not data:
        return 0.0
    counts = Counter(data)
    total = len(data)
    entropy = 0.0
    for count in counts.values():
        p = count / total
        entropy -= p * math.log2(p)
    return entropy


def extract_ascii_strings(data: bytes, min_len: int) -> list[tuple[int, str]]:
    results: list[tuple[int, str]] = []
    start = None
    buf: list[int] = []

    for idx, byte in enumerate(data):
        if 32 <= byte < 127:
            if start is None:
                start = idx
            buf.append(byte)
            continue
        if start is not None and len(buf) >= min_len:
            results.append((start, bytes(buf).decode("ascii")))
        start = None
        buf = []

    if start is not None and len(buf) >= min_len:
        results.append((start, bytes(buf).decode("ascii")))
    return results


def window_stats(data: bytes, window: int) -> list[dict[str, float | int]]:
    rows: list[dict[str, float | int]] = []
    for start in range(0, len(data), window):
        chunk = data[start:start + window]
        zeros = chunk.count(0)
        printable = sum(32 <= b < 127 for b in chunk)
        rows.append(
            {
                "start": start,
                "size": len(chunk),
                "zeros": zeros,
                "printable": printable,
                "entropy": shannon_entropy(chunk),
            }
        )
    return rows


def scan_pattern(data: bytes, pattern: bytes) -> list[int]:
    offsets: list[int] = []
    start = 0
    while True:
        found = data.find(pattern, start)
        if found < 0:
            return offsets
        offsets.append(found)
        start = found + 1


def render_offset_list(offsets: list[int], limit: int = 12) -> str:
    if not offsets:
        return "none"
    shown = ", ".join(f"0x{off:04X}" for off in offsets[:limit])
    if len(offsets) > limit:
        shown += f", ... ({len(offsets)} total)"
    return shown


def maybe_disasm_sample(payload: Path, offset: int, lines: int) -> list[str]:
    ndisasm = shutil.which("ndisasm")
    if ndisasm is None:
        return []
    completed = subprocess.run(
        [ndisasm, "-b", "16", "-e", str(offset), "-o", hex(offset), str(payload)],
        check=True,
        capture_output=True,
        text=True,
    )
    return completed.stdout.splitlines()[:lines]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--payload", default="build/entry_bootstrap_replay_readable_heuristic.bin")
    parser.add_argument("--report-out", default="build/unpacked_payload_report.md")
    parser.add_argument("--min-string", type=int, default=4)
    parser.add_argument("--window", type=int, default=256)
    parser.add_argument("--sample-lines", type=int, default=20)
    args = parser.parse_args()

    payload = Path(args.payload)
    report_out = Path(args.report_out)
    data = payload.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    strings = extract_ascii_strings(data, args.min_string)
    windows = window_stats(data, args.window)

    low_zero = sorted(windows, key=lambda row: (row["zeros"], -row["entropy"]))[:8]
    high_printable = sorted(windows, key=lambda row: (-row["printable"], row["zeros"]))[:8]
    high_entropy = sorted(windows, key=lambda row: (-row["entropy"], row["zeros"]))[:8]

    patterns = [
        ("near call opcode", b"\xE8"),
        ("far call opcode", b"\x9A"),
        ("iret opcode", b"\xCF"),
        ("int 21h", b"\xCD\x21"),
        ("int 10h", b"\xCD\x10"),
        ("CLI/STI pair", b"\xFA\xFB"),
    ]
    pattern_hits = [(label, scan_pattern(data, pattern)) for label, pattern in patterns]

    sample_offsets = [int(row["start"]) for row in low_zero[:3]]

    lines: list[str] = []
    lines.append("# Unpacked Payload Report")
    lines.append("")
    lines.append("## Identity")
    lines.append("")
    lines.append(f"- Payload: `{payload}`")
    lines.append(f"- Size: `{len(data)}` bytes")
    lines.append(f"- SHA-256: `{digest}`")
    lines.append("")
    lines.append("## Strings")
    lines.append("")
    for offset, text in strings[:24]:
        lines.append(f"- `0x{offset:04X}` `{text}`")
    if len(strings) > 24:
        lines.append(f"- ... `{len(strings) - 24}` more strings omitted")
    lines.append("")
    lines.append("## Window Stats")
    lines.append("")
    lines.append("### Low-zero windows")
    lines.append("")
    for row in low_zero:
        lines.append(
            f"- `0x{int(row['start']):04X}` size=`{int(row['size'])}` "
            f"zeros=`{int(row['zeros'])}` printable=`{int(row['printable'])}` "
            f"entropy=`{float(row['entropy']):.2f}`"
        )
    lines.append("")
    lines.append("### High-printable windows")
    lines.append("")
    for row in high_printable:
        lines.append(
            f"- `0x{int(row['start']):04X}` size=`{int(row['size'])}` "
            f"zeros=`{int(row['zeros'])}` printable=`{int(row['printable'])}` "
            f"entropy=`{float(row['entropy']):.2f}`"
        )
    lines.append("")
    lines.append("### High-entropy windows")
    lines.append("")
    for row in high_entropy:
        lines.append(
            f"- `0x{int(row['start']):04X}` size=`{int(row['size'])}` "
            f"zeros=`{int(row['zeros'])}` printable=`{int(row['printable'])}` "
            f"entropy=`{float(row['entropy']):.2f}`"
        )
    lines.append("")
    lines.append("## Opcode / Signature Scan")
    lines.append("")
    for label, offsets in pattern_hits:
        lines.append(f"- {label}: {render_offset_list(offsets)}")
    lines.append("")
    if shutil.which("ndisasm") is not None:
        lines.append("## Disassembly Samples")
        lines.append("")
        for offset in sample_offsets:
            lines.append(f"### Offset `0x{offset:04X}`")
            lines.append("")
            lines.append("```asm")
            lines.extend(maybe_disasm_sample(payload, offset, args.sample_lines))
            lines.append("```")
            lines.append("")

    report_out.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote unpacked payload report: {report_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

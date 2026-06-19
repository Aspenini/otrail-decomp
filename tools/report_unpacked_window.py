#!/usr/bin/env python3
"""Report bytes, disassembly, and trace provenance for an unpacked payload window."""

from __future__ import annotations

import argparse
import shutil
import subprocess
from collections import Counter
from pathlib import Path

from unpacked_window_analysis import compute_window_summary, count_overlap_bytes, load_trace_rows, parse_int

def render_hexdump(data: bytes, base: int) -> list[str]:
    lines: list[str] = []
    for offset in range(0, len(data), 16):
        chunk = data[offset:offset + 16]
        hex_part = " ".join(f"{byte:02X}" for byte in chunk)
        ascii_part = "".join(chr(byte) if 32 <= byte < 127 else "." for byte in chunk)
        lines.append(f"{base + offset:04X}: {hex_part:<47} |{ascii_part}|")
    return lines


def maybe_disasm(payload: Path, start: int, lines: int) -> list[str]:
    ndisasm = shutil.which("ndisasm")
    if ndisasm is None:
        return []
    completed = subprocess.run(
        [ndisasm, "-b", "16", "-e", str(start), "-o", hex(start), str(payload)],
        check=True,
        capture_output=True,
        text=True,
    )
    return completed.stdout.splitlines()[:lines]

def describe_event(row: dict[str, str], overlap_start: int, overlap_end: int, payload: bytes) -> str:
    event_type = row["type"]
    out_before = parse_int(row["out_before"])
    out_after = parse_int(row["out_after"])
    overlap = payload[overlap_start:overlap_end]
    overlap_hex = " ".join(f"{byte:02X}" for byte in overlap)
    parts = [
        f"`event={row['event_idx']}`",
        f"`type={event_type}`",
        f"`out=0x{out_before:04X}..0x{out_after:04X}`",
        f"`window=0x{overlap_start:04X}..0x{overlap_end:04X}`",
        f"`bytes={overlap_hex}`",
    ]
    if event_type == "literal":
        parts.append(f"`literal=0x{parse_int(row['literal']) & 0xFF:02X}`")
    elif event_type.startswith("short") or event_type.startswith("long"):
        copy_len = parse_int(row["copy_len"])
        back_disp = parse_int(row["back_disp"])
        copy_src = out_before + back_disp
        copy_src_16 = copy_src & 0xFFFF
        parts.append(f"`copy_len={copy_len}`")
        parts.append(f"`back_disp={back_disp}`")
        if copy_src < 0 or copy_src + copy_len > 0x10000:
            parts.append(f"`copy_src_linear={copy_src}..{copy_src + copy_len}`")
            parts.append(
                f"`copy_src_16=0x{copy_src_16:04X}..0x{(copy_src_16 + copy_len) & 0xFFFF:04X}`"
            )
        else:
            parts.append(f"`copy_src=0x{copy_src:04X}..0x{copy_src + copy_len:04X}`")
        token = parse_int(row["token_u16"])
        if token:
            parts.append(f"`token=0x{token:04X}`")
        ext = parse_int(row["ext"])
        if ext >= 0:
            parts.append(f"`ext={ext}`")
    return "- " + " ".join(parts)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--payload", default="build/entry_bootstrap_replay_readable_heuristic.bin")
    parser.add_argument("--trace", default="build/entry_handoff_trace.csv")
    parser.add_argument("--start", type=parse_int, required=True)
    parser.add_argument("--size", type=parse_int, default=0x40)
    parser.add_argument("--report-out", default="build/unpacked_window_report.md")
    parser.add_argument("--label", default="")
    parser.add_argument("--sample-lines", type=int, default=32)
    args = parser.parse_args()

    payload_path = Path(args.payload)
    trace_path = Path(args.trace)
    report_out = Path(args.report_out)

    payload = payload_path.read_bytes()
    start = args.start
    end = min(len(payload), start + args.size)
    window = payload[start:end]
    trace_rows = load_trace_rows(trace_path)
    summary = compute_window_summary(payload, trace_rows, start, args.size)

    overlaps: list[dict[str, str]] = []
    contributors: dict[str, dict[str, object]] = {}
    for row in trace_rows:
        out_before = parse_int(row["out_before"])
        out_after = parse_int(row["out_after"])
        if out_after <= start or out_before >= end:
            continue
        overlaps.append(row)
        if row["type"] != "literal":
            overlap = count_overlap_bytes(start, end, out_before, out_after)
            back_disp = parse_int(row["back_disp"])
            copy_len = parse_int(row["copy_len"])
            copy_src = out_before + back_disp
            copy_src_16 = copy_src & 0xFFFF
            if copy_src < 0 or copy_src + copy_len > 0x10000:
                label = f"wrap:0x{copy_src_16:04X}..0x{(copy_src_16 + copy_len) & 0xFFFF:04X}"
            else:
                label = f"0x{copy_src:04X}..0x{copy_src + copy_len:04X}"
            rec = contributors.setdefault(
                label,
                {"bytes": 0, "events": 0, "types": Counter(), "tokens": set()},
            )
            rec["bytes"] = int(rec["bytes"]) + overlap
            rec["events"] = int(rec["events"]) + 1
            cast_types = rec["types"]
            assert isinstance(cast_types, Counter)
            cast_types[row["type"]] += 1
            token = parse_int(row["token_u16"])
            if token:
                cast_tokens = rec["tokens"]
                assert isinstance(cast_tokens, set)
                cast_tokens.add(f"0x{token:04X}")

    event_counter = Counter(row["type"] for row in overlaps)

    lines: list[str] = []
    title = args.label if args.label else f"0x{start:04X}-0x{end:04X}"
    lines.append(f"# Unpacked Window Report: {title}")
    lines.append("")
    lines.append("## Window")
    lines.append("")
    lines.append(f"- Payload: `{payload_path}`")
    lines.append(f"- Trace: `{trace_path}`")
    lines.append(f"- Range: `0x{start:04X}..0x{end:04X}` (`{len(window)}` bytes)")
    lines.append(f"- Non-zero bytes: `{summary.nonzero}`")
    lines.append(f"- Zero bytes: `{summary.zero}`")
    lines.append(f"- Printable bytes: `{summary.printable}`")
    lines.append(f"- Control-like bytes: `{summary.ctrl}`")
    lines.append(f"- Overlapping events: `{summary.events}`")
    lines.append(f"- Literal bytes contributed: `{summary.literal_bytes}`")
    lines.append(f"- Short-copy bytes contributed: `{summary.short_bytes}`")
    lines.append(f"- Long-copy bytes contributed: `{summary.long_bytes}`")
    lines.append(f"- Wrapped-copy events: `{summary.wrapped_copy_events}`")
    lines.append(f"- Heuristic score: `{summary.score}`")
    if event_counter:
        summary = ", ".join(f"{kind}={count}" for kind, count in sorted(event_counter.items()))
        lines.append(f"- Event mix: `{summary}`")
    lines.append("")
    lines.append("## Hexdump")
    lines.append("")
    lines.append("```text")
    lines.extend(render_hexdump(window, start))
    lines.append("```")
    lines.append("")

    disasm = maybe_disasm(payload_path, start, args.sample_lines)
    if disasm:
        lines.append("## Disassembly")
        lines.append("")
        lines.append("```asm")
        lines.extend(disasm)
        lines.append("```")
        lines.append("")

    lines.append("## Copy Sources")
    lines.append("")
    if not contributors:
        lines.append("- No copy/backref contributors overlap this window.")
    else:
        ranked = sorted(
            contributors.items(),
            key=lambda item: (int(item[1]["bytes"]), int(item[1]["events"])),
            reverse=True,
        )
        for label, rec in ranked:
            types = rec["types"]
            tokens = rec["tokens"]
            assert isinstance(types, Counter)
            assert isinstance(tokens, set)
            type_summary = ",".join(f"{kind}:{count}" for kind, count in sorted(types.items()))
            token_summary = ", ".join(sorted(tokens)[:4]) if tokens else "none"
            if len(tokens) > 4:
                token_summary += f", ... ({len(tokens)} total)"
            lines.append(
                f"- `{label}` `bytes={rec['bytes']}` `events={rec['events']}` "
                f"`types={type_summary}` `tokens={token_summary}`"
            )
    lines.append("")

    lines.append("## Trace Overlap")
    lines.append("")
    if not overlaps:
        lines.append("- No trace events overlap this window.")
    else:
        for row in overlaps:
            out_before = parse_int(row["out_before"])
            out_after = parse_int(row["out_after"])
            overlap_start = max(start, out_before)
            overlap_end = min(end, out_after)
            lines.append(describe_event(row, overlap_start, overlap_end, payload))

    report_out.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote unpacked window report: {report_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

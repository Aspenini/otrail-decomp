#!/usr/bin/env python3
"""Report exact motif families behind unpacked payload windows."""

from __future__ import annotations

import argparse
import shutil
import subprocess
from pathlib import Path

from unpacked_window_analysis import (
    compute_window_summary,
    count_overlap_bytes,
    find_copy_consumers,
    find_output_writers,
    find_pattern_occurrences,
    load_trace_rows,
    parse_int,
    parse_trace_events,
)


def align_down(value: int, alignment: int) -> int:
    return value & ~(alignment - 1)


def align_up(value: int, alignment: int) -> int:
    return (value + alignment - 1) & ~(alignment - 1)


def parse_motif_specs(text: str) -> list[tuple[int, int]]:
    motifs: list[tuple[int, int]] = []
    for spec in text.split(","):
        spec = spec.strip()
        if not spec:
            continue
        start_text, size_text = spec.split(":", 1)
        motifs.append((parse_int(start_text), parse_int(size_text)))
    if not motifs:
        raise ValueError("at least one motif spec is required")
    return motifs


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


def format_writer(event) -> str:
    if event.event_type == "literal":
        return (
            f"`event={event.event_idx}` `type=literal` "
            f"`out=0x{event.out_before:04X}..0x{event.out_after:04X}` "
            f"`literal=0x{event.literal & 0xFF:02X}`"
        )
    source_start = event.source_start
    source_end = event.source_end
    return (
        f"`event={event.event_idx}` `type={event.event_type}` "
        f"`out=0x{event.out_before:04X}..0x{event.out_after:04X}` "
        f"`src=0x{source_start:04X}..0x{source_end:04X}` "
        f"`copy_len={event.copy_len}` "
        f"`token=0x{event.token_u16 & 0xFFFF:04X}`"
    )


def format_consumer(event, start: int, end: int) -> str:
    overlap = count_overlap_bytes(start, end, event.source_start, event.source_end)
    return (
        f"`event={event.event_idx}` `type={event.event_type}` "
        f"`target=0x{event.out_before:04X}..0x{event.out_after:04X}` "
        f"`src=0x{event.source_start:04X}..0x{event.source_end:04X}` "
        f"`overlap={overlap}` "
        f"`token=0x{event.token_u16 & 0xFFFF:04X}`"
    )


def render_occurrence_context(
    lines: list[str],
    payload_path: Path,
    payload: bytes,
    trace_rows: list[dict[str, str]],
    start: int,
    size: int,
    heading: str,
) -> None:
    block_start = align_down(start, 0x10)
    block_end = min(len(payload), align_up(start + size, 0x10))
    block = payload[block_start:block_end]
    summary = compute_window_summary(payload, trace_rows, block_start, block_end - block_start)

    lines.append(f"### {heading}")
    lines.append("")
    lines.append(f"- Range: `0x{start:04X}..0x{start + size:04X}`")
    lines.append(f"- Aligned block: `0x{block_start:04X}..0x{block_end:04X}`")
    lines.append(f"- Summary score: `{summary.score}`")
    lines.append(f"- Non-zero bytes: `{summary.nonzero}`")
    lines.append(f"- Literal/short/long bytes: `{summary.literal_bytes}` / `{summary.short_bytes}` / `{summary.long_bytes}`")
    lines.append("")
    lines.append("```text")
    lines.extend(render_hexdump(block, block_start))
    lines.append("```")
    lines.append("")

    disasm = maybe_disasm(payload_path, block_start, 12)
    if disasm:
        lines.append("```asm")
        lines.extend(disasm)
        lines.append("```")
        lines.append("")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--payload", default="build/entry_bootstrap_replay_readable_heuristic.bin")
    parser.add_argument("--trace", default="build/entry_handoff_trace.csv")
    parser.add_argument("--motifs", default="0x070B:0x4,0x0DC3:0x3")
    parser.add_argument("--report-out", default="build/unpacked_motif_family_report.md")
    args = parser.parse_args()

    payload_path = Path(args.payload)
    trace_path = Path(args.trace)
    report_out = Path(args.report_out)

    payload = payload_path.read_bytes()
    trace_rows = load_trace_rows(trace_path)
    events = parse_trace_events(trace_rows)
    motifs = parse_motif_specs(args.motifs)

    lines: list[str] = []
    lines.append("# Unpacked Motif Family Report")
    lines.append("")
    lines.append(f"- Payload: `{payload_path}`")
    lines.append(f"- Trace: `{trace_path}`")
    lines.append(f"- Motifs: `{args.motifs}`")
    lines.append("")

    for requested_start, size in motifs:
        requested_end = requested_start + size
        pattern = payload[requested_start:requested_end]
        occurrences = find_pattern_occurrences(payload, pattern)
        requested_writers = find_output_writers(events, requested_start, requested_end)
        requested_consumers = find_copy_consumers(events, requested_start, requested_end)

        seed_start = occurrences[0]
        seed_end = seed_start + size
        seed_writers = find_output_writers(events, seed_start, seed_end)
        seed_consumers = find_copy_consumers(events, seed_start, seed_end)

        lines.append(f"## Motif `0x{requested_start:04X}..0x{requested_end:04X}`")
        lines.append("")
        lines.append(f"- Bytes: `{pattern.hex().upper()}`")
        lines.append(f"- Size: `{size}`")
        lines.append(f"- Exact occurrence count: `{len(occurrences)}`")
        lines.append(f"- Exact occurrences: `{', '.join(f'0x{offset:04X}' for offset in occurrences)}`")
        lines.append(f"- Earliest exact seed: `0x{seed_start:04X}`")
        lines.append(f"- Requested window is seed: `{int(seed_start == requested_start)}`")
        lines.append("")

        lines.append("### Requested Writers")
        lines.append("")
        if requested_writers:
            for event in requested_writers:
                lines.append(f"- {format_writer(event)}")
        else:
            lines.append("- No writers overlap the requested motif window.")
        lines.append("")

        lines.append("### Requested Direct Consumers")
        lines.append("")
        if requested_consumers:
            for event in requested_consumers:
                lines.append(f"- {format_consumer(event, requested_start, requested_end)}")
        else:
            lines.append("- No direct copy consumers overlap the requested motif window.")
        lines.append("")

        if seed_start != requested_start:
            lines.append("### Earliest Seed Writers")
            lines.append("")
            for event in seed_writers:
                lines.append(f"- {format_writer(event)}")
            lines.append("")

            lines.append("### Earliest Seed Direct Consumers")
            lines.append("")
            if seed_consumers:
                for event in seed_consumers:
                    lines.append(f"- {format_consumer(event, seed_start, seed_end)}")
            else:
                lines.append("- No direct copy consumers overlap the earliest seed window.")
            lines.append("")

        lines.append("### Exact Occurrence Table")
        lines.append("")
        for occurrence_start in occurrences:
            occurrence_end = occurrence_start + size
            occurrence_writers = find_output_writers(events, occurrence_start, occurrence_end)
            writer_summary = "; ".join(format_writer(event) for event in occurrence_writers)
            if not writer_summary:
                writer_summary = "none"
            lines.append(
                f"- `0x{occurrence_start:04X}..0x{occurrence_end:04X}` "
                f"`writers={writer_summary}`"
            )
        lines.append("")

        render_occurrence_context(
            lines,
            payload_path,
            payload,
            trace_rows,
            requested_start,
            size,
            "Requested Context",
        )
        if seed_start != requested_start:
            render_occurrence_context(
                lines,
                payload_path,
                payload,
                trace_rows,
                seed_start,
                size,
                "Earliest Seed Context",
            )

        if seed_start != requested_start:
            lines.append("### Current Interpretation")
            lines.append("")
            lines.append(
                "- The requested motif is a reused downstream copy, not the earliest seed."
            )
            lines.append(
                "- The next lift should target the earliest seed plus its direct fan-out, not just the requested copy."
            )
            lines.append("")
        else:
            lines.append("### Current Interpretation")
            lines.append("")
            lines.append(
                "- The requested motif is also the earliest exact seed, so it is a valid family origin for further lift work."
            )
            lines.append("")

    report_out.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote unpacked motif family report: {report_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

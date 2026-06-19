#!/usr/bin/env python3
"""Trace contributor windows behind an unpacked payload target window."""

from __future__ import annotations

import argparse
import shutil
import subprocess
from collections import Counter
from dataclasses import dataclass
from pathlib import Path

from unpacked_window_analysis import compute_window_summary, count_overlap_bytes, load_trace_rows, parse_int


def align_down(value: int, alignment: int) -> int:
    return value & ~(alignment - 1)


def align_up(value: int, alignment: int) -> int:
    return (value + alignment - 1) & ~(alignment - 1)


@dataclass
class CopyContributor:
    label: str
    source_start: int
    source_end: int
    bytes_contributed: int
    events: int
    token_values: tuple[str, ...]
    short_events: int
    long_events: int
    wrapped: bool


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


def collect_copy_contributors(
    trace_rows: list[dict[str, str]],
    start: int,
    end: int,
) -> list[CopyContributor]:
    grouped: dict[tuple[int, int, bool], dict[str, object]] = {}

    for row in trace_rows:
        if row["type"] == "literal":
            continue
        out_before = parse_int(row["out_before"])
        out_after = parse_int(row["out_after"])
        overlap = count_overlap_bytes(start, end, out_before, out_after)
        if overlap == 0:
            continue

        copy_len = parse_int(row["copy_len"])
        back_disp = parse_int(row["back_disp"])
        copy_start = out_before + back_disp
        copy_end = copy_start + copy_len
        wrapped = copy_start < 0 or copy_end > 0x10000
        key = (copy_start & 0xFFFF, copy_len, wrapped)
        rec = grouped.setdefault(
            key,
            {
                "bytes": 0,
                "events": 0,
                "tokens": set(),
                "short_events": 0,
                "long_events": 0,
            },
        )
        rec["bytes"] = int(rec["bytes"]) + overlap
        rec["events"] = int(rec["events"]) + 1
        token = parse_int(row["token_u16"])
        if token:
            cast_tokens = rec["tokens"]
            assert isinstance(cast_tokens, set)
            cast_tokens.add(f"0x{token:04X}")
        if row["type"].startswith("short"):
            rec["short_events"] = int(rec["short_events"]) + 1
        elif row["type"].startswith("long"):
            rec["long_events"] = int(rec["long_events"]) + 1

    out: list[CopyContributor] = []
    for (copy_start_16, copy_len, wrapped), rec in grouped.items():
        copy_end_16 = (copy_start_16 + copy_len) & 0xFFFF
        if wrapped:
            label = f"wrap:0x{copy_start_16:04X}..0x{copy_end_16:04X}"
        else:
            label = f"0x{copy_start_16:04X}..0x{copy_start_16 + copy_len:04X}"
        tokens = rec["tokens"]
        assert isinstance(tokens, set)
        out.append(
            CopyContributor(
                label=label,
                source_start=copy_start_16,
                source_end=copy_start_16 + copy_len,
                bytes_contributed=int(rec["bytes"]),
                events=int(rec["events"]),
                token_values=tuple(sorted(tokens)),
                short_events=int(rec["short_events"]),
                long_events=int(rec["long_events"]),
                wrapped=wrapped,
            )
        )

    out.sort(key=lambda row: (row.bytes_contributed, row.events, row.long_events), reverse=True)
    return out


def render_window_summary_lines(name: str, summary) -> list[str]:
    return [
        f"- {name} range: `{summary.label}`",
        f"- {name} non-zero bytes: `{summary.nonzero}`",
        f"- {name} zero bytes: `{summary.zero}`",
        f"- {name} printable bytes: `{summary.printable}`",
        f"- {name} control-like bytes: `{summary.ctrl}`",
        f"- {name} events: `{summary.events}`",
        f"- {name} literal/short/long bytes: `{summary.literal_bytes}` / `{summary.short_bytes}` / `{summary.long_bytes}`",
        f"- {name} wrapped-copy events: `{summary.wrapped_copy_events}`",
        f"- {name} heuristic score: `{summary.score}`",
    ]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--payload", default="build/entry_bootstrap_replay_readable_heuristic.bin")
    parser.add_argument("--trace", default="build/entry_handoff_trace.csv")
    parser.add_argument("--start", type=parse_int, default=0x2000)
    parser.add_argument("--size", type=parse_int, default=0x40)
    parser.add_argument("--align", type=parse_int, default=0x10)
    parser.add_argument("--max-children", type=int, default=8)
    parser.add_argument("--max-grandchildren", type=int, default=5)
    parser.add_argument("--sample-lines", type=int, default=24)
    parser.add_argument("--report-out", default="build/unpacked_contributor_chain_report.md")
    args = parser.parse_args()

    payload_path = Path(args.payload)
    trace_path = Path(args.trace)
    report_out = Path(args.report_out)
    payload = payload_path.read_bytes()
    trace_rows = load_trace_rows(trace_path)

    root_start = args.start
    root_end = min(len(payload), root_start + args.size)
    root_summary = compute_window_summary(payload, trace_rows, root_start, args.size)
    root_contributors = collect_copy_contributors(trace_rows, root_start, root_end)[: args.max_children]

    lines: list[str] = []
    lines.append(f"# Unpacked Contributor Chain: 0x{root_start:04X}-0x{root_end:04X}")
    lines.append("")
    lines.append("## Root")
    lines.append("")
    lines.extend(render_window_summary_lines("Root", root_summary))
    lines.append("")

    root_disasm = maybe_disasm(payload_path, root_start, args.sample_lines)
    if root_disasm:
        lines.append("## Root Disassembly")
        lines.append("")
        lines.append("```asm")
        lines.extend(root_disasm)
        lines.append("```")
        lines.append("")

    lines.append("## Contributor Ranking")
    lines.append("")
    if not root_contributors:
        lines.append("- No copy contributors overlap the root window.")
    else:
        for contributor in root_contributors:
            token_text = ", ".join(contributor.token_values[:4]) if contributor.token_values else "none"
            if len(contributor.token_values) > 4:
                token_text += f", ... ({len(contributor.token_values)} total)"
            lines.append(
                f"- `{contributor.label}` "
                f"`bytes={contributor.bytes_contributed}` "
                f"`events={contributor.events}` "
                f"`short={contributor.short_events}` "
                f"`long={contributor.long_events}` "
                f"`wrapped={int(contributor.wrapped)}` "
                f"`tokens={token_text}`"
            )
    lines.append("")

    lines.append("## Contributor Detail")
    lines.append("")
    for contributor in root_contributors:
        child_start = align_down(contributor.source_start, args.align)
        child_end = min(len(payload), align_up(contributor.source_end, args.align))
        child_summary = compute_window_summary(payload, trace_rows, child_start, child_end - child_start)
        child_disasm = maybe_disasm(payload_path, child_start, min(args.sample_lines, 16))
        grandchildren = collect_copy_contributors(trace_rows, child_start, child_end)[: args.max_grandchildren]

        lines.append(
            f"### `{contributor.label}` "
            f"(aligned `0x{child_start:04X}-0x{child_end:04X}`)"
        )
        lines.append("")
        lines.extend(render_window_summary_lines("Child", child_summary))
        lines.append(f"- Direct bytes into root: `{contributor.bytes_contributed}`")
        lines.append("")
        if child_disasm:
            lines.append("```asm")
            lines.extend(child_disasm)
            lines.append("```")
            lines.append("")
        if grandchildren:
            lines.append("Immediate upstream contributors:")
            for grand in grandchildren:
                token_text = ", ".join(grand.token_values[:3]) if grand.token_values else "none"
                if len(grand.token_values) > 3:
                    token_text += f", ... ({len(grand.token_values)} total)"
                lines.append(
                    f"- `{grand.label}` "
                    f"`bytes={grand.bytes_contributed}` "
                    f"`events={grand.events}` "
                    f"`short={grand.short_events}` "
                    f"`long={grand.long_events}` "
                    f"`wrapped={int(grand.wrapped)}` "
                    f"`tokens={token_text}`"
                )
        else:
            lines.append("Immediate upstream contributors:")
            lines.append("- none")
        lines.append("")

    report_out.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote unpacked contributor chain report: {report_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

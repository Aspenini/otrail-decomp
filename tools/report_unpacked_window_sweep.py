#!/usr/bin/env python3
"""Compare multiple unpacked payload windows using the real handoff trace."""

from __future__ import annotations

import argparse
from pathlib import Path

from unpacked_window_analysis import WindowSummary, compute_window_summary, load_trace_rows, parse_int


class WindowSpec:
    def __init__(self, start: int, size: int) -> None:
        self.start = start
        self.size = size

    @property
    def end(self) -> int:
        return self.start + self.size

    @property
    def label(self) -> str:
        return f"0x{self.start:04X}-0x{self.end:04X}"


def parse_window_spec(text: str) -> WindowSpec:
    start_text, size_text = text.split(":", 1)
    return WindowSpec(start=parse_int(start_text), size=parse_int(size_text))

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--payload", default="build/entry_bootstrap_replay_readable_heuristic.bin")
    parser.add_argument("--trace", default="build/entry_handoff_trace.csv")
    parser.add_argument(
        "--windows",
        default="0x0818:0x38,0x0F46:0x50,0x2000:0x40,0x2F00:0x40,0x0600:0x60",
    )
    parser.add_argument("--report-out", default="build/unpacked_window_sweep_report.md")
    args = parser.parse_args()

    payload_path = Path(args.payload)
    trace_path = Path(args.trace)
    report_out = Path(args.report_out)
    payload = payload_path.read_bytes()
    trace_rows = load_trace_rows(trace_path)
    windows = [parse_window_spec(item.strip()) for item in args.windows.split(",") if item.strip()]

    summaries: list[WindowSummary] = []
    for spec in windows:
        summaries.append(compute_window_summary(payload, trace_rows, spec.start, spec.size))

    summaries.sort(key=lambda row: row.score, reverse=True)

    lines: list[str] = []
    lines.append("# Unpacked Window Sweep Report")
    lines.append("")
    lines.append("## Inputs")
    lines.append("")
    lines.append(f"- Payload: `{payload_path}`")
    lines.append(f"- Trace: `{trace_path}`")
    lines.append(f"- Window specs: `{args.windows}`")
    lines.append("")
    lines.append("## Ranked Windows")
    lines.append("")
    for row in summaries:
        lines.append(
            f"- `{row.label}` "
            f"`score={row.score}` "
            f"`nonzero={row.nonzero}` "
            f"`zero={row.zero}` "
            f"`printable={row.printable}` "
            f"`ctrl={row.ctrl}` "
            f"`events={row.events}` "
            f"`literal_events={row.literal_events}` "
            f"`short_events={row.short_events}` "
            f"`long_events={row.long_events}` "
            f"`literal_bytes={row.literal_bytes}` "
            f"`short_bytes={row.short_bytes}` "
            f"`long_bytes={row.long_bytes}` "
            f"`wrapped_copy_events={row.wrapped_copy_events}` "
            f"`wrapped_copy_bytes={row.wrapped_copy_bytes}`"
        )
    lines.append("")
    lines.append("## Notes")
    lines.append("")
    lines.append("- Higher scores favor denser windows, more literal contribution, more control-like bytes, and fewer zeros.")
    lines.append("- `wrapped_copy_events` counts overlaps where the source address underflowed the linear destination window and only made sense after 16-bit wrapping.")
    lines.append("- This is a triage heuristic, not a proof that a window is executable code.")

    report_out.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote unpacked window sweep report: {report_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

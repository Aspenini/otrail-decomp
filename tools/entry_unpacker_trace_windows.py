#!/usr/bin/env python3
"""Summarize unpacker trace events by source-offset windows."""

from __future__ import annotations

import argparse
import csv
from dataclasses import dataclass, field
from pathlib import Path


@dataclass
class WindowStats:
    start: int
    end_excl: int
    events: int = 0
    src_bytes: int = 0
    out_bytes: int = 0
    by_type: dict[str, int] = field(default_factory=dict)

    def add(self, row: dict[str, str]) -> None:
        self.events += 1
        self.src_bytes += int(row["src_delta"])
        self.out_bytes += int(row["out_delta"])
        etype = row["type"]
        self.by_type[etype] = self.by_type.get(etype, 0) + 1

    def dominant_type(self) -> str:
        if not self.by_type:
            return "-"
        return max(self.by_type.items(), key=lambda kv: kv[1])[0]


def parse_int_auto(text: str) -> int:
    return int(text, 0)


def build_windows(start: int, end_excl: int, width: int) -> list[WindowStats]:
    windows: list[WindowStats] = []
    cur = start
    while cur < end_excl:
        nxt = min(cur + width, end_excl)
        windows.append(WindowStats(start=cur, end_excl=nxt))
        cur = nxt
    return windows


def choose_window(windows: list[WindowStats], src_abs_before: int) -> WindowStats | None:
    for w in windows:
        if w.start <= src_abs_before < w.end_excl:
            return w
    return None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--trace-csv", default="build/unpacker_trace.csv")
    parser.add_argument("--start", default="0x1274F")
    parser.add_argument("--end", default="0x12845")
    parser.add_argument("--window", default="0x10")
    parser.add_argument("--report-out", default="build/unpacker_trace_windows.md")
    args = parser.parse_args()

    start = parse_int_auto(args.start)
    end_inclusive = parse_int_auto(args.end)
    end_excl = end_inclusive + 1
    width = parse_int_auto(args.window)
    if width <= 0:
        raise ValueError("window width must be positive")

    windows = build_windows(start, end_excl, width)
    trace_path = Path(args.trace_csv)
    if not trace_path.exists():
        raise FileNotFoundError(f"missing trace csv: {trace_path}")

    total_events = 0
    with trace_path.open("r", encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            src_abs_before = parse_int_auto(row["src_abs_before"])
            w = choose_window(windows, src_abs_before)
            if w is None:
                continue
            w.add(row)
            total_events += 1

    lines = [
        "# Entry Unpacker Trace Window Summary",
        "",
        f"- trace csv: `{trace_path}`",
        f"- mapped range: `{hex(start)}` to `{hex(end_inclusive)}`",
        f"- window size: `{hex(width)}`",
        f"- mapped events: `{total_events}`",
        "",
        "| Window | Events | Src Bytes | Out Bytes | Dominant Type | Type Mix |",
        "|---|---:|---:|---:|---|---|",
    ]

    for w in windows:
        mix_parts: list[str] = []
        for etype, count in sorted(w.by_type.items()):
            mix_parts.append(f"{etype}:{count}")
        type_mix = ", ".join(mix_parts) if mix_parts else "-"
        lines.append(
            f"| `{hex(w.start)}-{hex(w.end_excl - 1)}` | "
            f"{w.events} | {w.src_bytes} | {w.out_bytes} | "
            f"{w.dominant_type()} | {type_mix} |"
        )

    report_path = Path(args.report_out)
    report_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

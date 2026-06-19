"""Shared analysis helpers for unpacked payload windows."""

from __future__ import annotations

import csv
from dataclasses import dataclass
from pathlib import Path


CONTROL_BYTES = {0xE8, 0xE9, 0xEA, 0x9A, 0xC3, 0xCB, 0xCF, 0xCD, 0xFA, 0xFB}


def parse_int(text: str) -> int:
    return int(text, 0)


def load_trace_rows(path: Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8", newline="") as handle:
        return list(csv.DictReader(handle))


def count_overlap_bytes(start: int, end: int, row_start: int, row_end: int) -> int:
    overlap_start = max(start, row_start)
    overlap_end = min(end, row_end)
    return max(0, overlap_end - overlap_start)


@dataclass
class ParsedTraceEvent:
    event_idx: int
    event_type: str
    out_before: int
    out_after: int
    literal: int
    token_u16: int
    ext: int
    back_disp: int
    copy_len: int

    @property
    def is_copy(self) -> bool:
        return self.event_type != "literal"

    @property
    def source_start(self) -> int:
        return self.out_before + self.back_disp

    @property
    def source_end(self) -> int:
        return self.source_start + self.copy_len


def parse_trace_events(trace_rows: list[dict[str, str]]) -> list[ParsedTraceEvent]:
    return [
        ParsedTraceEvent(
            event_idx=parse_int(row["event_idx"]),
            event_type=row["type"],
            out_before=parse_int(row["out_before"]),
            out_after=parse_int(row["out_after"]),
            literal=parse_int(row["literal"]),
            token_u16=parse_int(row["token_u16"]),
            ext=parse_int(row["ext"]),
            back_disp=parse_int(row["back_disp"]),
            copy_len=parse_int(row["copy_len"]),
        )
        for row in trace_rows
    ]


def find_output_writers(events: list[ParsedTraceEvent], start: int, end: int) -> list[ParsedTraceEvent]:
    return [event for event in events if count_overlap_bytes(start, end, event.out_before, event.out_after) > 0]


def find_copy_consumers(events: list[ParsedTraceEvent], start: int, end: int) -> list[ParsedTraceEvent]:
    return [
        event
        for event in events
        if event.is_copy and count_overlap_bytes(start, end, event.source_start, event.source_end) > 0
    ]


def find_pattern_occurrences(payload: bytes, pattern: bytes) -> list[int]:
    occurrences: list[int] = []
    search_from = 0
    while True:
        offset = payload.find(pattern, search_from)
        if offset < 0:
            return occurrences
        occurrences.append(offset)
        search_from = offset + 1


@dataclass
class WindowSummary:
    start: int
    size: int
    nonzero: int
    zero: int
    printable: int
    ctrl: int
    events: int
    literal_events: int
    short_events: int
    long_events: int
    literal_bytes: int
    short_bytes: int
    long_bytes: int
    wrapped_copy_events: int
    wrapped_copy_bytes: int
    score: int

    @property
    def end(self) -> int:
        return self.start + self.size

    @property
    def label(self) -> str:
        return f"0x{self.start:04X}-0x{self.end:04X}"


def compute_window_summary(
    payload: bytes,
    trace_rows: list[dict[str, str]],
    start: int,
    size: int,
) -> WindowSummary:
    end = min(len(payload), start + size)
    chunk = payload[start:end]
    nonzero = sum(byte != 0 for byte in chunk)
    zero = len(chunk) - nonzero
    printable = sum(32 <= byte < 127 for byte in chunk)
    ctrl = sum(byte in CONTROL_BYTES for byte in chunk)

    literal_events = 0
    short_events = 0
    long_events = 0
    literal_bytes = 0
    short_bytes = 0
    long_bytes = 0
    wrapped_copy_events = 0
    wrapped_copy_bytes = 0
    events = 0

    for row in trace_rows:
        out_before = parse_int(row["out_before"])
        out_after = parse_int(row["out_after"])
        overlap = count_overlap_bytes(start, end, out_before, out_after)
        if overlap == 0:
            continue

        events += 1
        if row["type"] == "literal":
            literal_events += 1
            literal_bytes += overlap
            continue

        back_disp = parse_int(row["back_disp"])
        if row["type"].startswith("short"):
            short_events += 1
            short_bytes += overlap
            if out_before + back_disp < 0:
                wrapped_copy_events += 1
                wrapped_copy_bytes += overlap
            continue

        if row["type"].startswith("long"):
            long_events += 1
            long_bytes += overlap
            if out_before + back_disp < 0:
                wrapped_copy_events += 1
                wrapped_copy_bytes += overlap

    score = nonzero + printable + ctrl * 6 + literal_bytes * 2 - zero - wrapped_copy_events * 6
    return WindowSummary(
        start=start,
        size=end - start,
        nonzero=nonzero,
        zero=zero,
        printable=printable,
        ctrl=ctrl,
        events=events,
        literal_events=literal_events,
        short_events=short_events,
        long_events=long_events,
        literal_bytes=literal_bytes,
        short_bytes=short_bytes,
        long_bytes=long_bytes,
        wrapped_copy_events=wrapped_copy_events,
        wrapped_copy_bytes=wrapped_copy_bytes,
        score=score,
    )

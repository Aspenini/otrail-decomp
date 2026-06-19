#!/usr/bin/env python3
"""Check frozen unpacked motif-family fixtures against the current payload and trace."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from unpacked_window_analysis import (
    find_copy_consumers,
    find_output_writers,
    find_pattern_occurrences,
    load_trace_rows,
    parse_int,
    parse_trace_events,
)


def event_ids(events) -> list[int]:
    return [event.event_idx for event in events]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fixtures", default="config/unpacked_motif_fixtures.json")
    parser.add_argument("--payload", default="build/entry_bootstrap_replay_readable_heuristic.bin")
    parser.add_argument("--trace", default="build/entry_handoff_trace.csv")
    args = parser.parse_args()

    root = Path(".").resolve()
    payload = (root / args.payload).read_bytes()
    trace_rows = load_trace_rows(root / args.trace)
    events = parse_trace_events(trace_rows)
    fixtures_cfg = json.loads((root / args.fixtures).read_text(encoding="utf-8"))

    fail_count = 0
    for fixture in fixtures_cfg.get("fixtures", []):
        start = parse_int(str(fixture["start"]))
        size = parse_int(str(fixture["size"]))
        end = start + size
        pattern = payload[start:end]
        occurrences = find_pattern_occurrences(payload, pattern)
        seed_start = occurrences[0] if occurrences else None

        requested_writers = find_output_writers(events, start, end)
        requested_consumers = find_copy_consumers(events, start, end)

        seed_writers = []
        seed_consumers = []
        if seed_start is not None:
            seed_writers = find_output_writers(events, seed_start, seed_start + size)
            seed_consumers = find_copy_consumers(events, seed_start, seed_start + size)

        checks = {
            "bytes_hex": pattern.hex().upper(),
            "occurrences": [f"0x{offset:04X}".upper() for offset in occurrences],
            "seed_start": None if seed_start is None else f"0x{seed_start:04X}".upper(),
            "requested_writer_event_ids": event_ids(requested_writers),
            "requested_consumer_event_ids": event_ids(requested_consumers),
            "seed_writer_event_ids": event_ids(seed_writers),
            "seed_consumer_event_ids": event_ids(seed_consumers),
        }

        expected = {
            "bytes_hex": str(fixture["expected_bytes_hex"]).upper(),
            "occurrences": [str(text).upper() for text in fixture["expected_occurrences"]],
            "seed_start": str(fixture["expected_seed_start"]).upper(),
            "requested_writer_event_ids": [int(value) for value in fixture["expected_requested_writer_event_ids"]],
            "requested_consumer_event_ids": [int(value) for value in fixture["expected_requested_consumer_event_ids"]],
            "seed_writer_event_ids": [int(value) for value in fixture["expected_seed_writer_event_ids"]],
            "seed_consumer_event_ids": [int(value) for value in fixture["expected_seed_consumer_event_ids"]],
        }

        mismatches = [
            f"{key} expected={expected[key]} actual={checks[key]}"
            for key in expected
            if checks[key] != expected[key]
        ]

        fixture_id = fixture.get("id", "<unnamed>")
        if mismatches:
            fail_count += 1
            print(f"[FAIL] {fixture_id}: " + "; ".join(mismatches))
            continue

        print(
            f"[PASS] {fixture_id}: seed={checks['seed_start']} "
            f"occurrences={len(checks['occurrences'])} "
            f"requested_writers={len(checks['requested_writer_event_ids'])}"
        )

    total = len(fixtures_cfg.get("fixtures", []))
    print("")
    print(f"Unpacked motif fixture summary: total={total} fail={fail_count}")
    return 1 if fail_count else 0


if __name__ == "__main__":
    raise SystemExit(main())

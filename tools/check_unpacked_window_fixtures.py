#!/usr/bin/env python3
"""Check frozen unpacked-window summary fixtures against the current payload and trace."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from unpacked_window_analysis import compute_window_summary, load_trace_rows, parse_int


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fixtures", default="config/unpacked_window_fixtures.json")
    parser.add_argument("--payload", default="build/entry_bootstrap_replay_readable_heuristic.bin")
    parser.add_argument("--trace", default="build/entry_handoff_trace.csv")
    args = parser.parse_args()

    root = Path(".").resolve()
    fixtures_cfg = json.loads((root / args.fixtures).read_text(encoding="utf-8"))
    payload = (root / args.payload).read_bytes()
    trace_rows = load_trace_rows(root / args.trace)

    fail_count = 0
    for fixture in fixtures_cfg.get("fixtures", []):
        summary = compute_window_summary(
            payload,
            trace_rows,
            parse_int(str(fixture["start"])),
            parse_int(str(fixture["size"])),
        )
        checks = {
            "nonzero": str(fixture["expected_nonzero"]),
            "zero": str(fixture["expected_zero"]),
            "printable": str(fixture["expected_printable"]),
            "ctrl": str(fixture["expected_ctrl"]),
            "events": str(fixture["expected_events"]),
            "literal_events": str(fixture["expected_literal_events"]),
            "short_events": str(fixture["expected_short_events"]),
            "long_events": str(fixture["expected_long_events"]),
            "literal_bytes": str(fixture["expected_literal_bytes"]),
            "short_bytes": str(fixture["expected_short_bytes"]),
            "long_bytes": str(fixture["expected_long_bytes"]),
            "wrapped_copy_events": str(fixture["expected_wrapped_copy_events"]),
            "wrapped_copy_bytes": str(fixture["expected_wrapped_copy_bytes"]),
            "score": str(fixture["expected_score"]),
        }
        actual = {
            "nonzero": str(summary.nonzero),
            "zero": str(summary.zero),
            "printable": str(summary.printable),
            "ctrl": str(summary.ctrl),
            "events": str(summary.events),
            "literal_events": str(summary.literal_events),
            "short_events": str(summary.short_events),
            "long_events": str(summary.long_events),
            "literal_bytes": str(summary.literal_bytes),
            "short_bytes": str(summary.short_bytes),
            "long_bytes": str(summary.long_bytes),
            "wrapped_copy_events": str(summary.wrapped_copy_events),
            "wrapped_copy_bytes": str(summary.wrapped_copy_bytes),
            "score": str(summary.score),
        }

        mismatches = [
            f"{key} expected={expected} actual={actual[key]}"
            for key, expected in checks.items()
            if actual[key] != expected
        ]

        fixture_id = fixture.get("id", "<unnamed>")
        if mismatches:
            fail_count += 1
            print(f"[FAIL] {fixture_id}: " + "; ".join(mismatches))
        else:
            print(
                f"[PASS] {fixture_id}: range={summary.label} "
                f"score={summary.score} nonzero={summary.nonzero} events={summary.events}"
            )

    total = len(fixtures_cfg.get("fixtures", []))
    print("")
    print(f"Unpacked window fixture summary: total={total} fail={fail_count}")
    return 1 if fail_count else 0


if __name__ == "__main__":
    raise SystemExit(main())

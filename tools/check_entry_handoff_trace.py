#!/usr/bin/env python3
"""Verify early prepared-stream unpacker event traces from the real EXE handoff."""

from __future__ import annotations

import argparse
import csv
import json
import subprocess
from pathlib import Path


def parse_kv_output(text: str) -> dict[str, str]:
    out: dict[str, str] = {}
    for line in text.splitlines():
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        out[key.strip()] = value.strip()
    return out


def summarize_events(trace_csv: Path, max_events: int) -> tuple[str, int, int, dict[str, str], dict[str, int]]:
    parts: list[str] = []
    last_src_after = 0
    last_out_after = 0
    last_row: dict[str, str] = {}
    ext_counts: dict[str, int] = {}

    with trace_csv.open(newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for index, row in enumerate(reader):
            if index >= max_events:
                break
            event_type = row["type"]
            if event_type == "literal":
                parts.append(f"L:{int(row['literal']):02X}")
            elif event_type == "short":
                parts.append(f"S:{row['back_disp']}:{row['copy_len']}")
            elif event_type == "long":
                parts.append(
                    f"G:{row['token_u16'][2:]}:{row['back_disp']}:{row['copy_len']}"
                )
            else:
                parts.append(event_type)

            last_src_after = int(row["src_abs_after"], 16)
            last_out_after = int(row["out_after"])
            last_row = row
            ext_value = row.get("ext", "")
            if ext_value and ext_value != "-1":
                ext_counts[ext_value] = ext_counts.get(ext_value, 0) + 1

    return "|".join(parts), last_src_after, last_out_after, last_row, ext_counts


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fixtures", default="config/entry_handoff_trace_fixtures.json")
    parser.add_argument("--dump-runner", default="build/entry_handoff_dump_stream")
    parser.add_argument("--trace-runner", default="build/entry_unpacker_trace")
    args = parser.parse_args()

    root = Path(".").resolve()
    fixtures_cfg = json.loads((root / args.fixtures).read_text(encoding="utf-8"))
    fixtures = fixtures_cfg.get("fixtures", [])
    dump_runner = root / args.dump_runner
    trace_runner = root / args.trace_runner
    if not dump_runner.exists():
        raise SystemExit(f"Missing handoff dump runner: {dump_runner}")
    if not trace_runner.exists():
        raise SystemExit(f"Missing unpacker trace runner: {trace_runner}")

    fail_count = 0
    for fixture in fixtures:
        fixture_id = fixture.get("id", "<unnamed>")
        dump_path = root / "build" / f"{fixture_id}_stream.bin"
        trace_path = root / "build" / f"{fixture_id}_trace.csv"
        max_events = int(fixture["max_events"])

        dump_result = parse_kv_output(
            subprocess.run(
                [
                    str(dump_runner),
                    str(root / fixture["exe_path"]),
                    str(fixture.get("load_seg", "0xA000")),
                    str(dump_path),
                ],
                check=True,
                capture_output=True,
                text=True,
            ).stdout
        )
        trace_result = parse_kv_output(
            subprocess.run(
                [
                    str(trace_runner),
                    str(dump_path),
                    "0",
                    str(int(fixture.get("dst_cap", 65536))),
                    str(int(fixture.get("mode", 1))),
                    str(max_events),
                    str(trace_path),
                ],
                check=True,
                capture_output=True,
                text=True,
            ).stdout
        )

        summary, src_after, out_after, last_row, ext_counts = summarize_events(trace_path, max_events)
        checks = {
            "dump_seed_word": str(fixture["expected_seed_word"]),
            "dump_first_gate_is_literal": str(int(fixture["expected_first_gate_is_literal"])),
            "trace_status": str(fixture["expected_status"]),
            "trace_events_written": str(max_events),
            "trace_src_used": str(int(fixture["expected_src_used"])),
            "trace_dst_written": str(int(fixture["expected_dst_written"])),
            "trace_src_after": str(int(fixture["expected_src_used"])),
            "trace_out_after": str(int(fixture["expected_dst_written"])),
        }
        if "expected_event_summary" in fixture:
            checks["trace_summary"] = str(fixture["expected_event_summary"])
        if "expected_last_event_type" in fixture:
            checks["last_event_type"] = str(fixture["expected_last_event_type"])
        if "expected_last_event_status" in fixture:
            checks["last_event_status"] = str(fixture["expected_last_event_status"])
        if "expected_last_event_token_u16" in fixture:
            checks["last_event_token_u16"] = str(fixture["expected_last_event_token_u16"])
        if "expected_last_event_ext" in fixture:
            checks["last_event_ext"] = str(fixture["expected_last_event_ext"])
        if "expected_last_event_back_disp" in fixture:
            checks["last_event_back_disp"] = str(int(fixture["expected_last_event_back_disp"]))
        if "expected_ext_0_count" in fixture:
            checks["ext_0_count"] = str(int(fixture["expected_ext_0_count"]))
        if "expected_ext_1_count" in fixture:
            checks["ext_1_count"] = str(int(fixture["expected_ext_1_count"]))
        actual = {
            "dump_seed_word": dump_result.get("seed_word", ""),
            "dump_first_gate_is_literal": dump_result.get("first_gate_is_literal", ""),
            "trace_status": trace_result.get("status", ""),
            "trace_events_written": trace_result.get("events_written", ""),
            "trace_src_used": trace_result.get("src_used", ""),
            "trace_dst_written": trace_result.get("dst_written", ""),
            "trace_summary": summary,
            "trace_src_after": str(src_after),
            "trace_out_after": str(out_after),
            "last_event_type": last_row.get("type", ""),
            "last_event_status": last_row.get("status", ""),
            "last_event_token_u16": last_row.get("token_u16", ""),
            "last_event_ext": last_row.get("ext", ""),
            "last_event_back_disp": last_row.get("back_disp", ""),
            "ext_0_count": str(ext_counts.get("0", 0)),
            "ext_1_count": str(ext_counts.get("1", 0)),
        }

        mismatches = []
        for key, expected in checks.items():
            if actual[key] != expected:
                mismatches.append(f"{key} expected={expected} actual={actual[key]}")

        if mismatches:
            fail_count += 1
            print(f"[FAIL] {fixture_id}: " + "; ".join(mismatches))
        else:
            print(
                f"[PASS] {fixture_id}: events={actual['trace_events_written']} "
                f"src_used={actual['trace_src_used']} dst_written={actual['trace_dst_written']}"
            )

    print("")
    print(f"Entry handoff trace summary: total={len(fixtures)} fail={fail_count}")
    return 1 if fail_count else 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Verify deterministic fixtures for the lifted unit_0002 handoff model."""

from __future__ import annotations

import argparse
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


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fixtures", default="config/entry_handoff_fixtures.json")
    parser.add_argument("--runner", default="build/entry_handoff_fixture")
    args = parser.parse_args()

    root = Path(".").resolve()
    fixtures_cfg = json.loads((root / args.fixtures).read_text(encoding="utf-8"))
    fixtures = fixtures_cfg.get("fixtures", [])
    runner = root / args.runner
    if not runner.exists():
        raise SystemExit(f"Missing handoff fixture runner: {runner}")

    fail_count = 0
    for fixture in fixtures:
        fixture_id = fixture.get("id", "<unnamed>")
        completed = subprocess.run(
            [
                str(runner),
                str(root / fixture["exe_path"]),
                str(fixture.get("load_seg", "0xA000")),
            ],
            check=True,
            capture_output=True,
            text=True,
        )
        result = parse_kv_output(completed.stdout)
        checks = {
            "status": str(fixture["expected_status"]),
            "entry_cs": str(fixture["expected_entry_cs"]),
            "stage0_copy_bytes": str(fixture["expected_stage0_copy_bytes"]),
            "stage0_segment_delta": str(fixture["expected_stage0_segment_delta"]),
            "stage1_total_paragraphs": str(fixture["expected_stage1_total_paragraphs"]),
            "unpacker_ds": str(fixture["expected_unpacker_ds"]),
            "unpacker_es": str(fixture["expected_unpacker_es"]),
            "stream_src_offset": str(int(fixture["expected_stream_src_offset"])),
            "stream_dst_offset": str(int(fixture["expected_stream_dst_offset"])),
            "seed_word": str(fixture["expected_seed_word"]),
            "seed_bits": str(fixture["expected_seed_bits"]),
            "seed_bits_left": str(int(fixture["expected_seed_bits_left"])),
            "first_gate_is_literal": str(int(fixture["expected_first_gate_is_literal"])),
        }

        mismatches = []
        for key, expected in checks.items():
            actual = result.get(key, "")
            if actual != expected:
                mismatches.append(f"{key} expected={expected} actual={actual}")

        if mismatches:
            fail_count += 1
            print(f"[FAIL] {fixture_id}: " + "; ".join(mismatches))
        else:
            print(
                f"[PASS] {fixture_id}: seed_word={result['seed_word']} "
                f"first_gate_is_literal={result['first_gate_is_literal']}"
            )

    print("")
    print(f"Entry handoff fixture summary: total={len(fixtures)} fail={fail_count}")
    return 1 if fail_count else 0


if __name__ == "__main__":
    raise SystemExit(main())

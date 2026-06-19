#!/usr/bin/env python3
"""Verify composed entry bootstrap fixtures (loader relocation + unpacker replay)."""

from __future__ import annotations

import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from typing import Any


def parse_kv_output(text: str) -> dict[str, str]:
    out: dict[str, str] = {}
    for line in text.splitlines():
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        out[key.strip()] = value.strip()
    return out


def run_fixture(runner: Path, exe_path: Path, fixture: dict[str, Any]) -> dict[str, str]:
    completed = subprocess.run(
        [
            str(runner),
            str(exe_path),
            str(fixture.get("load_seg", "0xA000")),
            str(fixture.get("mode", 1)),
        ],
        check=True,
        capture_output=True,
        text=True,
    )
    return parse_kv_output(completed.stdout)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fixtures", default="config/entry_bootstrap_fixtures.json")
    parser.add_argument("--runner", default="build/entry_bootstrap_replay")
    args = parser.parse_args()

    root = Path(".").resolve()
    fixtures_cfg = json.loads((root / args.fixtures).read_text(encoding="utf-8"))
    fixtures = fixtures_cfg.get("fixtures", [])
    runner = root / args.runner
    if not runner.exists():
        raise SystemExit(f"Missing bootstrap runner: {runner}")

    fail_count = 0
    for fixture in fixtures:
        fixture_id = fixture.get("id", "<unnamed>")
        exe_path = root / fixture["exe_path"]
        result = run_fixture(runner, exe_path, fixture)
        output_file = root / result["output_file"]
        digest = hashlib.sha256(output_file.read_bytes()).hexdigest()

        checks = {
            "status": str(fixture["expected_status"]),
            "stage0_copy_bytes": str(fixture["expected_stage0_copy_bytes"]),
            "stage0_segment_delta": str(fixture["expected_stage0_segment_delta"]),
            "stage1_total_paragraphs": str(fixture["expected_stage1_total_paragraphs"]),
            "relocated_cs": str(fixture["expected_relocated_cs"]),
            "unpacker_ds": str(fixture["expected_unpacker_ds"]),
            "unpacker_es": str(fixture["expected_unpacker_es"]),
            "pass_schedule": str(fixture["expected_pass_schedule"]),
            "pass_trace": str(fixture["expected_pass_trace"]),
            "src_used": str(int(fixture["expected_src_used"])),
            "dst_written": str(int(fixture["expected_dst_written"])),
        }

        mismatches = []
        for key, expected in checks.items():
            actual = result.get(key, "")
            if actual != expected:
                mismatches.append(f"{key} expected={expected} actual={actual}")
        if digest != str(fixture["expected_sha256"]):
            mismatches.append(
                f"sha256 expected={fixture['expected_sha256']} actual={digest}"
            )

        if mismatches:
            fail_count += 1
            print(f"[FAIL] {fixture_id}: " + "; ".join(mismatches))
        else:
            print(
                f"[PASS] {fixture_id}: status={result['status']} "
                f"dst_written={result['dst_written']} sha256={digest}"
            )

    print("")
    print(f"Bootstrap fixture summary: total={len(fixtures)} fail={fail_count}")
    return 1 if fail_count else 0


if __name__ == "__main__":
    raise SystemExit(main())

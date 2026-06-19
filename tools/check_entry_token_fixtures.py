#!/usr/bin/env python3
"""Verify lifted unit_0003 / unit_0004 token fixtures against the real EXE handoff."""

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
    parser.add_argument("--fixtures", default="config/entry_token_fixtures.json")
    parser.add_argument("--runner", default="build/entry_token_fixture")
    args = parser.parse_args()

    root = Path(".").resolve()
    fixtures_cfg = json.loads((root / args.fixtures).read_text(encoding="utf-8"))
    fixtures = fixtures_cfg.get("fixtures", [])
    runner = root / args.runner
    if not runner.exists():
        raise SystemExit(f"Missing token fixture runner: {runner}")

    fail_count = 0
    for fixture in fixtures:
        fixture_id = fixture.get("id", "<unnamed>")
        result = parse_kv_output(
            subprocess.run(
                [
                    str(runner),
                    str(root / fixture["exe_path"]),
                    str(fixture.get("load_seg", "0xA000")),
                    str(int(fixture["target_event"])),
                ],
                check=True,
                capture_output=True,
                text=True,
            ).stdout
        )

        checks = {
            "status": str(fixture["expected_status"]),
            "event_type": str(fixture["expected_event_type"]),
            "token_kind": str(int(fixture["expected_token_kind"])),
            "backref_disp": str(int(fixture["expected_backref_disp"])),
            "copy_len": str(int(fixture["expected_copy_len"])),
            "control_byte": str(int(fixture["expected_control_byte"])),
            "needs_unit_0004_resolution": str(
                int(fixture["expected_needs_unit_0004_resolution"])
            ),
            "cursor_src_pos": str(int(fixture["expected_cursor_src_pos"])),
            "cursor_bitbuf": str(fixture["expected_cursor_bitbuf"]),
            "cursor_bits_left": str(int(fixture["expected_cursor_bits_left"])),
        }

        if "expected_control_kind" in fixture:
            checks["control_kind"] = str(int(fixture["expected_control_kind"]))
        if "expected_control_copy_len" in fixture:
            checks["control_copy_len"] = str(int(fixture["expected_control_copy_len"]))

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
                f"[PASS] {fixture_id}: type={result['event_type']} "
                f"disp={result.get('backref_disp', '')} len={result.get('copy_len', '')}"
            )

    print("")
    print(f"Entry token fixture summary: total={len(fixtures)} fail={fail_count}")
    return 1 if fail_count else 0


if __name__ == "__main__":
    raise SystemExit(main())

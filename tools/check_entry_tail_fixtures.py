#!/usr/bin/env python3
"""Verify lifted unit_0004 / unit_0005 / unit_0006 tail fixtures."""

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
    parser.add_argument("--fixtures", default="config/entry_tail_fixtures.json")
    parser.add_argument("--runner", default="build/entry_tail_fixture")
    args = parser.parse_args()

    root = Path(".").resolve()
    fixtures_cfg = json.loads((root / args.fixtures).read_text(encoding="utf-8"))
    fixtures = fixtures_cfg.get("fixtures", [])
    runner = root / args.runner
    if not runner.exists():
        raise SystemExit(f"Missing entry tail fixture runner: {runner}")

    fail_count = 0
    for fixture in fixtures:
        fixture_id = fixture.get("id", "<unnamed>")
        if fixture["mode"] == "slice":
            cmd = [str(runner), "slice"]
        elif fixture["mode"] == "bootstrap":
            cmd = [
                str(runner),
                "bootstrap",
                str(fixture["popped_bx"]),
                str(fixture["first_stream_byte"]),
                str(fixture["control_word"]),
            ]
            if int(fixture["control_word"], 0) == 1:
                cmd.extend(
                    [
                        str(fixture["jump_off"]),
                        str(fixture["jump_seg_word"]),
                        str(fixture["stack_off"]),
                        str(fixture["stack_seg_rel"]),
                    ]
                )
        else:
            cmd = [
                str(runner),
                str(fixture["mode"]),
                str(fixture["bx_seed"]),
                str(fixture["dx_rel"]),
                str(fixture["di"]),
            ]
            if fixture["mode"] == "byte":
                cmd.append(str(fixture["byte_value"]))
            else:
                cmd.append(str(fixture["control_word"]))
                if int(fixture["control_word"], 0) == 1:
                    cmd.extend(
                        [
                            str(fixture["jump_off"]),
                            str(fixture["jump_seg_word"]),
                            str(fixture["stack_off"]),
                            str(fixture["stack_seg_rel"]),
                        ]
                    )

        result = parse_kv_output(
            subprocess.run(cmd, check=True, capture_output=True, text=True).stdout
        )

        checks = {"status": "ok", "kind": str(fixture["expected_kind"])}
        for key, value in fixture.items():
            if key.startswith("expected_") and key not in {"expected_kind"}:
                checks[key.removeprefix("expected_")] = str(value)

        mismatches = []
        for key, expected in checks.items():
            actual = result.get(key, "")
            if actual != expected:
                mismatches.append(f"{key} expected={expected} actual={actual}")

        if mismatches:
            fail_count += 1
            print(f"[FAIL] {fixture_id}: " + "; ".join(mismatches))
        else:
            print(f"[PASS] {fixture_id}: kind={result['kind']}")

    print("")
    print(f"Entry tail fixture summary: total={len(fixtures)} fail={fail_count}")
    return 1 if fail_count else 0


if __name__ == "__main__":
    raise SystemExit(main())

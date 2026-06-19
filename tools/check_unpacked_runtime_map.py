#!/usr/bin/env python3
"""Check C runtime-region descriptors against the JSON runtime map."""

from __future__ import annotations

import argparse
import json
import subprocess
from pathlib import Path
from typing import Any


ROLE_KIND = {
    "source_envelope": "source_envelope",
    "source_neighborhood": "source_neighborhood",
    "root_window": "root_window",
    "exact_clone": "exact_clone",
    "exact_clone_head": "exact_clone_head",
    "near_clone": "near_clone",
}


def parse_int(value: Any) -> int:
    if isinstance(value, int):
        return value
    return int(str(value), 0)


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
    parser.add_argument("--runtime-map", default="config/unpacked_runtime_map.json")
    parser.add_argument("--runner", default="build/unpacked_runtime_fixture")
    args = parser.parse_args()

    root = Path(".").resolve()
    runtime_map_path = root / args.runtime_map
    runner = root / args.runner

    runtime_map = json.loads(runtime_map_path.read_text(encoding="utf-8"))
    expected_regions = [
        region
        for region in runtime_map.get("regions", [])
        if isinstance(region, dict) and region.get("family") == "dense_0f46"
    ]

    result = subprocess.run(
        [str(runner), "map0f46"],
        check=True,
        capture_output=True,
        text=True,
    )
    actual = parse_kv_output(result.stdout)

    mismatches: list[str] = []
    if actual.get("status") != "ok":
        mismatches.append(f"status expected=ok actual={actual.get('status', '')}")
    if actual.get("kind") != "map0f46":
        mismatches.append(f"kind expected=map0f46 actual={actual.get('kind', '')}")

    actual_count = int(actual.get("region_count") or -1)
    if actual_count != len(expected_regions):
        mismatches.append(f"region_count expected={len(expected_regions)} actual={actual_count}")

    for index, expected in enumerate(expected_regions):
        raw = actual.get(f"region_{index}")
        if not raw:
            mismatches.append(f"missing region_{index}")
            continue
        fields = raw.split("|", 7)
        if len(fields) != 8:
            mismatches.append(f"region_{index} malformed={raw}")
            continue

        start = parse_int(expected.get("start"))
        end = parse_int(expected.get("end"))
        expected_row = [
            str(expected.get("id")),
            str(expected.get("family")),
            str(expected.get("role")),
            f"0x{start:04X}",
            f"0x{end:04X}",
            ROLE_KIND.get(str(expected.get("role")), "unknown"),
            str(end - start),
            str(expected.get("purpose")),
        ]

        for field_name, expected_value, actual_value in zip(
            ["id", "family", "role", "start", "end", "kind", "size", "purpose"],
            expected_row,
            fields,
        ):
            if actual_value != expected_value:
                mismatches.append(
                    f"region_{index}.{field_name} expected={expected_value} actual={actual_value}"
                )

    if mismatches:
        print("[FAIL] runtime_map_0f46: " + "; ".join(mismatches))
        return 1

    print(f"[PASS] runtime_map_0f46: regions={len(expected_regions)} checked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

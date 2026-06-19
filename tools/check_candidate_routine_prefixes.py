#!/usr/bin/env python3
"""Check candidate routine-prefix scanner output against fixture expectations."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any

from report_candidate_routine_prefixes import (
    RoutinePrefixCandidate,
    containing_runtime_regions,
    load_runtime_regions,
    scan_image,
)


def parse_int(value: Any) -> int:
    if isinstance(value, int):
        return value
    return int(str(value), 0)


def parse_far_call(value: str) -> tuple[int, int]:
    segment, offset = value.split(":", 1)
    return parse_int(segment), parse_int(offset)


def candidate_key(candidate: RoutinePrefixCandidate) -> tuple[int, int, int, int, int, int, int]:
    return (
        candidate.offset,
        candidate.loop_count,
        candidate.far_call_segment,
        candidate.far_call_offset,
        candidate.save_ax_bp_disp,
        candidate.save_bx_bp_disp,
        candidate.save_dx_bp_disp,
    )


def expected_key(row: dict[str, Any]) -> tuple[int, int, int, int, int, int, int]:
    segment, offset = parse_far_call(str(row["far_call"]))
    saves = row["saves"]
    return (
        parse_int(row["offset"]),
        parse_int(row["loop_count"]),
        segment,
        offset,
        int(saves[0]),
        int(saves[1]),
        int(saves[2]),
    )


def region_ids_for_offset(regions: list[dict[str, Any]], offset: int) -> list[str]:
    return [str(region.get("id")) for region in containing_runtime_regions(regions, offset)]


def format_key(row: tuple[int, int, int, int, int, int, int]) -> str:
    return (
        f"offset=0x{row[0]:04X} loop_count=0x{row[1]:04X} "
        f"far_call=0x{row[2]:04X}:0x{row[3]:04X} saves={row[4]},{row[5]},{row[6]}"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fixtures", default="config/candidate_routine_prefix_fixtures.json")
    parser.add_argument("--runtime-map", default="config/unpacked_runtime_map.json")
    args = parser.parse_args()

    root = Path(".").resolve()
    fixtures = json.loads((root / args.fixtures).read_text(encoding="utf-8"))
    runtime_regions = load_runtime_regions(root / args.runtime_map)
    fail_count = 0

    for image in fixtures.get("images", []):
        image_id = str(image["id"])
        path = root / str(image["path"])
        if not path.exists():
            print(f"[FAIL] {image_id}: missing image {path}")
            fail_count += 1
            continue

        actual = sorted(candidate_key(candidate) for candidate in scan_image(image_id, path.read_bytes()))
        expected = sorted(expected_key(row) for row in image.get("expected", []))

        missing = [row for row in expected if row not in actual]
        unexpected = [row for row in actual if row not in expected]
        if missing or unexpected:
            fail_count += 1
            details: list[str] = []
            details.extend(f"missing {format_key(row)}" for row in missing)
            details.extend(f"unexpected {format_key(row)}" for row in unexpected)
            print(f"[FAIL] {image_id}: " + "; ".join(details))
            continue

        region_mismatches: list[str] = []
        for row in image.get("expected", []):
            if "expected_regions" not in row:
                continue
            offset = parse_int(row["offset"])
            expected_regions = [str(region_id) for region_id in row["expected_regions"]]
            actual_regions = region_ids_for_offset(runtime_regions, offset)
            if actual_regions != expected_regions:
                region_mismatches.append(
                    f"0x{offset:04X} regions expected={expected_regions} actual={actual_regions}"
                )
        if region_mismatches:
            fail_count += 1
            print(f"[FAIL] {image_id}: " + "; ".join(region_mismatches))
            continue

        print(f"[PASS] {image_id}: candidates={len(actual)}")

    total = len(fixtures.get("images", []))
    print("")
    print(f"Candidate routine-prefix fixture summary: total={total} fail={fail_count}")
    return 1 if fail_count else 0


if __name__ == "__main__":
    sys.exit(main())

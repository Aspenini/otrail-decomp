#!/usr/bin/env python3
"""Report configured and whole-binary match progress."""

from __future__ import annotations

import argparse
import json
from collections import defaultdict
from pathlib import Path
from typing import Any


def load_config(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def pct(numerator: int, denominator: int) -> float:
    if denominator == 0:
        return 0.0
    return (numerator / denominator) * 100.0


def format_ratio(numerator: int, denominator: int, zero_label: str = "n/a") -> str:
    if denominator == 0:
        return f"{numerator}/{denominator} ({zero_label})"
    return f"{numerator}/{denominator} ({pct(numerator, denominator):.2f}%)"


def resolve_path(project_root: Path, raw: str | None) -> Path | None:
    if not raw:
        return None
    candidate = Path(raw)
    if candidate.is_absolute():
        return candidate
    return project_root / candidate


def parse_int(value: Any) -> int:
    if isinstance(value, int):
        return value
    return int(str(value), 0)


def merge_intervals(intervals: list[tuple[int, int]]) -> list[tuple[int, int]]:
    merged: list[tuple[int, int]] = []
    for start, end in sorted(intervals):
        if not merged or start > merged[-1][1]:
            merged.append((start, end))
            continue
        prev_start, prev_end = merged[-1]
        merged[-1] = (prev_start, max(prev_end, end))
    return merged


def sum_intervals(intervals: list[tuple[int, int]]) -> int:
    return sum(end - start for start, end in intervals)


def classify_unit_origin(unit: dict[str, Any]) -> str:
    expected_hash = unit.get("expected_sha256")
    expected_hex = unit.get("expected_bytes_hex")
    size_bytes = int(unit.get("size_bytes") or 0)

    if unit.get("materialize_from_binary", False):
        return "baseline"
    if expected_hash or expected_hex or size_bytes > 0:
        return "authored"
    return "auxiliary"


def print_runtime_progress(project_root: Path, map_path: Path, fixtures_path: Path) -> None:
    if not map_path.is_file():
        return

    data = load_config(map_path)
    regions = data.get("regions", [])
    if not isinstance(regions, list):
        raise SystemExit(f"Invalid runtime map: expected regions list in {map_path}")

    payload_size = int(data.get("payload_size_bytes") or 0)
    fixture_ids: set[str] = set()
    if fixtures_path.is_file():
        fixtures_data = load_config(fixtures_path)
        fixture_ids = {
            str(fixture.get("id"))
            for fixture in fixtures_data.get("fixtures", [])
            if isinstance(fixture, dict) and fixture.get("id")
        }

    intervals: list[tuple[int, int]] = []
    family_intervals: dict[str, list[tuple[int, int]]] = defaultdict(list)
    family_region_counts: dict[str, int] = defaultdict(int)
    role_counts: dict[str, int] = defaultdict(int)
    fixture_backed = 0

    for region in regions:
        if not isinstance(region, dict):
            raise SystemExit(f"Invalid runtime map region in {map_path}")
        start = parse_int(region.get("start"))
        end = parse_int(region.get("end"))
        if end <= start:
            raise SystemExit(f"Invalid runtime range for {region.get('id')}: end <= start")
        if payload_size and end > payload_size:
            raise SystemExit(f"Invalid runtime range for {region.get('id')}: range exceeds payload")

        fixture_id = str(region.get("fixture_id") or "")
        if fixture_ids and fixture_id not in fixture_ids:
            raise SystemExit(f"Runtime region {region.get('id')} references missing fixture {fixture_id}")
        if fixture_id:
            fixture_backed += 1

        family = str(region.get("family") or "unknown")
        role = str(region.get("role") or "unknown")
        interval = (start, end)
        intervals.append(interval)
        family_intervals[family].append(interval)
        family_region_counts[family] += 1
        role_counts[role] += 1

    merged = merge_intervals(intervals)
    checked_bytes = sum_intervals(intervals)
    unique_checked_bytes = sum_intervals(merged)

    print("Post-unpack runtime recovery")
    if payload_size:
        print(f"- Payload bytes: {payload_size}")
    print(f"- Checked runtime regions: {len(regions)}")
    print(f"- Fixture-backed regions: {fixture_backed}")
    print(f"- Checked bytes including clone/overlap spans: {checked_bytes}")
    if payload_size:
        print(
            "- Unique checked runtime bytes: "
            f"{unique_checked_bytes}/{payload_size} ({pct(unique_checked_bytes, payload_size):.2f}%)"
        )
    else:
        print(f"- Unique checked runtime bytes: {unique_checked_bytes}")

    print("- Runtime families:")
    for family in sorted(family_intervals):
        family_unique = sum_intervals(merge_intervals(family_intervals[family]))
        print(f"  - {family}: {family_region_counts[family]} regions / {family_unique} unique bytes")
    print("- Region roles:")
    for role in sorted(role_counts):
        print(f"  - {role}: {role_counts[role]}")
    print("- This is checked unpacked-payload recovery, separate from packed EXE src/unit semantic-lift bytes.")
    print("")


def print_progress_status(path: Path) -> None:
    if not path.is_file():
        return
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError):
        print("Active focus (status file unreadable)")
        print(f"- See: {path}")
        print("")
        return

    phase = data.get("phase")
    focus = data.get("current_focus")
    updated = data.get("last_updated")
    recent = data.get("recent_done")
    next_up = data.get("next_up")

    print("Active focus")
    if updated:
        print(f"- Last updated: {updated}")
    if phase:
        print(f"- Phase: {phase}")
    if focus:
        print(f"- Now: {focus}")
    if isinstance(recent, list) and recent:
        print("- Recently done:")
        for item in recent[-8:]:
            print(f"  - {item}")
    if isinstance(next_up, list) and next_up:
        print("- Next up:")
        for item in next_up[:8]:
            print(f"  - {item}")
    print("")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--config",
        default="config/functions.json",
        help="Path to functions metadata JSON file",
    )
    parser.add_argument(
        "--status",
        default="config/progress_status.json",
        help="Optional JSON with current_focus / phase for human-readable progress",
    )
    parser.add_argument(
        "--runtime-map",
        default="config/unpacked_runtime_map.json",
        help="Optional JSON with checked post-unpack runtime regions",
    )
    parser.add_argument(
        "--runtime-fixtures",
        default="config/unpacked_runtime_fixtures.json",
        help="Optional JSON with runtime fixture IDs used to validate the runtime map",
    )
    args = parser.parse_args()

    config_path = Path(args.config).resolve()
    project_root = config_path.parent.parent
    status_path = resolve_path(project_root, args.status) or Path(args.status).resolve()
    runtime_map_path = resolve_path(project_root, args.runtime_map) or Path(args.runtime_map).resolve()
    runtime_fixtures_path = resolve_path(project_root, args.runtime_fixtures) or Path(args.runtime_fixtures).resolve()
    print_progress_status(status_path)

    cfg = load_config(config_path)
    binaries = cfg.get("binaries", [])
    units = cfg.get("units", [])

    if not isinstance(binaries, list) or not isinstance(units, list):
        raise SystemExit("Invalid config: expected list fields for binaries and units")

    binary_size_map: dict[str, int] = {}
    binary_name_map: dict[str, str] = {}
    for binary in binaries:
        binary_id = binary.get("id")
        if not binary_id:
            continue
        binary_size_map[binary_id] = int(binary.get("size_bytes") or 0)
        binary_name_map[binary_id] = binary.get("name") or binary_id

    configured_unit_total = 0
    configured_unit_matched = 0
    configured_bytes_total = 0
    configured_bytes_matched = 0

    origin_totals: dict[str, dict[str, int]] = defaultdict(
        lambda: {"units": 0, "matched_units": 0, "bytes": 0, "matched_bytes": 0}
    )
    source_state_totals: dict[str, dict[str, int]] = defaultdict(
        lambda: {"units": 0, "bytes": 0}
    )
    per_binary_matched_bytes: dict[str, int] = defaultdict(int)
    for unit in units:
        size_bytes = int(unit.get("size_bytes") or 0)
        unit_binary = unit.get("binary_id") or "unknown"
        unit_status = str(unit.get("status") or "unmatched")
        origin = classify_unit_origin(unit)
        source_state = str(unit.get("source_state") or "").strip()
        configured_unit_total += 1
        configured_bytes_total += size_bytes
        origin_totals[origin]["units"] += 1
        origin_totals[origin]["bytes"] += size_bytes
        if source_state:
            source_state_totals[source_state]["units"] += 1
            source_state_totals[source_state]["bytes"] += size_bytes

        if unit_status == "matched":
            configured_unit_matched += 1
            configured_bytes_matched += size_bytes
            per_binary_matched_bytes[unit_binary] += size_bytes
            origin_totals[origin]["matched_units"] += 1
            origin_totals[origin]["matched_bytes"] += size_bytes

    print("Configured progress")
    print(
        f"- Units: {configured_unit_matched}/{configured_unit_total} "
        f"({pct(configured_unit_matched, configured_unit_total):.2f}%)"
    )
    print(
        f"- Bytes: {configured_bytes_matched}/{configured_bytes_total} "
        f"({pct(configured_bytes_matched, configured_bytes_total):.2f}%)"
    )
    print("")
    print("Verification origin")

    baseline = origin_totals["baseline"]
    print(
        "- Baseline materialized units: "
        f"{format_ratio(baseline['matched_units'], baseline['units'], 'retired')}"
    )
    print(
        "- Baseline materialized bytes: "
        f"{format_ratio(baseline['matched_bytes'], baseline['bytes'], 'retired')}"
    )

    authored = origin_totals["authored"]
    print(
        f"- Authored rebuild units: {format_ratio(authored['matched_units'], authored['units'])}"
    )
    print(
        f"- Authored rebuild bytes: {format_ratio(authored['matched_bytes'], authored['bytes'])}"
    )

    auxiliary = origin_totals["auxiliary"]
    if auxiliary["units"]:
        print(f"- Auxiliary/untyped configured units: {auxiliary['units']}")
    print("")

    if source_state_totals:
        print("Source state tags")
        for state in sorted(source_state_totals):
            counts = source_state_totals[state]
            print(
                f"- {state}: {counts['units']} units / {counts['bytes']} bytes "
                f"({pct(counts['bytes'], configured_bytes_total):.2f}% of configured bytes)"
            )
        semantic = source_state_totals.get("semantic_lift", {"bytes": 0})
        print(
            "- Readable semantic-lift bytes: "
            f"{semantic['bytes']}/{configured_bytes_total} "
            f"({pct(semantic['bytes'], configured_bytes_total):.2f}%)"
        )
        print("")

    source_total = 0
    placeholder_sources = 0
    non_placeholder_sources = 0
    missing_sources = 0
    seen_sources: set[Path] = set()
    for unit in units:
        source_path = resolve_path(project_root, unit.get("source_path"))
        if source_path is None or source_path in seen_sources:
            continue
        seen_sources.add(source_path)
        source_total += 1
        try:
            text = source_path.read_text(encoding="utf-8")
        except OSError:
            missing_sources += 1
            continue
        if "placeholder" in text.lower():
            placeholder_sources += 1
        else:
            non_placeholder_sources += 1

    print("Source inventory")
    print(f"- Source files inspected: {source_total}")
    print(f"- Placeholder-tagged source files: {placeholder_sources}")
    print(f"- Non-placeholder source files: {non_placeholder_sources}")
    if missing_sources:
        print(f"- Missing/unreadable source files: {missing_sources}")
    print("- Placeholder detection is heuristic and looks for the literal marker in source text.")
    print("")

    print("Whole-binary progress")

    grand_matched = 0
    grand_total = 0
    for binary_id, total in binary_size_map.items():
        matched = per_binary_matched_bytes.get(binary_id, 0)
        grand_matched += matched
        grand_total += total
        name = binary_name_map.get(binary_id, binary_id)
        print(f"- {name}: {matched}/{total} ({pct(matched, total):.2f}%)")

    print(
        f"- Grand total: {grand_matched}/{grand_total} "
        f"({pct(grand_matched, grand_total):.2f}%)"
    )
    print("")
    print_runtime_progress(project_root, runtime_map_path, runtime_fixtures_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

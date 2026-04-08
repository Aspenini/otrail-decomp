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


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--config",
        default="config/functions.json",
        help="Path to functions metadata JSON file",
    )
    args = parser.parse_args()

    cfg = load_config(Path(args.config).resolve())
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

    per_binary_matched_bytes: dict[str, int] = defaultdict(int)
    for unit in units:
        size_bytes = int(unit.get("size_bytes") or 0)
        unit_binary = unit.get("binary_id") or "unknown"
        unit_status = str(unit.get("status") or "unmatched")
        configured_unit_total += 1
        configured_bytes_total += size_bytes

        if unit_status == "matched":
            configured_unit_matched += 1
            configured_bytes_matched += size_bytes
            per_binary_matched_bytes[unit_binary] += size_bytes

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
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

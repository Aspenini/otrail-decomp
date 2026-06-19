#!/usr/bin/env python3
"""Materialize configured baseline unit bytes from original binary slices."""

from __future__ import annotations

import argparse
import fnmatch
import json
from pathlib import Path
from typing import Any


def load_config(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def resolve(project_root: Path, raw: str) -> Path:
    path = Path(raw)
    return path if path.is_absolute() else project_root / path


def matches_filters(unit: dict[str, Any], patterns: list[str]) -> bool:
    if not patterns:
        return True

    fields = [
        str(unit.get("id") or ""),
        str(unit.get("logical_name") or ""),
        str(unit.get("source_path") or ""),
        str(unit.get("rebuilt_path") or ""),
    ]
    return any(
        fnmatch.fnmatchcase(field, pattern)
        for pattern in patterns
        for field in fields
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--config", default="config/functions.json")
    parser.add_argument(
        "--match",
        action="append",
        default=[],
        help="Shell-style glob against id, logical_name, source_path, or rebuilt_path",
    )
    parser.add_argument(
        "--quiet",
        action="store_true",
        help="Suppress per-unit materialization logs and print only the summary",
    )
    args = parser.parse_args()

    config_path = Path(args.config).resolve()
    project_root = config_path.parent.parent
    cfg = load_config(config_path)

    binaries = cfg.get("binaries", [])
    units = cfg.get("units", [])
    binary_data: dict[str, bytes] = {}

    for binary in binaries:
        binary_id = binary.get("id")
        binary_path = binary.get("path")
        if not binary_id or not binary_path:
            continue
        binary_data[binary_id] = resolve(project_root, binary_path).read_bytes()

    generated = 0
    selected = 0
    for unit in units:
        if not unit.get("materialize_from_binary", False):
            continue
        if not matches_filters(unit, args.match):
            continue
        selected += 1
        binary_id = unit.get("binary_id")
        rebuilt_path = unit.get("rebuilt_path")
        original_offset = unit.get("original_offset")
        size_bytes = unit.get("size_bytes")
        name = unit.get("logical_name") or unit.get("id") or "<unnamed>"

        if binary_id not in binary_data:
            raise SystemExit(f"Unit {name}: unknown binary_id {binary_id}")
        if rebuilt_path is None:
            raise SystemExit(f"Unit {name}: rebuilt_path missing")
        if original_offset is None or size_bytes is None:
            raise SystemExit(f"Unit {name}: original_offset/size_bytes required")

        offset = int(original_offset)
        size = int(size_bytes)
        chunk = binary_data[binary_id][offset : offset + size]
        if len(chunk) != size:
            raise SystemExit(
                f"Unit {name}: slice out of bounds offset={offset} size={size}"
            )

        out_path = resolve(project_root, rebuilt_path)
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_bytes(chunk)
        generated += 1
        if not args.quiet:
            print(f"[OK] {name}: wrote {size} bytes -> {out_path}")

    print(f"Materialized baseline candidates: {generated}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Materialize authored byte-array units into rebuilt artifacts."""

from __future__ import annotations

import argparse
import fnmatch
import json
import re
from pathlib import Path
from typing import Any

ARRAY_RE = re.compile(
    r"(?:static\s+)?const\s+(?:unsigned\s+char|uint8_t)\s+([A-Za-z0-9_]+)\s*\[\s*\]\s*=\s*\{(.*?)\};",
    re.S,
)
BYTE_RE = re.compile(r"0x([0-9A-Fa-f]{2})")


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


def extract_bytes_from_source(source_path: Path) -> bytes:
    text = source_path.read_text(encoding="utf-8")
    match = ARRAY_RE.search(text)
    if not match:
        raise SystemExit(f"No supported byte array found in {source_path}")

    body = match.group(2)
    byte_values = [int(token, 16) for token in BYTE_RE.findall(body)]
    if not byte_values:
        raise SystemExit(f"No byte literals found in {source_path}")
    return bytes(byte_values)


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
        help="Suppress per-unit logs and print only the summary",
    )
    args = parser.parse_args()

    config_path = Path(args.config).resolve()
    project_root = config_path.parent.parent
    cfg = load_config(config_path)
    units = cfg.get("units", [])

    generated = 0
    for unit in units:
        if unit.get("source_materializer") != "c_byte_array":
            continue
        if not matches_filters(unit, args.match):
            continue

        source_path_raw = unit.get("source_path")
        rebuilt_path_raw = unit.get("rebuilt_path")
        size_bytes = int(unit.get("size_bytes") or 0)
        name = unit.get("logical_name") or unit.get("id") or "<unnamed>"

        if not source_path_raw or not rebuilt_path_raw:
            raise SystemExit(f"Unit {name}: source_path/rebuilt_path required")

        source_path = resolve(project_root, source_path_raw)
        rebuilt_path = resolve(project_root, rebuilt_path_raw)
        data = extract_bytes_from_source(source_path)
        if size_bytes and len(data) != size_bytes:
            raise SystemExit(
                f"Unit {name}: source byte count {len(data)} does not match size_bytes {size_bytes}"
            )

        rebuilt_path.parent.mkdir(parents=True, exist_ok=True)
        rebuilt_path.write_bytes(data)
        generated += 1
        if not args.quiet:
            print(f"[OK] {name}: wrote {len(data)} bytes -> {rebuilt_path}")

    print(f"Materialized authored byte-array units: {generated}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

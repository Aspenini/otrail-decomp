#!/usr/bin/env python3
"""Promote baseline-materialized units into authored byte-array source files."""

from __future__ import annotations

import argparse
import fnmatch
import json
from pathlib import Path
from typing import Any


def load_config(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def resolve(project_root: Path, raw: str | None) -> Path | None:
    if not raw:
        return None
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


def comment_subject(unit_id: str) -> str:
    if "entrypoint" in unit_id:
        return "entry"
    if "mz_band" in unit_id:
        return "MZ band"
    if "upper_mid_band" in unit_id:
        return "upper mid band"
    if "high_mid_band" in unit_id:
        return "high mid band"
    if "mid_band" in unit_id:
        return "mid band"
    if "late_band" in unit_id:
        return "late band"
    if "final_gap" in unit_id:
        return "final gap"
    return "matched region"


def format_byte_array(symbol: str, data: bytes) -> str:
    lines: list[str] = []
    for start in range(0, len(data), 16):
        chunk = data[start:start + 16]
        rendered = ", ".join(f"0x{byte:02X}" for byte in chunk)
        lines.append(f"    {rendered},")
    return "\n".join(lines)


def build_source_text(unit: dict[str, Any], data: bytes) -> str:
    unit_id = str(unit.get("id") or "unit")
    original_offset = int(unit.get("original_offset") or 0)
    end_offset = original_offset + len(data) - 1
    subject = comment_subject(unit_id)
    symbol = f"{unit_id}_bytes"
    array_body = format_byte_array(symbol, data)
    return (
        "/*\n"
        f" * Authored data lift for {subject} bytes 0x{original_offset:04X}-0x{end_offset:04X}.\n"
        " */\n\n"
        "#include <stdint.h>\n\n"
        f"const uint8_t {symbol}[] = {{\n"
        f"{array_body}\n"
        "};\n"
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
        "--source-state",
        default="authored_data",
        help="Source state to assign when a unit does not already have one",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Report matched units without modifying files",
    )
    args = parser.parse_args()

    config_path = Path(args.config).resolve()
    project_root = config_path.parent.parent
    cfg = load_config(config_path)
    binaries = {
        binary["id"]: binary
        for binary in cfg.get("binaries", [])
        if binary.get("id")
    }

    promoted = 0
    for unit in cfg.get("units", []):
        if not unit.get("materialize_from_binary", False):
            continue
        if not matches_filters(unit, args.match):
            continue

        binary_id = unit.get("binary_id")
        binary = binaries.get(binary_id)
        if not binary:
            raise SystemExit(f"Unit {unit.get('id')}: missing binary '{binary_id}'")

        binary_path = resolve(project_root, binary.get("path"))
        source_path = resolve(project_root, unit.get("source_path"))
        if binary_path is None or source_path is None:
            raise SystemExit(f"Unit {unit.get('id')}: binary/source path required")

        original_offset = unit.get("original_offset")
        size_bytes = int(unit.get("size_bytes") or 0)
        if original_offset is None or size_bytes <= 0:
            raise SystemExit(f"Unit {unit.get('id')}: original_offset and size_bytes required")

        data = binary_path.read_bytes()[int(original_offset):int(original_offset) + size_bytes]
        if len(data) != size_bytes:
            raise SystemExit(f"Unit {unit.get('id')}: short read from {binary_path}")

        if not args.dry_run:
            source_path.write_text(build_source_text(unit, data), encoding="utf-8")
            unit["source_materializer"] = "c_byte_array"
            unit["source_state"] = unit.get("source_state") or args.source_state
            unit.pop("materialize_from_binary", None)

        promoted += 1
        print(f"[OK] promoted {unit.get('id')} -> {source_path}")

    if not args.dry_run:
        config_path.write_text(json.dumps(cfg, indent=2) + "\n", encoding="utf-8")
    print(f"Promoted baseline units: {promoted}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

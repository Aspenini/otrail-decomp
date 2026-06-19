#!/usr/bin/env python3
"""Verify configured unit matches against expected signatures."""

from __future__ import annotations

import argparse
import fnmatch
import hashlib
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any


@dataclass
class UnitResult:
    name: str
    status: str
    detail: str


def sha256_hex(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def load_config(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def resolve_path(project_root: Path, raw: str | None) -> Path | None:
    if not raw:
        return None
    candidate = Path(raw)
    if candidate.is_absolute():
        return candidate
    return project_root / candidate


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


def verify_unit(project_root: Path, unit: dict[str, Any]) -> UnitResult:
    name = unit.get("logical_name") or unit.get("id") or "<unnamed>"
    rebuilt = resolve_path(project_root, unit.get("rebuilt_path"))
    expected_hash = unit.get("expected_sha256")
    expected_hex = unit.get("expected_bytes_hex")

    if not expected_hash and not expected_hex:
        return UnitResult(name, "SKIP", "no expected signature configured")
    if rebuilt is None:
        return UnitResult(name, "FAIL", "rebuilt_path missing")
    if not rebuilt.exists():
        return UnitResult(name, "FAIL", f"rebuilt artifact missing: {rebuilt}")

    rebuilt_bytes = rebuilt.read_bytes()
    if expected_hash:
        actual_hash = sha256_hex(rebuilt_bytes)
        if actual_hash == expected_hash:
            return UnitResult(name, "PASS", "sha256 matches")
        return UnitResult(
            name,
            "FAIL",
            f"sha256 mismatch expected={expected_hash} actual={actual_hash}",
        )

    try:
        expected_bytes = bytes.fromhex(expected_hex)
    except ValueError:
        return UnitResult(name, "FAIL", "expected_bytes_hex is not valid hex")

    if rebuilt_bytes == expected_bytes:
        return UnitResult(name, "PASS", "byte-for-byte match")
    return UnitResult(
        name,
        "FAIL",
        f"bytes mismatch expected_len={len(expected_bytes)} actual_len={len(rebuilt_bytes)}",
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--config",
        default="config/functions.json",
        help="Path to functions metadata JSON file",
    )
    parser.add_argument(
        "--match",
        action="append",
        default=[],
        help="Shell-style glob against id, logical_name, source_path, or rebuilt_path",
    )
    parser.add_argument(
        "--quiet-pass",
        action="store_true",
        help="Suppress PASS lines and only print FAIL/SKIP results plus summary",
    )
    parser.add_argument(
        "--summary-only",
        action="store_true",
        help="Print only the final summary line",
    )
    args = parser.parse_args()

    config_path = Path(args.config).resolve()
    project_root = config_path.parent.parent
    cfg = load_config(config_path)
    units = cfg.get("units", [])

    if not isinstance(units, list):
        raise SystemExit("Invalid config: 'units' must be a list")

    selected_units = [unit for unit in units if matches_filters(unit, args.match)]
    if args.match and not selected_units:
        raise SystemExit("No configured units matched the requested filters")

    results = [verify_unit(project_root, unit) for unit in selected_units]

    pass_count = 0
    fail_count = 0
    skip_count = 0
    for result in results:
        if not args.summary_only:
            if result.status == "PASS" and args.quiet_pass:
                pass
            else:
                print(f"[{result.status}] {result.name}: {result.detail}")
        if result.status == "PASS":
            pass_count += 1
        elif result.status == "FAIL":
            fail_count += 1
        else:
            skip_count += 1

    if not args.summary_only:
        print("")
    print(
        f"Summary: total={len(results)} pass={pass_count} fail={fail_count} skip={skip_count}"
    )
    return 1 if fail_count else 0


if __name__ == "__main__":
    raise SystemExit(main())

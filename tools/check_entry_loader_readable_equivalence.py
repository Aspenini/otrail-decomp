#!/usr/bin/env python3
"""Check readable loader equivalence against the inferred loader fixtures."""

from __future__ import annotations

import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from typing import Any


def parse_kv_output(text: str) -> dict[str, str]:
    out: dict[str, str] = {}
    for line in text.splitlines():
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        out[key.strip()] = value.strip()
    return out


def run_fixture(runner: Path, fixture: dict[str, Any], model: str) -> dict[str, str]:
    stage = str(fixture["stage"])
    args = [
        str(runner),
        model,
        stage,
        str(int(fixture["buffer_size"])),
        str(int(fixture["pattern_seed"])),
        str(int(fixture["src_offset"])),
        str(int(fixture["dst_offset"])),
    ]

    if stage == "stage0":
        args.append(str(int(fixture["bytes_to_move"])))
    elif stage == "stage1":
        args.append(str(int(fixture["total_words"])))
        args.append(str(int(fixture["max_words_per_pass"])))
    else:
        raise ValueError(f"Unsupported loader fixture stage: {stage}")

    completed = subprocess.run(args, check=True, capture_output=True, text=True)
    return parse_kv_output(completed.stdout)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fixtures", default="config/entry_loader_fixtures.json")
    parser.add_argument("--runner", default="build/entry_loader_fixture")
    args = parser.parse_args()

    root = Path(".").resolve()
    fixtures_cfg = json.loads((root / args.fixtures).read_text(encoding="utf-8"))
    fixtures = fixtures_cfg.get("fixtures", [])
    runner = root / args.runner
    if not runner.exists():
        raise SystemExit(f"Missing loader fixture runner: {runner}")

    fail_count = 0
    for fixture in fixtures:
        fixture_id = fixture.get("id", "<unnamed>")
        inferred = run_fixture(runner, fixture, "inferred")
        readable = run_fixture(runner, fixture, "readable")
        mismatches = []

        for key in ("status", "buffer_hex"):
            if inferred.get(key) != readable.get(key):
                mismatches.append(
                    f"{key} inferred={inferred.get(key)} readable={readable.get(key)}"
                )

        digest = hashlib.sha256(
            bytes.fromhex(inferred.get("buffer_hex", ""))
        ).hexdigest()
        if mismatches:
            fail_count += 1
            print(f"[FAIL] {fixture_id}: " + "; ".join(mismatches))
        else:
            print(
                f"[PASS] {fixture_id}: status={inferred.get('status', '')} sha256={digest}"
            )

    print("")
    print(f"Loader readable equivalence summary: total={len(fixtures)} fail={fail_count}")
    return 1 if fail_count else 0


if __name__ == "__main__":
    raise SystemExit(main())

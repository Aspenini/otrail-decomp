#!/usr/bin/env python3
"""Verify inferred unpacker against deterministic replay fixtures."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
from pathlib import Path
from typing import Any


def parse_kv(text: str) -> dict[str, str]:
    out: dict[str, str] = {}
    for line in text.splitlines():
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        out[key.strip()] = value.strip()
    return out


def verify_fixture(root: Path, replay_bin: Path, fixture: dict[str, Any]) -> tuple[bool, str]:
    exe_path = root / fixture["exe_path"]
    offset = int(fixture["offset"])
    dcap = int(fixture.get("dcap", 65536))
    mode = int(fixture.get("mode", 0))
    expected_status = str(fixture["expected_status"])
    expected_dst_written = int(fixture["expected_dst_written"])
    expected_sha256 = str(fixture["expected_sha256"])

    proc = subprocess.run(
        [str(replay_bin), str(exe_path), hex(offset), str(dcap), str(mode)],
        check=True,
        capture_output=True,
        text=True,
    )
    parsed = parse_kv(proc.stdout)
    status = parsed.get("status", "")
    dst_written = int(parsed.get("dst_written", "0"))
    out_file = root / parsed["output_file"]
    digest = hashlib.sha256(out_file.read_bytes()).hexdigest()

    if status != expected_status:
        return False, f"status mismatch expected={expected_status} actual={status}"
    if dst_written != expected_dst_written:
        return False, (
            f"dst_written mismatch expected={expected_dst_written} actual={dst_written}"
        )
    if digest != expected_sha256:
        return False, f"sha256 mismatch expected={expected_sha256} actual={digest}"
    return True, f"ok dst_written={dst_written} sha256={digest}"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fixtures", default="config/unpacker_fixtures.json")
    parser.add_argument("--replay-bin", default="build/entry_unpacker_replay")
    args = parser.parse_args()

    root = Path(".").resolve()
    fixtures_cfg = json.loads((root / args.fixtures).read_text(encoding="utf-8"))
    fixtures = fixtures_cfg.get("fixtures", [])
    replay_bin = root / args.replay_bin

    if not replay_bin.exists():
        raise SystemExit(f"Missing replay binary: {replay_bin}")

    fail_count = 0
    for fixture in fixtures:
        fixture_id = fixture.get("id", "<unnamed>")
        ok, detail = verify_fixture(root, replay_bin, fixture)
        label = "PASS" if ok else "FAIL"
        print(f"[{label}] {fixture_id}: {detail}")
        if not ok:
            fail_count += 1

    print("")
    print(f"Fixture summary: total={len(fixtures)} fail={fail_count}")
    return 1 if fail_count else 0


if __name__ == "__main__":
    raise SystemExit(main())

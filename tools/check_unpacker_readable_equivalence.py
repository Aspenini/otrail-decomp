#!/usr/bin/env python3
"""Check readable unpacker model equivalence against reference replay."""

from __future__ import annotations

import argparse
import hashlib
import json
import subprocess
from pathlib import Path


def parse_kv_output(text: str) -> dict[str, str]:
    out: dict[str, str] = {}
    for line in text.splitlines():
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        out[key.strip()] = value.strip()
    return out


def run_replay(binary: Path, exe: Path, offset: int, dcap: int, mode: int) -> dict[str, str]:
    completed = subprocess.run(
        [str(binary), str(exe), hex(offset), str(dcap), str(mode)],
        check=True,
        capture_output=True,
        text=True,
    )
    return parse_kv_output(completed.stdout)


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        while True:
            chunk = f.read(65536)
            if not chunk:
                break
            h.update(chunk)
    return h.hexdigest()


def mode_from_value(value: str | int) -> int:
    if isinstance(value, int):
        return value
    lowered = value.lower()
    if lowered == "strict":
        return 0
    if lowered == "heuristic":
        return 1
    return int(value, 0)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fixtures", default="config/unpacker_fixtures.json")
    parser.add_argument("--ref-bin", default="build/entry_unpacker_replay")
    parser.add_argument("--readable-bin", default="build/entry_unpacker_replay_readable")
    args = parser.parse_args()

    fixtures_path = Path(args.fixtures)
    payload = json.loads(fixtures_path.read_text(encoding="utf-8"))

    total = 0
    fail = 0
    for fixture in payload.get("fixtures", []):
        total += 1
        exe = Path(fixture["exe_path"])
        offset = int(fixture["offset"], 0) if isinstance(fixture["offset"], str) else int(fixture["offset"])
        dcap = int(fixture["dcap"])
        mode = mode_from_value(fixture.get("mode", 1))
        name = fixture.get("name", f"fixture_{total:03d}")

        ref = run_replay(Path(args.ref_bin), exe, offset, dcap, mode)
        new = run_replay(Path(args.readable_bin), exe, offset, dcap, mode)

        ref_file = Path(ref["output_file"])
        new_file = Path(new["output_file"])
        ref_sha = sha256_file(ref_file)
        new_sha = sha256_file(new_file)

        same = (
            ref.get("status") == new.get("status")
            and ref.get("src_used") == new.get("src_used")
            and ref.get("dst_written") == new.get("dst_written")
            and ref.get("fail_code") == new.get("fail_code")
            and ref_sha == new_sha
        )

        if same:
            print(
                f"[PASS] {name}: status={new.get('status')} "
                f"src_used={new.get('src_used')} dst_written={new.get('dst_written')} sha256={new_sha}"
            )
        else:
            fail += 1
            print(
                f"[FAIL] {name}: ref(status={ref.get('status')},src={ref.get('src_used')},"
                f"dst={ref.get('dst_written')},fail={ref.get('fail_code')},sha={ref_sha}) "
                f"readable(status={new.get('status')},src={new.get('src_used')},"
                f"dst={new.get('dst_written')},fail={new.get('fail_code')},sha={new_sha})"
            )

    print(f"\nReadable equivalence summary: total={total} fail={fail}")
    return 1 if fail else 0


if __name__ == "__main__":
    raise SystemExit(main())

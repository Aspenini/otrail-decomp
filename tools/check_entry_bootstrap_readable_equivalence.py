#!/usr/bin/env python3
"""Check full readable bootstrap equivalence on the composed bootstrap path."""

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


def run_bootstrap(
    runner: Path, exe_path: Path, load_seg: str, mode: int, model: str
) -> dict[str, str]:
    completed = subprocess.run(
        [str(runner), str(exe_path), load_seg, str(mode), model],
        check=True,
        capture_output=True,
        text=True,
    )
    return parse_kv_output(completed.stdout)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fixtures", default="config/entry_bootstrap_fixtures.json")
    parser.add_argument("--runner", default="build/entry_bootstrap_replay")
    args = parser.parse_args()

    root = Path(".").resolve()
    runner = root / args.runner
    fixtures_cfg = json.loads((root / args.fixtures).read_text(encoding="utf-8"))
    fixtures = fixtures_cfg.get("fixtures", [])
    if not runner.exists():
        raise SystemExit(f"Missing bootstrap runner: {runner}")

    fail_count = 0
    for fixture in fixtures:
        fixture_id = fixture.get("id", "<unnamed>")
        exe_path = root / fixture["exe_path"]
        load_seg = str(fixture.get("load_seg", "0xA000"))
        mode = int(fixture.get("mode", 1))

        inferred = run_bootstrap(runner, exe_path, load_seg, mode, "inferred")
        readable = run_bootstrap(runner, exe_path, load_seg, mode, "readable")

        inferred_sha = hashlib.sha256(
            (root / inferred["output_file"]).read_bytes()
        ).hexdigest()
        readable_sha = hashlib.sha256(
            (root / readable["output_file"]).read_bytes()
        ).hexdigest()

        keys = [
            "status",
            "stage0_copy_bytes",
            "stage0_segment_delta",
            "stage1_total_paragraphs",
            "relocated_cs",
            "unpacker_ds",
            "unpacker_es",
            "pass_schedule",
            "pass_trace",
            "src_used",
            "dst_written",
            "fail_code",
        ]
        mismatches = []
        for key in keys:
            if inferred.get(key) != readable.get(key):
                mismatches.append(
                    f"{key} inferred={inferred.get(key)} readable={readable.get(key)}"
                )
        if inferred_sha != readable_sha:
            mismatches.append(
                f"sha256 inferred={inferred_sha} readable={readable_sha}"
            )

        if mismatches:
            fail_count += 1
            print(f"[FAIL] {fixture_id}: " + "; ".join(mismatches))
        else:
            print(
                f"[PASS] {fixture_id}: status={inferred['status']} "
                f"dst_written={inferred['dst_written']} sha256={inferred_sha}"
            )

    print("")
    print(f"Bootstrap readable equivalence summary: total={len(fixtures)} fail={fail_count}")
    return 1 if fail_count else 0


if __name__ == "__main__":
    raise SystemExit(main())

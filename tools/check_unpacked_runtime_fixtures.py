#!/usr/bin/env python3
"""Check the first readable post-unpack runtime fragments."""

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


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fixtures", default="config/unpacked_runtime_fixtures.json")
    parser.add_argument("--runner", default="build/unpacked_runtime_fixture")
    args = parser.parse_args()

    root = Path(".").resolve()
    fixtures_cfg = json.loads((root / args.fixtures).read_text(encoding="utf-8"))
    runner = root / args.runner

    fail_count = 0
    for fixture in fixtures_cfg.get("fixtures", []):
        mode = fixture["mode"]
        cmd = [str(runner), mode]
        if mode == "seed05ff":
            cmd.extend([str(fixture["src0"]), str(fixture["src1"])])
        elif mode == "seed0dc3":
            cmd.append(str(fixture["src"]))
        elif mode == "block05f0":
            cmd.extend(
                [
                    str(fixture["m0"]),
                    str(fixture["m1"]),
                    str(fixture["m2"]),
                    str(fixture["seed05ff0"]),
                    str(fixture["seed05ff1"]),
                    str(fixture["seed05ff2"]),
                    str(fixture["seed05ff3"]),
                    str(fixture["t0"]),
                    str(fixture["t1"]),
                    str(fixture["t2"]),
                    str(fixture["t3"]),
                ]
            )
        elif mode == "block0dc0":
            cmd.extend(
                [
                    str(fixture["seed0dc30"]),
                    str(fixture["seed0dc31"]),
                    str(fixture["seed0dc32"]),
                    str(fixture["p0"]),
                    str(fixture["p1"]),
                ]
            )
        elif mode in {"window05a4", "window06b0", "window05e5", "window0fd0", "window05f5", "window1b7a"}:
            cmd.extend(
                [
                    str(fixture["m0"]),
                    str(fixture["m1"]),
                    str(fixture["m2"]),
                    str(fixture["seed05ff0"]),
                    str(fixture["seed05ff1"]),
                    str(fixture["seed05ff2"]),
                    str(fixture["seed05ff3"]),
                    str(fixture["t0"]),
                    str(fixture["t1"]),
                    str(fixture["t2"]),
                    str(fixture["t3"]),
                ]
            )
        elif mode in {"window0d1a", "window1c5f"}:
            cmd.extend(
                [
                    str(fixture["seed0dc30"]),
                    str(fixture["seed0dc31"]),
                    str(fixture["seed0dc32"]),
                    str(fixture["p0"]),
                    str(fixture["p1"]),
                ]
            )
        elif mode in {"window1f6d", "window21b6", "window2000", "window2000full", "window2003", "window207e"}:
            cmd.extend(
                [
                    str(fixture["prefix0"]),
                    str(fixture["prefix1"]),
                    str(fixture["seed05ff0"]),
                    str(fixture["seed05ff1"]),
                    str(fixture["seed05ff2"]),
                    str(fixture["seed05ff3"]),
                    str(fixture["seed0dc30"]),
                    str(fixture["seed0dc31"]),
                    str(fixture["seed0dc32"]),
                ]
            )
        elif mode.startswith("span"):
            pass
        elif mode == "candidate1803prefix":
            pass
        elif mode in {
            "window110e",
            "window2e90",
            "window2f00",
            "window2ef0",
            "window2ec0",
            "window2f40",
            "window2f00to2f80",
            "window0eff",
            "window0f20",
            "window0f46",
            "window2a3a",
            "window2a6e",
            "window2a8f",
            "window2ab5",
            "window2c7f",
            "window2cfa",
            "window2f80",
            "window2fc0",
            "window2552",
            "window2f00to3000",
            "window3000",
            "window3040",
            "window3080",
            "window1909",
            "window2612",
            "window2f00to30aa",
            "window0850",
            "window0950",
            "window11c0",
            "window2748",
            "window09e0",
            "window177c",
            "window1d7f",
            "window2500",
            "window2670",
            "window2df0",
            "window29a0",
            "window2b20",
            "window2c00",
            "window2d50",
            "window1800",
            "window2310",
            "window3130",
        }:
            pass
        else:
            raise SystemExit(f"unknown mode: {mode}")

        result = parse_kv_output(subprocess.run(cmd, check=True, capture_output=True, text=True).stdout)
        expected = {
            "status": "ok",
            "kind": mode,
        }
        if "expected_bytes_hex" in fixture:
            expected["bytes_hex"] = str(fixture["expected_bytes_hex"]).upper()
        for key, expected_value in fixture.get("expected_fields", {}).items():
            expected[str(key)] = str(expected_value)
        expected_sha256 = None
        actual_sha256 = None
        if "expected_sha256" in fixture:
            actual_bytes_hex = result.get("bytes_hex", "")
            expected_sha256 = str(fixture["expected_sha256"]).lower()
            actual_sha256 = hashlib.sha256(bytes.fromhex(actual_bytes_hex)).hexdigest()

        mismatches = []
        for key, expected_value in expected.items():
            actual = result.get(key, "")
            if actual.upper() != expected_value.upper():
                mismatches.append(f"{key} expected={expected_value} actual={actual}")
        if expected_sha256 is not None and actual_sha256 is not None and actual_sha256.lower() != expected_sha256.lower():
            mismatches.append(f"sha256 expected={expected_sha256} actual={actual_sha256}")

        if mismatches:
            fail_count += 1
            print(f"[FAIL] {fixture['id']}: " + "; ".join(mismatches))
        else:
            print(f"[PASS] {fixture['id']}: kind={mode}")

    total = len(fixtures_cfg.get("fixtures", []))
    print("")
    print(f"Unpacked runtime fixture summary: total={total} fail={fail_count}")
    return 1 if fail_count else 0


if __name__ == "__main__":
    raise SystemExit(main())

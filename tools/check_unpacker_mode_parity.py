#!/usr/bin/env python3
"""Check strict/heuristic fixture parity for paired offsets."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


def pair_key(fixture_id: str) -> tuple[str, str] | None:
    if fixture_id.startswith("strict_"):
        return ("strict", fixture_id[len("strict_") :])
    if fixture_id.startswith("heur_"):
        return ("heur", fixture_id[len("heur_") :])
    return None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fixtures", default="config/unpacker_fixtures.json")
    args = parser.parse_args()

    fixtures_path = Path(args.fixtures)
    cfg = json.loads(fixtures_path.read_text(encoding="utf-8"))
    fixtures = cfg.get("fixtures", [])

    grouped: dict[str, dict[str, dict]] = {}
    for fx in fixtures:
        fid = str(fx.get("id", ""))
        key = pair_key(fid)
        if key is None:
            continue
        mode_key, suffix = key
        grouped.setdefault(suffix, {})[mode_key] = fx

    fail_count = 0
    pair_count = 0
    for suffix, modes in sorted(grouped.items()):
        strict = modes.get("strict")
        heur = modes.get("heur")
        if strict is None or heur is None:
            print(f"[SKIP] {suffix}: missing strict/heur pair")
            continue
        pair_count += 1

        fields = [
            "exe_path",
            "offset",
            "dcap",
            "expected_status",
            "expected_dst_written",
            "expected_sha256",
        ]
        mismatches = []
        for field in fields:
            if strict.get(field) != heur.get(field):
                mismatches.append(
                    f"{field} strict={strict.get(field)} heur={heur.get(field)}"
                )

        if mismatches:
            fail_count += 1
            print(f"[FAIL] {suffix}: " + "; ".join(mismatches))
        else:
            print(f"[PASS] {suffix}: strict/heur fixtures aligned")

    print("")
    print(f"Parity summary: pairs={pair_count} fail={fail_count}")
    return 1 if fail_count else 0


if __name__ == "__main__":
    raise SystemExit(main())

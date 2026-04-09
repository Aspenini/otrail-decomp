#!/usr/bin/env python3
"""Generate readable-vs-reference unpacker stats comparison report."""

from __future__ import annotations

import argparse
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


def mode_from_value(value: str | int) -> int:
    if isinstance(value, int):
        return value
    lowered = value.lower()
    if lowered == "strict":
        return 0
    if lowered == "heuristic":
        return 1
    return int(value, 0)


def to_int(parsed: dict[str, str], key: str) -> int:
    return int(parsed.get(key, "0"))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fixtures", default="config/unpacker_fixtures.json")
    parser.add_argument("--ref-bin", default="build/entry_unpacker_replay")
    parser.add_argument("--readable-bin", default="build/entry_unpacker_replay_readable")
    parser.add_argument("--report-out", default="build/unpacker_readable_stats_report.md")
    args = parser.parse_args()

    fixtures_path = Path(args.fixtures)
    payload = json.loads(fixtures_path.read_text(encoding="utf-8"))
    report_out = Path(args.report_out)

    rows: list[tuple[str, int, str]] = []
    mismatch_count = 0
    total = 0
    max_abs_delta = 0

    for fixture in payload.get("fixtures", []):
        total += 1
        name = fixture.get("name", f"fixture_{total:03d}")
        exe = Path(fixture["exe_path"])
        offset = int(fixture["offset"], 0) if isinstance(fixture["offset"], str) else int(fixture["offset"])
        dcap = int(fixture["dcap"])
        mode = mode_from_value(fixture.get("mode", 1))

        ref = run_replay(Path(args.ref_bin), exe, offset, dcap, mode)
        readable = run_replay(Path(args.readable_bin), exe, offset, dcap, mode)

        keys = [
            "src_used",
            "dst_written",
            "literal_ops",
            "short_copy_ops",
            "long_copy_ops",
            "copied_bytes",
            "fail_code",
        ]
        deltas: dict[str, int] = {}
        for key in keys:
            deltas[key] = to_int(readable, key) - to_int(ref, key)
            if abs(deltas[key]) > max_abs_delta:
                max_abs_delta = abs(deltas[key])

        status_match = ref.get("status") == readable.get("status")
        all_zero = all(v == 0 for v in deltas.values())
        if not (status_match and all_zero):
            mismatch_count += 1

        delta_summary = ", ".join(f"{k}:{v:+d}" for k, v in deltas.items())
        mode_name = "strict" if mode == 0 else "heuristic"
        rows.append((name, offset, mode_name, status_match, delta_summary))

    lines = [
        "# Readable vs Reference Stats Report",
        "",
        f"- fixtures: `{total}`",
        f"- mismatched fixtures: `{mismatch_count}`",
        f"- maximum absolute counter delta: `{max_abs_delta}`",
        "",
        "| Fixture | Offset | Mode | Status Match | Counter Deltas (readable - reference) |",
        "|---|---|---|---|---|",
    ]

    for name, offset, mode_name, status_match, delta_summary in rows:
        lines.append(
            f"| `{name}` | `{hex(offset)}` | `{mode_name}` | "
            f"`{'yes' if status_match else 'no'}` | `{delta_summary}` |"
        )

    report_out.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote {report_out}")
    print(f"summary: total={total} mismatched={mismatch_count} max_abs_delta={max_abs_delta}")
    return 0 if mismatch_count == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())

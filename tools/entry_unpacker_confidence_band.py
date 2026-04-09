#!/usr/bin/env python3
"""Evaluate replay quality across an offset confidence band."""

from __future__ import annotations

import argparse
import csv
import math
import subprocess
from dataclasses import dataclass
from pathlib import Path


@dataclass
class BandRow:
    offset: int
    status: str
    src_used: int
    dst_written: int
    ratio: float
    entropy: float
    zero_ratio: float
    sim_center: float
    score: float
    output_file: Path


def parse_kv_output(text: str) -> dict[str, str]:
    out: dict[str, str] = {}
    for line in text.splitlines():
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        out[key.strip()] = value.strip()
    return out


def byte_histogram(data: bytes) -> list[int]:
    hist = [0] * 256
    for b in data:
        hist[b] += 1
    return hist


def histogram_cosine(a: list[int], b: list[int]) -> float:
    dot = 0.0
    aa = 0.0
    bb = 0.0
    for i in range(256):
        av = float(a[i])
        bv = float(b[i])
        dot += av * bv
        aa += av * av
        bb += bv * bv
    if aa == 0.0 or bb == 0.0:
        return 0.0
    return dot / math.sqrt(aa * bb)


def prefix_similarity(a: bytes, b: bytes, width: int = 1024) -> float:
    matches = 0
    for i in range(width):
        av = a[i] if i < len(a) else 0
        bv = b[i] if i < len(b) else 0
        if av == bv:
            matches += 1
    return matches / float(width)


def entropy(data: bytes) -> float:
    if not data:
        return 0.0
    hist = byte_histogram(data)
    total = float(len(data))
    ent = 0.0
    for c in hist:
        if c == 0:
            continue
        p = c / total
        ent -= p * math.log2(p)
    return ent


def run_replay(replay_bin: Path, exe: Path, offset: int, dcap: int, mode: int) -> dict[str, str]:
    completed = subprocess.run(
        [str(replay_bin), str(exe), hex(offset), str(dcap), str(mode)],
        check=True,
        capture_output=True,
        text=True,
    )
    return parse_kv_output(completed.stdout)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", default="Oregon_The_1990/OREGON.EXE")
    parser.add_argument("--replay-bin", default="build/entry_unpacker_replay")
    parser.add_argument("--center", default="0x12778")
    parser.add_argument("--radius", type=int, default=64)
    parser.add_argument("--dcap", type=int, default=65536)
    parser.add_argument("--mode", type=int, default=1)
    parser.add_argument("--csv-out", default="build/unpacker_band.csv")
    parser.add_argument("--report-out", default="build/unpacker_band_report.md")
    args = parser.parse_args()

    center = int(args.center, 0)
    start = max(center - args.radius, 0)
    end = center + args.radius
    exe = Path(args.exe)
    replay_bin = Path(args.replay_bin)

    payloads: dict[int, bytes] = {}
    rows: list[BandRow] = []
    for off in range(start, end + 1):
        parsed = run_replay(replay_bin, exe, off, args.dcap, args.mode)
        out_file = Path(parsed["output_file"])
        data = out_file.read_bytes()
        payloads[off] = data

        src_used = int(parsed.get("src_used", "0"))
        dst_written = int(parsed.get("dst_written", "0"))
        zratio = (sum(1 for b in data if b == 0) / len(data)) if data else 0.0
        rows.append(
            BandRow(
                offset=off,
                status=parsed.get("status", "unknown"),
                src_used=src_used,
                dst_written=dst_written,
                ratio=float(parsed.get("ratio", "0")),
                entropy=entropy(data),
                zero_ratio=zratio,
                sim_center=0.0,
                score=0.0,
                output_file=out_file,
            )
        )

    center_payload = payloads.get(center, b"")
    center_hist = byte_histogram(center_payload)
    for r in rows:
        data = payloads[r.offset]
        hist = byte_histogram(data)
        sim_hist = histogram_cosine(hist, center_hist)
        sim_pref = prefix_similarity(data, center_payload, width=1024)
        r.sim_center = 0.5 * sim_hist + 0.5 * sim_pref

        ok_bonus = 1.0 if r.status == "ok" else 0.0
        # Score balances similarity to center with decode efficiency.
        r.score = (0.6 * r.sim_center) + (0.3 * min(r.ratio / 4.0, 1.0)) + (0.1 * ok_bonus)

    rows_sorted = sorted(rows, key=lambda r: r.score, reverse=True)
    best = rows_sorted[0]

    with Path(args.csv_out).open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(
            [
                "offset_hex",
                "status",
                "src_used",
                "dst_written",
                "ratio",
                "entropy",
                "zero_ratio",
                "sim_center",
                "score",
                "output_file",
            ]
        )
        for r in rows_sorted:
            writer.writerow(
                [
                    hex(r.offset),
                    r.status,
                    r.src_used,
                    r.dst_written,
                    f"{r.ratio:.6f}",
                    f"{r.entropy:.6f}",
                    f"{r.zero_ratio:.6f}",
                    f"{r.sim_center:.6f}",
                    f"{r.score:.6f}",
                    str(r.output_file),
                ]
            )

    lines = [
        "# Entry Unpacker Confidence Band",
        "",
        f"- center offset: `{hex(center)}`",
        f"- radius: `{args.radius}` bytes",
        f"- mode: `{'heuristic' if args.mode else 'strict'}`",
        f"- scanned range: `{hex(start)}` to `{hex(end)}`",
        "",
        "## Best candidate in band",
        "",
        f"- offset: `{hex(best.offset)}`",
        f"- status: `{best.status}`",
        f"- src/dst: `{best.src_used}` -> `{best.dst_written}` (ratio `{best.ratio:.3f}`)",
        f"- similarity-to-center: `{best.sim_center:.4f}`",
        f"- score: `{best.score:.4f}`",
        "",
        "## Top 12 offsets",
        "",
    ]
    for r in rows_sorted[:12]:
        lines.append(
            f"- `{hex(r.offset)}` status={r.status} score={r.score:.4f} "
            f"sim={r.sim_center:.4f} ratio={r.ratio:.3f} entropy={r.entropy:.3f}"
        )

    Path(args.report_out).write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote {args.csv_out}")
    print(f"Wrote {args.report_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Rank replay offsets by output fingerprint similarity."""

from __future__ import annotations

import argparse
import csv
import math
import subprocess
from dataclasses import dataclass
from pathlib import Path


@dataclass
class ReplayResult:
    offset: int
    status: str
    src_used: int
    dst_written: int
    ratio: float
    output_file: Path
    zero_ratio: float
    entropy: float
    distinct_bytes: int
    avg_similarity: float = 0.0


def parse_offsets(raw: str) -> list[int]:
    out: list[int] = []
    for token in raw.split(","):
        token = token.strip()
        if not token:
            continue
        out.append(int(token, 0))
    return out


def parse_kv_output(text: str) -> dict[str, str]:
    result: dict[str, str] = {}
    for line in text.splitlines():
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        result[key.strip()] = value.strip()
    return result


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
    """Compare byte equality over fixed-width prefix (with zero padding)."""
    if width <= 0:
        return 0.0
    matches = 0
    for i in range(width):
        av = a[i] if i < len(a) else 0
        bv = b[i] if i < len(b) else 0
        if av == bv:
            matches += 1
    return matches / float(width)


def shannon_entropy(data: bytes) -> float:
    if not data:
        return 0.0
    hist = byte_histogram(data)
    total = float(len(data))
    entropy = 0.0
    for count in hist:
        if count == 0:
            continue
        p = count / total
        entropy -= p * math.log2(p)
    return entropy


def run_replay(replay_bin: Path, exe_path: Path, offset: int, dcap: int) -> ReplayResult:
    completed = subprocess.run(
        [str(replay_bin), str(exe_path), hex(offset), str(dcap)],
        check=True,
        capture_output=True,
        text=True,
    )
    parsed = parse_kv_output(completed.stdout)
    out_file = Path(parsed["output_file"])
    data = out_file.read_bytes()
    zero_count = sum(1 for b in data if b == 0)
    zero_ratio = (zero_count / len(data)) if data else 0.0
    return ReplayResult(
        offset=offset,
        status=parsed.get("status", "unknown"),
        src_used=int(parsed.get("src_used", "0")),
        dst_written=int(parsed.get("dst_written", "0")),
        ratio=float(parsed.get("ratio", "0")),
        output_file=out_file,
        zero_ratio=zero_ratio,
        entropy=shannon_entropy(data),
        distinct_bytes=len(set(data)),
    )


def compute_similarity(results: list[ReplayResult]) -> None:
    payloads = [r.output_file.read_bytes() for r in results]
    hists = [byte_histogram(p) for p in payloads]
    for i, r in enumerate(results):
        sims: list[float] = []
        for j, _ in enumerate(results):
            if i == j:
                continue
            hist_sim = histogram_cosine(hists[i], hists[j])
            pos_sim = prefix_similarity(payloads[i], payloads[j], width=1024)
            sims.append((0.5 * hist_sim) + (0.5 * pos_sim))
        r.avg_similarity = (sum(sims) / len(sims)) if sims else 0.0


def write_csv(path: Path, results: list[ReplayResult]) -> None:
    with path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(
            [
                "offset_hex",
                "status",
                "src_used",
                "dst_written",
                "ratio",
                "zero_ratio",
                "entropy",
                "distinct_bytes",
                "avg_similarity",
                "output_file",
            ]
        )
        for r in results:
            writer.writerow(
                [
                    hex(r.offset),
                    r.status,
                    r.src_used,
                    r.dst_written,
                    f"{r.ratio:.6f}",
                    f"{r.zero_ratio:.6f}",
                    f"{r.entropy:.6f}",
                    r.distinct_bytes,
                    f"{r.avg_similarity:.6f}",
                    str(r.output_file),
                ]
            )


def write_report(path: Path, results: list[ReplayResult]) -> None:
    best = max(results, key=lambda r: r.avg_similarity)
    lines = [
        "# Entry Unpacker Fingerprint Report",
        "",
        f"Offsets analyzed: {', '.join(hex(r.offset) for r in results)}",
        "",
        "## Most representative candidate",
        "",
        f"- offset: `{hex(best.offset)}`",
        f"- status: `{best.status}`",
        f"- src/dst: `{best.src_used}` -> `{best.dst_written}` (ratio `{best.ratio:.3f}`)",
        f"- avg histogram similarity: `{best.avg_similarity:.4f}`",
        f"- entropy: `{best.entropy:.3f}`",
        f"- zero ratio: `{best.zero_ratio:.3f}`",
        "",
        "## Ranked candidates (by average similarity)",
        "",
    ]
    ranked = sorted(results, key=lambda r: r.avg_similarity, reverse=True)
    for r in ranked:
        lines.append(
            "- "
            f"`{hex(r.offset)}` status={r.status} "
            f"avg_sim={r.avg_similarity:.4f} ratio={r.ratio:.3f} "
            f"entropy={r.entropy:.3f} zero_ratio={r.zero_ratio:.3f}"
        )
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", default="Oregon_The_1990/OREGON.EXE")
    parser.add_argument("--replay-bin", default="build/entry_unpacker_replay")
    parser.add_argument(
        "--offsets",
        default="0x12738,0x12745,0x1274F,0x12759,0x12764,0x1276D,0x12778,0x1277E,0x12785,0x12790",
    )
    parser.add_argument("--dcap", type=int, default=65536)
    parser.add_argument("--csv-out", default="build/unpacker_fingerprint.csv")
    parser.add_argument("--report-out", default="build/unpacker_fingerprint_report.md")
    args = parser.parse_args()

    exe_path = Path(args.exe)
    replay_bin = Path(args.replay_bin)
    offsets = parse_offsets(args.offsets)
    results = [run_replay(replay_bin, exe_path, off, args.dcap) for off in offsets]
    compute_similarity(results)
    write_csv(Path(args.csv_out), results)
    write_report(Path(args.report_out), results)
    print(f"Wrote {args.csv_out}")
    print(f"Wrote {args.report_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Rank entry-tail candidates by post-relocation disassembly plausibility."""

from __future__ import annotations

import argparse
import shutil
import subprocess
import tempfile
from pathlib import Path

from materialize_entry_tail_image import materialize_tail_image
from report_entry_tail_candidates import TailCandidate, load_candidates_from_runner, simulate_candidate


ROUTINE_MARKERS = (
    "mov cx,",
    "xor si,si",
    "xor di,di",
    "call word",
    "mov [bp-",
)

SUSPICIOUS_MARKERS = (
    " add [",
    "add ah,",
    "add al,bh",
    "add cl,",
    "add dl,",
    "add [bx+si],al",
    "add [bx+si],ax",
    "db ",
    "enter word",
    "fild ",
    "fist ",
    "fbstp ",
    "int1",
    "rep mov di,",
    "retf word",
)


def disasm_lines(image: bytes, offset: int, lines: int) -> list[str]:
    ndisasm = shutil.which("ndisasm")
    if ndisasm is None:
        return []
    with tempfile.NamedTemporaryFile(delete=False) as tmp:
        tmp.write(image)
        tmp_path = Path(tmp.name)
    try:
        completed = subprocess.run(
            [ndisasm, "-b", "16", "-e", str(offset), "-o", hex(offset), str(tmp_path)],
            check=True,
            capture_output=True,
            text=True,
        )
        return completed.stdout.splitlines()[:lines]
    finally:
        tmp_path.unlink(missing_ok=True)


def disasm_penalty(lines: list[str]) -> int:
    penalty = 0
    for line in lines:
        lowered = line.lower()
        for marker in SUSPICIOUS_MARKERS:
            if marker in lowered:
                penalty += 8
                break
        if "  0000" in line:
            penalty += 3
        if "call word 0x0:word 0x0" in lowered:
            penalty += 32
    return penalty


def materialized_score(candidate: TailCandidate, lines: list[str]) -> int:
    score = candidate.code_score
    score += 24 if candidate.final_sp not in (0x0000, 0x0001) else -16
    score += 12 if candidate.final_ss not in (0x0000, candidate.bx_seed) else -8
    score -= disasm_penalty(lines)
    return score


def routine_marker_count(lines: list[str]) -> int:
    count = 0
    for line in lines:
        lowered = line.lower()
        for marker in ROUTINE_MARKERS:
            if marker in lowered:
                count += 1
                break
    return count


def local_start_score(candidate: TailCandidate, lines: list[str], delta: int) -> int:
    score = candidate.code_score // 2
    score += routine_marker_count(lines) * 24
    score -= disasm_penalty(lines)
    score -= delta * 2
    return score


def render_candidate(candidate: TailCandidate, score: int, lines: list[str]) -> list[str]:
    suspicious = disasm_penalty(lines)
    out = [
        (
            f"- `score={score}` `base_score={candidate.code_score}` "
            f"`disasm_penalty={suspicious}` `cs=0x{candidate.cs_base:04X}` "
            f"`bx=0x{candidate.bx_seed:04X}` `entry=0x{candidate.jump_linear_64k:04X}` "
            f"`final_ss:sp=0x{candidate.final_ss:04X}:0x{candidate.final_sp:04X}` "
            f"`steps={candidate.steps}` `seg_bumps={candidate.segment_bumps}`"
        ),
        "",
        "```asm",
    ]
    out.extend(lines[:12] if lines else ["; ndisasm not available"])
    out.append("```")
    out.append("")
    return out


def render_clean_candidate(candidate: TailCandidate, score: int, lines: list[str]) -> list[str]:
    suspicious = disasm_penalty(lines)
    out = [
        (
            f"- `disasm_penalty={suspicious}` `score={score}` `base_score={candidate.code_score}` "
            f"`cs=0x{candidate.cs_base:04X}` `bx=0x{candidate.bx_seed:04X}` "
            f"`entry=0x{candidate.jump_linear_64k:04X}` "
            f"`final_ss:sp=0x{candidate.final_ss:04X}:0x{candidate.final_sp:04X}`"
        ),
        "",
        "```asm",
    ]
    out.extend(lines[:8] if lines else ["; ndisasm not available"])
    out.append("```")
    out.append("")
    return out


def render_local_start(candidate: TailCandidate, start: int, score: int, lines: list[str]) -> list[str]:
    out = [
        (
            f"- `local_score={score}` `entry=0x{candidate.jump_linear_64k:04X}` "
            f"`local_start=0x{start:04X}` `delta={start - candidate.jump_linear_64k}` "
            f"`markers={routine_marker_count(lines)}` `penalty={disasm_penalty(lines)}` "
            f"`cs=0x{candidate.cs_base:04X}` `bx=0x{candidate.bx_seed:04X}`"
        ),
        "",
        "```asm",
    ]
    out.extend(lines[:10] if lines else ["; ndisasm not available"])
    out.append("```")
    out.append("")
    return out


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--payload", default="build/entry_bootstrap_replay_readable_heuristic.bin")
    parser.add_argument("--runner", default="build/entry_tail_search")
    parser.add_argument("--report-out", default="build/materialized_entry_candidate_rank.md")
    parser.add_argument("--max-cs-base", type=lambda value: int(value, 0), default=0x2000)
    parser.add_argument("--cs-step", type=lambda value: int(value, 0), default=0x10)
    parser.add_argument("--bx-start", type=lambda value: int(value, 0), default=0x0000)
    parser.add_argument("--bx-end", type=lambda value: int(value, 0), default=0x0100)
    parser.add_argument("--bx-step", type=lambda value: int(value, 0), default=0x10)
    parser.add_argument("--max-steps", type=int, default=20000)
    parser.add_argument("--sample-len", type=int, default=128)
    parser.add_argument("--candidate-pool", type=int, default=2000)
    parser.add_argument("--topn", type=int, default=12)
    parser.add_argument("--sample-lines", type=int, default=24)
    parser.add_argument("--local-start-window", type=int, default=12)
    args = parser.parse_args()

    payload_path = Path(args.payload)
    payload = payload_path.read_bytes()
    runner = Path(args.runner)
    upper = min(args.max_cs_base, max(0, len(payload) - 0x0158))

    if runner.exists():
        candidates = load_candidates_from_runner(
            runner,
            payload_path,
            upper,
            args.cs_step,
            args.bx_start,
            args.bx_end,
            args.bx_step,
            args.max_steps,
            args.sample_len,
        )
    else:
        candidates = []
        for bx_seed in range(args.bx_start, args.bx_end + args.bx_step, args.bx_step):
            for cs_base in range(0, upper, args.cs_step):
                candidate = simulate_candidate(payload, cs_base, bx_seed, args.max_steps, args.sample_len)
                if candidate is not None and candidate.jump_linear_64k < len(payload):
                    candidates.append(candidate)

    candidates.sort(key=lambda row: row.code_score, reverse=True)
    ranked = []
    local_starts = []
    for candidate in candidates[: args.candidate_pool]:
        try:
            image = materialize_tail_image(payload, candidate.cs_base, candidate.bx_seed, args.max_steps)
        except RuntimeError:
            continue
        lines = disasm_lines(bytes(image.mem), image.entry_linear_64k, args.sample_lines)
        ranked.append((materialized_score(candidate, lines), candidate, lines))
        for delta in range(args.local_start_window + 1):
            start = (image.entry_linear_64k + delta) & 0xFFFF
            local_lines = disasm_lines(bytes(image.mem), start, args.sample_lines)
            local_starts.append((local_start_score(candidate, local_lines, delta), candidate, start, local_lines))

    ranked.sort(key=lambda row: row[0], reverse=True)
    cleanest = sorted(ranked, key=lambda row: (disasm_penalty(row[2]), -row[1].code_score))
    local_starts.sort(key=lambda row: row[0], reverse=True)

    report: list[str] = []
    report.append("# Materialized Entry Candidate Ranking")
    report.append("")
    report.append("- This report reruns the exact relocation tail for the best byte-heuristic candidates.")
    report.append("- Scores penalize post-relocation disassembly that still looks like sparse data.")
    report.append(f"- Candidate pool: `{min(len(candidates), args.candidate_pool)}`")
    report.append("")
    report.append("## Top Materialized Candidates")
    report.append("")
    for score, candidate, lines in ranked[: args.topn]:
        report.extend(render_candidate(candidate, score, lines))
    report.append("## Cleanest Relocated Disassembly")
    report.append("")
    report.append("- Sorted by lowest suspicious-instruction penalty, then original byte-heuristic score.")
    report.append("")
    for score, candidate, lines in cleanest[: args.topn]:
        report.extend(render_clean_candidate(candidate, score, lines))
    report.append("## Best Local Starts")
    report.append("")
    report.append("- Scores small offsets after each materialized entry, useful when the far jump lands on a header or prefix bytes before a plausible instruction run.")
    report.append("")
    seen_local: set[tuple[int, int]] = set()
    shown = 0
    for score, candidate, start, lines in local_starts:
        key = (candidate.jump_linear_64k, start)
        if key in seen_local:
            continue
        seen_local.add(key)
        report.extend(render_local_start(candidate, start, score, lines))
        shown += 1
        if shown >= args.topn:
            break

    Path(args.report_out).write_text("\n".join(report), encoding="utf-8")
    print(f"Wrote materialized entry candidate ranking: {args.report_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

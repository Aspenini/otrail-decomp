#!/usr/bin/env python3
"""Search candidate CS placements for the exact 0x127EC relocation tail model."""

from __future__ import annotations

import argparse
import csv
import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path

MEM_SIZE = 0x10000
STREAM_OFFSET = 0x0158
CONTROL_SEGMENT_BUMP = 0x0000
CONTROL_TERMINAL = 0x0001
CONTROL_FLOW_BYTES = {0xE8, 0xE9, 0xEA, 0x9A, 0xC3, 0xCB, 0xCF}


def read_u16le(buf: bytes | bytearray, off: int) -> int:
    return buf[off] | (buf[(off + 1) & 0xFFFF] << 8)


def leading_zero_prefix(data: bytes) -> int:
    for idx, byte in enumerate(data):
        if byte != 0:
            return idx
    return len(data)


def maybe_disasm_sample(payload: Path, offset: int, lines: int) -> list[str]:
    ndisasm = shutil.which("ndisasm")
    if ndisasm is None:
        return []
    completed = subprocess.run(
        [ndisasm, "-b", "16", "-e", str(offset), "-o", hex(offset), str(payload)],
        check=True,
        capture_output=True,
        text=True,
    )
    return completed.stdout.splitlines()[:lines]


@dataclass
class TailCandidate:
    cs_base: int
    bx_seed: int
    steps: int
    segment_bumps: int
    header_hits: int
    jump_off: int
    jump_seg_word: int
    stack_off: int
    stack_seg_rel: int
    jump_linear_20: int
    jump_linear_64k: int
    target_nonzero: int
    target_ctrl: int
    target_zero_prefix: int
    target_ascii: int
    target_first_ctrl: int
    final_ds: int
    final_es: int
    final_ss: int
    final_sp: int

    @property
    def code_score(self) -> int:
        score = self.target_ctrl * 32
        score += self.target_nonzero
        score += self.target_ascii // 4
        score += 12 if self.header_hits > 0 else 0
        score -= self.target_zero_prefix * 2
        score -= min(self.segment_bumps, 512) // 16
        return score

    @property
    def sample_anchor(self) -> int:
        if self.target_first_ctrl >= 0:
            return self.jump_linear_64k + self.target_first_ctrl
        return self.jump_linear_64k + self.target_zero_prefix


@dataclass
class TailFamily:
    jump_linear_64k: int
    members: list[TailCandidate]

    @property
    def best(self) -> TailCandidate:
        return max(self.members, key=lambda row: row.code_score)

    @property
    def bx_values(self) -> list[int]:
        return sorted({row.bx_seed for row in self.members})

    @property
    def cs_values(self) -> list[int]:
        return sorted({row.cs_base for row in self.members})


def simulate_candidate(
    payload: bytes,
    cs_base: int,
    bx_seed: int,
    max_steps: int,
    sample_len: int,
) -> TailCandidate | None:
    mem = bytearray(MEM_SIZE)
    mem[: len(payload)] = payload
    si = (cs_base + STREAM_OFFSET) & 0xFFFF
    dx_rel = bx_seed & 0xFFFF
    di = 0
    steps = 0
    segment_bumps = 0
    header_hits = 0

    if si >= len(mem):
        return None

    value = mem[si]
    si = (si + 1) & 0xFFFF
    if value == 0:
        control_word = read_u16le(mem, si)
        si = (si + 2) & 0xFFFF
        if control_word == CONTROL_SEGMENT_BUMP:
            dx_rel = (dx_rel + 0x0FFF) & 0xFFFF
            segment_bumps += 1
            steps += 1
        elif control_word == CONTROL_TERMINAL:
            jump_off = read_u16le(mem, cs_base)
            jump_seg_word = read_u16le(mem, (cs_base + 2) & 0xFFFF)
            stack_off = read_u16le(mem, (cs_base + 4) & 0xFFFF)
            stack_seg_rel = read_u16le(mem, (cs_base + 6) & 0xFFFF)
            jump_linear_20 = ((((jump_seg_word + bx_seed) & 0xFFFF) << 4) + jump_off) & 0xFFFFF
            jump_linear_64k = jump_linear_20 & 0xFFFF
            window = payload[jump_linear_64k:jump_linear_64k + sample_len]
            first_ctrl = next((idx for idx, byte in enumerate(window) if byte in CONTROL_FLOW_BYTES), -1)
            return TailCandidate(
                cs_base=cs_base,
                bx_seed=bx_seed,
                steps=steps,
                segment_bumps=segment_bumps,
                header_hits=header_hits,
                jump_off=jump_off,
                jump_seg_word=jump_seg_word,
                stack_off=stack_off,
                stack_seg_rel=stack_seg_rel,
                jump_linear_20=jump_linear_20,
                jump_linear_64k=jump_linear_64k,
                target_nonzero=sum(byte != 0 for byte in window),
                target_ctrl=sum(byte in CONTROL_FLOW_BYTES for byte in window),
                target_zero_prefix=leading_zero_prefix(window),
                target_ascii=sum(32 <= byte < 127 for byte in window),
                target_first_ctrl=first_ctrl,
                final_ds=(bx_seed - 0x0010) & 0xFFFF,
                final_es=(bx_seed - 0x0010) & 0xFFFF,
                final_ss=(stack_seg_rel + bx_seed) & 0xFFFF,
                final_sp=stack_off,
            )
        value = control_word & 0x00FF

    di = (di + value) & 0xFFFF
    dx_rel = (dx_rel + (di >> 4)) & 0xFFFF
    di &= 0x000F
    patch_off = ((dx_rel << 4) + di) & 0xFFFF
    if cs_base <= patch_off < cs_base + 8 or cs_base <= ((patch_off + 1) & 0xFFFF) < cs_base + 8:
        header_hits += 1
    patched = (read_u16le(mem, patch_off) + bx_seed) & 0xFFFF
    mem[patch_off] = patched & 0xFF
    mem[(patch_off + 1) & 0xFFFF] = (patched >> 8) & 0xFF
    steps += 1

    while steps < max_steps:
        value = mem[si]
        si = (si + 1) & 0xFFFF

        if value == 0:
            control_word = read_u16le(mem, si)
            si = (si + 2) & 0xFFFF
            if control_word == CONTROL_SEGMENT_BUMP:
                dx_rel = (dx_rel + 0x0FFF) & 0xFFFF
                segment_bumps += 1
                steps += 1
                continue
            if control_word == CONTROL_TERMINAL:
                jump_off = read_u16le(mem, cs_base)
                jump_seg_word = read_u16le(mem, (cs_base + 2) & 0xFFFF)
                stack_off = read_u16le(mem, (cs_base + 4) & 0xFFFF)
                stack_seg_rel = read_u16le(mem, (cs_base + 6) & 0xFFFF)
                jump_linear_20 = ((((jump_seg_word + bx_seed) & 0xFFFF) << 4) + jump_off) & 0xFFFFF
                jump_linear_64k = jump_linear_20 & 0xFFFF
                window = payload[jump_linear_64k:jump_linear_64k + sample_len]
                first_ctrl = next((idx for idx, byte in enumerate(window) if byte in CONTROL_FLOW_BYTES), -1)
                return TailCandidate(
                    cs_base=cs_base,
                    bx_seed=bx_seed,
                    steps=steps,
                    segment_bumps=segment_bumps,
                    header_hits=header_hits,
                    jump_off=jump_off,
                    jump_seg_word=jump_seg_word,
                    stack_off=stack_off,
                    stack_seg_rel=stack_seg_rel,
                    jump_linear_20=jump_linear_20,
                    jump_linear_64k=jump_linear_64k,
                    target_nonzero=sum(byte != 0 for byte in window),
                    target_ctrl=sum(byte in CONTROL_FLOW_BYTES for byte in window),
                    target_zero_prefix=leading_zero_prefix(window),
                    target_ascii=sum(32 <= byte < 127 for byte in window),
                    target_first_ctrl=first_ctrl,
                    final_ds=(bx_seed - 0x0010) & 0xFFFF,
                    final_es=(bx_seed - 0x0010) & 0xFFFF,
                    final_ss=(stack_seg_rel + bx_seed) & 0xFFFF,
                    final_sp=stack_off,
                )
            value = control_word & 0x00FF

        di = (di + value) & 0xFFFF
        paragraph_carry = di >> 4
        di &= 0x000F
        dx_rel = (dx_rel + paragraph_carry) & 0xFFFF
        patch_off = ((dx_rel << 4) + di) & 0xFFFF
        if cs_base <= patch_off < cs_base + 8 or cs_base <= ((patch_off + 1) & 0xFFFF) < cs_base + 8:
            header_hits += 1
        patched = (read_u16le(mem, patch_off) + bx_seed) & 0xFFFF
        mem[patch_off] = patched & 0xFF
        mem[(patch_off + 1) & 0xFFFF] = (patched >> 8) & 0xFF
        steps += 1

    return None


def render_candidate(candidate: TailCandidate) -> str:
    return (
        f"- `cs_base=0x{candidate.cs_base:04X}` "
        f"`bx=0x{candidate.bx_seed:04X}` "
        f"`score={candidate.code_score}` "
        f"`jump_rel=0x{candidate.jump_linear_64k:04X}` "
        f"`steps={candidate.steps}` "
        f"`seg_bumps={candidate.segment_bumps}` "
        f"`header_hits={candidate.header_hits}` "
        f"`jump_ip=0x{candidate.jump_off:04X}` "
        f"`jump_seg_word=0x{candidate.jump_seg_word:04X}` "
        f"`final_ds=0x{candidate.final_ds:04X}` "
        f"`final_es=0x{candidate.final_es:04X}` "
        f"`final_ss=0x{candidate.final_ss:04X}` "
        f"`final_sp=0x{candidate.final_sp:04X}` "
        f"`sp=0x{candidate.stack_off:04X}` "
        f"`ss_rel=0x{candidate.stack_seg_rel:04X}` "
        f"`target_nonzero={candidate.target_nonzero}` "
        f"`target_ctrl={candidate.target_ctrl}` "
        f"`target_ascii={candidate.target_ascii}` "
        f"`target_zero_prefix={candidate.target_zero_prefix}` "
        f"`target_first_ctrl={candidate.target_first_ctrl}`"
    )

def render_family(family: TailFamily) -> str:
    best = family.best
    shown_bx = ", ".join(f"0x{value:04X}" for value in family.bx_values[:8])
    if len(family.bx_values) > 8:
        shown_bx += f", ... ({len(family.bx_values)} total)"
    return (
        f"- `jump_rel=0x{family.jump_linear_64k:04X}` "
        f"`members={len(family.members)}` "
        f"`bx_count={len(family.bx_values)}` "
        f"`cs_count={len(family.cs_values)}` "
        f"`best_score={best.code_score}` "
        f"`best_cs=0x{best.cs_base:04X}` "
        f"`best_bx=0x{best.bx_seed:04X}` "
        f"`best_ctrl={best.target_ctrl}` "
        f"`best_nonzero={best.target_nonzero}` "
        f"`best_anchor=0x{best.sample_anchor:04X}` "
        f"`bx_values={shown_bx}`"
    )


def load_candidates_from_runner(
    runner: Path,
    payload_path: Path,
    max_cs_base: int,
    cs_step: int,
    bx_start: int,
    bx_end: int,
    bx_step: int,
    max_steps: int,
    sample_len: int,
) -> list[TailCandidate]:
    completed = subprocess.run(
        [
            str(runner),
            str(payload_path),
            hex(max_cs_base),
            hex(cs_step),
            hex(bx_start),
            hex(bx_end),
            hex(bx_step),
            str(max_steps),
            str(sample_len),
        ],
        check=True,
        capture_output=True,
        text=True,
    )
    lines = completed.stdout.splitlines()
    if not lines:
        return []

    out: list[TailCandidate] = []
    for row in csv.DictReader(lines, delimiter="\t"):
        normalized = {}
        for key, value in row.items():
            if value is None:
                normalized[key] = ""
            elif "=" in value:
                normalized[key] = value.split("=", 1)[1]
            else:
                normalized[key] = value
        out.append(
            TailCandidate(
                cs_base=int(normalized["cs_base"], 0),
                bx_seed=int(normalized["bx_seed"], 0),
                steps=int(normalized["steps"], 0),
                segment_bumps=int(normalized["segment_bumps"], 0),
                header_hits=int(normalized["header_hits"], 0),
                jump_off=int(normalized["jump_off"], 0),
                jump_seg_word=int(normalized["jump_seg_word"], 0),
                stack_off=int(normalized["stack_off"], 0),
                stack_seg_rel=int(normalized["stack_seg_rel"], 0),
                jump_linear_20=int(normalized["jump_linear_20"], 0),
                jump_linear_64k=int(normalized["jump_linear_64k"], 0),
                target_nonzero=int(normalized["target_nonzero"], 0),
                target_ctrl=int(normalized["target_ctrl"], 0),
                target_zero_prefix=int(normalized["target_zero_prefix"], 0),
                target_ascii=int(normalized["target_ascii"], 0),
                target_first_ctrl=int(normalized["target_first_ctrl"], 0),
                final_ds=int(normalized["final_ds"], 0),
                final_es=int(normalized["final_es"], 0),
                final_ss=int(normalized["final_ss"], 0),
                final_sp=int(normalized["final_sp"], 0),
            )
        )
    return out


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--payload", default="build/entry_bootstrap_replay_readable_heuristic.bin")
    parser.add_argument("--report-out", default="build/entry_tail_candidates_report.md")
    parser.add_argument("--runner", default="build/entry_tail_search")
    parser.add_argument("--max-cs-base", type=lambda s: int(s, 0), default=0x4000)
    parser.add_argument("--cs-step", type=lambda s: int(s, 0), default=0x10)
    parser.add_argument("--bx-seed", type=lambda s: int(s, 0), default=0x10)
    parser.add_argument("--bx-start", type=lambda s: int(s, 0))
    parser.add_argument("--bx-end", type=lambda s: int(s, 0))
    parser.add_argument("--bx-step", type=lambda s: int(s, 0), default=0x10)
    parser.add_argument("--max-steps", type=int, default=20000)
    parser.add_argument("--sample-len", type=int, default=128)
    parser.add_argument("--sample-lines", type=int, default=20)
    parser.add_argument("--topn", type=int, default=12)
    args = parser.parse_args()

    payload_path = Path(args.payload)
    report_out = Path(args.report_out)
    payload = payload_path.read_bytes()
    runner = Path(args.runner)

    candidates: list[TailCandidate] = []
    families: list[TailFamily] = []
    upper = min(args.max_cs_base, max(0, len(payload) - STREAM_OFFSET))
    if args.bx_start is not None or args.bx_end is not None:
        bx_start = args.bx_start if args.bx_start is not None else args.bx_seed
        bx_end = args.bx_end if args.bx_end is not None else args.bx_seed
        bx_values = range(bx_start, bx_end + args.bx_step, args.bx_step)
    else:
        bx_values = [args.bx_seed]
    bx_values = list(bx_values)
    if runner.exists():
        candidates = load_candidates_from_runner(
            runner,
            payload_path,
            upper,
            args.cs_step,
            bx_values[0],
            bx_values[-1],
            args.bx_step,
            args.max_steps,
            args.sample_len,
        )
    else:
        for bx_seed in bx_values:
            for cs_base in range(0, upper, args.cs_step):
                candidate = simulate_candidate(
                    payload,
                    cs_base,
                    bx_seed,
                    args.max_steps,
                    args.sample_len,
                )
                if candidate is not None and candidate.jump_linear_64k < len(payload):
                    candidates.append(candidate)

    candidates.sort(
        key=lambda row: (
            row.code_score,
            row.target_ctrl,
            row.target_nonzero,
            -row.target_zero_prefix,
            1 if row.header_hits > 0 else 0,
            -row.segment_bumps,
        ),
        reverse=True,
    )
    top = candidates[: args.topn]
    by_jump: dict[int, list[TailCandidate]] = {}
    for candidate in candidates:
        by_jump.setdefault(candidate.jump_linear_64k, []).append(candidate)
    families = [TailFamily(jump_linear_64k=jump, members=members) for jump, members in by_jump.items()]
    families.sort(
        key=lambda family: (
            family.best.code_score,
            len(family.members),
            family.best.target_ctrl,
            family.best.target_nonzero,
        ),
        reverse=True,
    )
    top_families = families[: args.topn]

    lines: list[str] = []
    lines.append("# Entry Tail Candidate Report")
    lines.append("")
    lines.append("## Model")
    lines.append("")
    lines.append("- Exact tail decode starts at packed offset `0x127EC`.")
    lines.append("- Stream source is modeled at `CS:0x0158`.")
    lines.append("- The model now includes the exact `unit_0004` bootstrap before `unit_0005`: `pop bx ; add bx,0x10 ; mov dx,bx ; xor di,di ; lodsb`.")
    lines.append("- `0x0000` control means `DX += 0x0FFF` within a 64K relocation window.")
    lines.append("- `0x0001` control means terminal handoff via header words at `CS:0x0000`.")
    lines.append("- Reported `bx` values are post-`add bx,0x10` seeds.")
    if args.bx_start is not None or args.bx_end is not None:
        bx_start = args.bx_start if args.bx_start is not None else args.bx_seed
        bx_end = args.bx_end if args.bx_end is not None else args.bx_seed
        lines.append(
            f"- Search range: `cs_base=0x0000..0x{upper:04X}` step `0x{args.cs_step:02X}`, "
            f"`bx=0x{bx_start:04X}..0x{bx_end:04X}` step `0x{args.bx_step:02X}`."
        )
    else:
        lines.append(
            f"- Search range: `cs_base=0x0000..0x{upper:04X}` step `0x{args.cs_step:02X}`, "
            f"fixed `bx_seed=0x{args.bx_seed:04X}`."
        )
    lines.append(f"- Candidates found with in-payload jump targets: `{len(candidates)}`.")
    lines.append(f"- Distinct jump-target families: `{len(families)}`.")
    lines.append("")
    lines.append("## Families")
    lines.append("")
    if not top_families:
        lines.append("- No families found under the current search assumptions.")
    else:
        for family in top_families:
            lines.append(render_family(family))
    lines.append("")
    lines.append("## Top Candidates")
    lines.append("")
    if not top:
        lines.append("- No in-payload candidates found under the current fixed-basis assumptions.")
    else:
        for candidate in top:
            lines.append(render_candidate(candidate))
    lines.append("")

    if top:
        lines.append("## Disassembly Samples")
        lines.append("")
        for family in top_families[:3]:
            candidate = family.best
            lines.append(
                f"### `jump_rel=0x{candidate.jump_linear_64k:04X}` "
                f"`anchor=0x{candidate.sample_anchor:04X}` "
                f"`best_cs=0x{candidate.cs_base:04X}` "
                f"`best_bx=0x{candidate.bx_seed:04X}`"
            )
            lines.append("")
            lines.append("```asm")
            sample = maybe_disasm_sample(payload_path, candidate.sample_anchor, args.sample_lines)
            if sample:
                lines.extend(sample)
            else:
                lines.append("; ndisasm not available")
            lines.append("```")
            lines.append("")

    report_out.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote entry tail candidate report: {report_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

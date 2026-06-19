#!/usr/bin/env python3
"""Materialize a candidate post-tail relocated runtime image."""

from __future__ import annotations

import argparse
import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path

from report_entry_tail_candidates import (
    CONTROL_SEGMENT_BUMP,
    CONTROL_TERMINAL,
    MEM_SIZE,
    STREAM_OFFSET,
    read_u16le,
)


@dataclass
class RelocationPatch:
    step: int
    offset: int
    before: int
    after: int


@dataclass
class MaterializedTailImage:
    mem: bytearray
    steps: int
    segment_bumps: int
    patches: list[RelocationPatch]
    jump_off: int
    jump_seg_word: int
    stack_off: int
    stack_seg_rel: int
    final_ds: int
    final_es: int
    final_ss: int
    final_sp: int

    @property
    def entry_linear_20(self) -> int:
        return ((((self.jump_seg_word + self.final_ds + 0x10) & 0xFFFF) << 4) + self.jump_off) & 0xFFFFF

    @property
    def entry_linear_64k(self) -> int:
        return self.entry_linear_20 & 0xFFFF


def disasm_sample(path: Path, offset: int, lines: int) -> list[str]:
    ndisasm = shutil.which("ndisasm")
    if ndisasm is None:
        return ["; ndisasm not available"]
    completed = subprocess.run(
        [ndisasm, "-b", "16", "-e", str(offset), "-o", hex(offset), str(path)],
        check=True,
        capture_output=True,
        text=True,
    )
    return completed.stdout.splitlines()[:lines]


def materialize_tail_image(payload: bytes, cs_base: int, bx_seed: int, max_steps: int) -> MaterializedTailImage:
    mem = bytearray(MEM_SIZE)
    mem[: len(payload)] = payload
    si = (cs_base + STREAM_OFFSET) & 0xFFFF
    dx_rel = bx_seed & 0xFFFF
    di = 0
    steps = 0
    segment_bumps = 0
    patches: list[RelocationPatch] = []

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
                final_base = (bx_seed - 0x0010) & 0xFFFF
                return MaterializedTailImage(
                    mem=mem,
                    steps=steps,
                    segment_bumps=segment_bumps,
                    patches=patches,
                    jump_off=jump_off,
                    jump_seg_word=jump_seg_word,
                    stack_off=stack_off,
                    stack_seg_rel=stack_seg_rel,
                    final_ds=final_base,
                    final_es=final_base,
                    final_ss=(stack_seg_rel + bx_seed) & 0xFFFF,
                    final_sp=stack_off,
                )
            value = control_word & 0x00FF

        di = (di + value) & 0xFFFF
        dx_rel = (dx_rel + (di >> 4)) & 0xFFFF
        di &= 0x000F
        patch_off = ((dx_rel << 4) + di) & 0xFFFF
        before = read_u16le(mem, patch_off)
        after = (before + bx_seed) & 0xFFFF
        mem[patch_off] = after & 0xFF
        mem[(patch_off + 1) & 0xFFFF] = (after >> 8) & 0xFF
        patches.append(RelocationPatch(step=steps, offset=patch_off, before=before, after=after))
        steps += 1

    raise RuntimeError(f"tail did not reach terminal control within {max_steps} steps")


def write_report(
    report_out: Path,
    image_out: Path,
    payload_path: Path,
    image: MaterializedTailImage,
    sample_lines: int,
) -> None:
    lines: list[str] = []
    lines.append("# Materialized Entry Tail Image")
    lines.append("")
    lines.append(f"- Payload: `{payload_path}`")
    lines.append(f"- Image: `{image_out}`")
    lines.append(f"- Relocation steps: `{image.steps}`")
    lines.append(f"- Segment bumps: `{image.segment_bumps}`")
    lines.append(f"- Word patches: `{len(image.patches)}`")
    lines.append(f"- Final DS/ES: `0x{image.final_ds:04X}`")
    lines.append(f"- Final SS:SP: `0x{image.final_ss:04X}:0x{image.final_sp:04X}`")
    lines.append(f"- Far jump header: `0x{image.jump_seg_word:04X}:0x{image.jump_off:04X}`")
    lines.append(f"- Entry linear 20-bit: `0x{image.entry_linear_20:05X}`")
    lines.append(f"- Entry linear 64K alias: `0x{image.entry_linear_64k:04X}`")
    lines.append("")
    lines.append("## Entry Disassembly")
    lines.append("")
    lines.append("```asm")
    lines.extend(disasm_sample(image_out, image.entry_linear_64k, sample_lines))
    lines.append("```")
    lines.append("")
    lines.append("## Patch Summary")
    lines.append("")
    for patch in image.patches[:32]:
        lines.append(
            f"- `step={patch.step}` `off=0x{patch.offset:04X}` "
            f"`before=0x{patch.before:04X}` `after=0x{patch.after:04X}`"
        )
    if len(image.patches) > 32:
        lines.append(f"- ... `{len(image.patches) - 32}` additional patches omitted")
    report_out.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--payload", default="build/entry_bootstrap_replay_readable_heuristic.bin")
    parser.add_argument("--cs-base", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--bx-seed", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--max-steps", type=int, default=20000)
    parser.add_argument("--image-out", default="build/entry_tail_materialized.bin")
    parser.add_argument("--report-out", default="build/entry_tail_materialized_report.md")
    parser.add_argument("--sample-lines", type=int, default=48)
    args = parser.parse_args()

    payload_path = Path(args.payload)
    image_out = Path(args.image_out)
    report_out = Path(args.report_out)
    payload = payload_path.read_bytes()
    image = materialize_tail_image(payload, args.cs_base, args.bx_seed, args.max_steps)
    image_out.parent.mkdir(parents=True, exist_ok=True)
    image_out.write_bytes(image.mem)
    write_report(report_out, image_out, payload_path, image, args.sample_lines)
    print(f"Wrote materialized tail image: {image_out}")
    print(f"Wrote materialized tail report: {report_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

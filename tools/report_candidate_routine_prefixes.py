#!/usr/bin/env python3
"""Scan unpacked images for short routine-prefix shapes worth semantic follow-up."""

from __future__ import annotations

import argparse
import json
import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Any


PREFIX_SIZE = 21


@dataclass(frozen=True)
class RoutinePrefixCandidate:
    image_label: str
    offset: int
    loop_count: int
    far_call_offset: int
    far_call_segment: int
    save_ax_bp_disp: int
    save_bx_bp_disp: int
    save_dx_bp_disp: int

    @property
    def score(self) -> int:
        score = 0
        if self.loop_count != 0:
            score += 10
        if self.far_call_segment not in (0x0000, 0xFFFF):
            score += 20
        if (self.save_ax_bp_disp, self.save_bx_bp_disp, self.save_dx_bp_disp) == (-12, -10, -8):
            score += 30
        if self.far_call_offset < 0x0100:
            score += 5
        return score

    @property
    def far_call(self) -> str:
        return f"0x{self.far_call_segment:04X}:0x{self.far_call_offset:04X}"


def s8(byte: int) -> int:
    return byte - 0x100 if byte & 0x80 else byte


def u16le(data: bytes, offset: int) -> int:
    return data[offset] | (data[offset + 1] << 8)


def parse_int(value: Any) -> int:
    if isinstance(value, int):
        return value
    return int(str(value), 0)


def scan_image(label: str, data: bytes) -> list[RoutinePrefixCandidate]:
    out: list[RoutinePrefixCandidate] = []
    for offset in range(0, max(0, len(data) - PREFIX_SIZE + 1)):
        if data[offset] != 0xB9:
            continue
        if data[offset + 3:offset + 7] != b"\x31\xF6\x31\xFF":
            continue
        if data[offset + 7] != 0x9A:
            continue
        if data[offset + 12:offset + 14] != b"\x89\x46":
            continue
        if data[offset + 15:offset + 17] != b"\x89\x5E":
            continue
        if data[offset + 18:offset + 20] != b"\x89\x56":
            continue
        out.append(
            RoutinePrefixCandidate(
                image_label=label,
                offset=offset,
                loop_count=u16le(data, offset + 1),
                far_call_offset=u16le(data, offset + 8),
                far_call_segment=u16le(data, offset + 10),
                save_ax_bp_disp=s8(data[offset + 14]),
                save_bx_bp_disp=s8(data[offset + 17]),
                save_dx_bp_disp=s8(data[offset + 20]),
            )
        )
    return out


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


def by_offset(candidates: list[RoutinePrefixCandidate]) -> dict[int, RoutinePrefixCandidate]:
    return {candidate.offset: candidate for candidate in candidates}


def load_runtime_regions(path: Path) -> list[dict[str, Any]]:
    if not path.exists():
        return []
    data = json.loads(path.read_text(encoding="utf-8"))
    return [row for row in data.get("regions", []) if isinstance(row, dict)]


def containing_runtime_regions(regions: list[dict[str, Any]], offset: int) -> list[dict[str, Any]]:
    out: list[dict[str, Any]] = []
    for region in regions:
        start = parse_int(region.get("start"))
        end = parse_int(region.get("end"))
        if start <= offset < end:
            out.append(region)
    return out


def render_region_refs(regions: list[dict[str, Any]]) -> str:
    if not regions:
        return "`unmapped`"
    return ", ".join(
        f"`{region.get('id')}`/{region.get('family')}/{region.get('role')}"
        for region in regions
    )


def render_offset_refs(offsets: list[int]) -> str:
    return ", ".join(f"`0x{offset:04X}`" for offset in offsets) if offsets else "`none`"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--payload", default="build/entry_bootstrap_replay_readable_heuristic.bin")
    parser.add_argument("--materialized", action="append", default=[])
    parser.add_argument("--report-out", default="build/candidate_routine_prefix_report.md")
    parser.add_argument("--runtime-map", default="config/unpacked_runtime_map.json")
    parser.add_argument("--sample-lines", type=int, default=10)
    args = parser.parse_args()
    runtime_regions = load_runtime_regions(Path(args.runtime_map))

    image_paths = [("unpacked payload", Path(args.payload))]
    for item in args.materialized:
        image_paths.append((Path(item).name, Path(item)))

    candidates_by_image: dict[str, list[RoutinePrefixCandidate]] = {}
    paths_by_image: dict[str, Path] = {}
    candidates: list[tuple[RoutinePrefixCandidate, Path]] = []
    for label, path in image_paths:
        if not path.exists():
            continue
        data = path.read_bytes()
        image_candidates = scan_image(label, data)
        candidates_by_image[label] = image_candidates
        paths_by_image[label] = path
        candidates.extend((candidate, path) for candidate in image_candidates)
    candidates.sort(key=lambda row: (-row[0].score, row[0].image_label, row[0].offset))

    lines: list[str] = []
    lines.append("# Candidate Routine Prefix Report")
    lines.append("")
    lines.append("## Summary")
    lines.append("")
    lines.append(f"- Candidate prefixes found: `{len(candidates)}`")
    lines.append("- Pattern: `mov cx,imm16; xor si,si; xor di,di; far call; save AX/BX/DX to BP-relative locals`")
    lines.append("")
    if runtime_regions and "unpacked payload" in candidates_by_image:
        lines.append("## Runtime Map Classification")
        lines.append("")
        for candidate in sorted(candidates_by_image["unpacked payload"], key=lambda item: item.offset):
            lines.append(
                f"- `0x{candidate.offset:04X}` regions: "
                f"{render_region_refs(containing_runtime_regions(runtime_regions, candidate.offset))}"
            )
        lines.append("")
    if len(candidates_by_image) >= 2 and "unpacked payload" in candidates_by_image:
        payload = by_offset(candidates_by_image["unpacked payload"])
        lines.append("## Image Comparison")
        lines.append("")
        for label, image_candidates in candidates_by_image.items():
            if label == "unpacked payload":
                continue
            other = by_offset(image_candidates)
            shared = sorted(set(payload) & set(other))
            payload_only = sorted(set(payload) - set(other))
            other_only = sorted(set(other) - set(payload))
            changed = [
                offset
                for offset in shared
                if (
                    payload[offset].far_call_segment,
                    payload[offset].far_call_offset,
                    payload[offset].loop_count,
                    payload[offset].save_ax_bp_disp,
                    payload[offset].save_bx_bp_disp,
                    payload[offset].save_dx_bp_disp,
                )
                != (
                    other[offset].far_call_segment,
                    other[offset].far_call_offset,
                    other[offset].loop_count,
                    other[offset].save_ax_bp_disp,
                    other[offset].save_bx_bp_disp,
                    other[offset].save_dx_bp_disp,
                )
            ]
            lines.append(f"### `{label}` vs `unpacked payload`")
            lines.append("")
            lines.append(f"- Shared offsets: `{len(shared)}`")
            lines.append(f"- Payload-only offsets: {', '.join(f'`0x{offset:04X}`' for offset in payload_only) or '`none`'}")
            lines.append(f"- Materialized-only offsets: {', '.join(f'`0x{offset:04X}`' for offset in other_only) or '`none`'}")
            if changed:
                lines.append(
                    "- Changed descriptors: "
                    + ", ".join(
                        f"`0x{offset:04X}` {payload[offset].far_call}->{other[offset].far_call}"
                        for offset in changed
                    )
                )
            else:
                lines.append("- Changed descriptors: `none`")
            lines.append("")
    if candidates_by_image:
        lines.append("## Exact Prefix Byte Groups")
        lines.append("")
        for label, image_candidates in candidates_by_image.items():
            path = paths_by_image[label]
            data = path.read_bytes()
            groups: dict[str, list[int]] = {}
            for candidate in image_candidates:
                prefix_hex = data[candidate.offset:candidate.offset + PREFIX_SIZE].hex().upper()
                groups.setdefault(prefix_hex, []).append(candidate.offset)
            lines.append(f"### `{label}`")
            lines.append("")
            for prefix_hex, offsets in sorted(groups.items(), key=lambda item: (-len(item[1]), item[1])):
                lines.append(f"- `{len(offsets)}` hits at {render_offset_refs(offsets)}")
                lines.append(f"  - bytes: `{prefix_hex}`")
            lines.append("")
    lines.append("## Candidates")
    lines.append("")
    if not candidates:
        lines.append("- none")
    for candidate, path in candidates:
        lines.append(
            f"- `score={candidate.score}` `{candidate.image_label}` `offset=0x{candidate.offset:04X}` "
            f"`cx=0x{candidate.loop_count:04X}` `far_call={candidate.far_call}` "
            f"`saves={candidate.save_ax_bp_disp},{candidate.save_bx_bp_disp},{candidate.save_dx_bp_disp}`"
        )
        if candidate.image_label == "unpacked payload" and runtime_regions:
            lines.append(f"- Runtime regions: {render_region_refs(containing_runtime_regions(runtime_regions, candidate.offset))}")
        lines.append("")
        lines.append("```asm")
        lines.extend(disasm_sample(path, candidate.offset, args.sample_lines))
        lines.append("```")
        lines.append("")

    report_out = Path(args.report_out)
    report_out.write_text("\n".join(lines), encoding="utf-8")
    print(f"Wrote candidate routine-prefix report: {report_out}")
    print(f"Candidate prefixes found: {len(candidates)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Map unpacker trace events into provisional source-offset blocks."""

from __future__ import annotations

import argparse
import csv
import json
from dataclasses import dataclass, field
from pathlib import Path


def parse_int_auto(text: str | int) -> int:
    if isinstance(text, int):
        return text
    return int(text, 0)


@dataclass
class Block:
    block_id: str
    start: int
    end: int
    label: str
    anchors: list[str]
    events: int = 0
    src_bytes: int = 0
    out_bytes: int = 0
    type_counts: dict[str, int] = field(default_factory=dict)
    sample_events: list[int] = field(default_factory=list)

    def contains(self, addr: int) -> bool:
        return self.start <= addr <= self.end

    def add(self, row: dict[str, str]) -> None:
        self.events += 1
        self.src_bytes += int(row["src_delta"])
        self.out_bytes += int(row["out_delta"])
        etype = row["type"]
        self.type_counts[etype] = self.type_counts.get(etype, 0) + 1
        idx = int(row["event_idx"])
        if len(self.sample_events) < 4:
            self.sample_events.append(idx)

    def dominant_type(self) -> str:
        if not self.type_counts:
            return "-"
        return max(self.type_counts.items(), key=lambda kv: kv[1])[0]


def load_blocks(path: Path) -> list[Block]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    blocks: list[Block] = []
    for item in payload.get("blocks", []):
        blocks.append(
            Block(
                block_id=item["id"],
                start=parse_int_auto(item["start"]),
                end=parse_int_auto(item["end"]),
                label=item["label"],
                anchors=list(item.get("anchors", [])),
            )
        )
    blocks.sort(key=lambda b: b.start)
    return blocks


def find_block(blocks: list[Block], addr: int) -> Block | None:
    for block in blocks:
        if block.contains(addr):
            return block
    return None


def type_mix(counts: dict[str, int]) -> str:
    if not counts:
        return "-"
    parts = [f"{k}:{v}" for k, v in sorted(counts.items())]
    return ", ".join(parts)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--trace-csv", default="build/unpacker_trace.csv")
    parser.add_argument(
        "--blocks-json",
        default="config/entry_unpacker_blocks_1274f_12845.json",
    )
    parser.add_argument(
        "--report-out",
        default="build/unpacker_trace_blocks.md",
    )
    parser.add_argument(
        "--csv-out",
        default="build/unpacker_trace_blocks.csv",
    )
    args = parser.parse_args()

    trace_csv = Path(args.trace_csv)
    blocks_json = Path(args.blocks_json)
    report_out = Path(args.report_out)
    csv_out = Path(args.csv_out)

    blocks = load_blocks(blocks_json)
    if not trace_csv.exists():
        raise FileNotFoundError(f"missing trace csv: {trace_csv}")
    if not blocks:
        raise ValueError(f"no blocks found in {blocks_json}")

    unmapped = 0
    mapped_events = 0
    with trace_csv.open("r", encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            src_abs_before = parse_int_auto(row["src_abs_before"])
            block = find_block(blocks, src_abs_before)
            if block is None:
                unmapped += 1
                continue
            block.add(row)
            mapped_events += 1

    with csv_out.open("w", encoding="utf-8", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(
            [
                "block_id",
                "start_hex",
                "end_hex",
                "label",
                "events",
                "src_bytes",
                "out_bytes",
                "dominant_type",
                "type_mix",
                "sample_event_idxs",
            ]
        )
        for b in blocks:
            writer.writerow(
                [
                    b.block_id,
                    hex(b.start),
                    hex(b.end),
                    b.label,
                    b.events,
                    b.src_bytes,
                    b.out_bytes,
                    b.dominant_type(),
                    type_mix(b.type_counts),
                    ";".join(str(v) for v in b.sample_events),
                ]
            )

    lines = [
        "# Entry Unpacker Trace-to-Block Correlation",
        "",
        f"- trace csv: `{trace_csv}`",
        f"- blocks file: `{blocks_json}`",
        f"- mapped events: `{mapped_events}`",
        f"- unmapped events: `{unmapped}`",
        "",
        "| Block | Range | Events | Src Bytes | Out Bytes | Dominant Type | Type Mix | Samples |",
        "|---|---|---:|---:|---:|---|---|---|",
    ]
    for b in blocks:
        lines.append(
            f"| `{b.block_id}` | `{hex(b.start)}-{hex(b.end)}` | "
            f"{b.events} | {b.src_bytes} | {b.out_bytes} | {b.dominant_type()} | "
            f"{type_mix(b.type_counts)} | {','.join(str(v) for v in b.sample_events) or '-'} |"
        )

    lines.append("")
    lines.append("## Block Hypotheses")
    lines.append("")
    for b in blocks:
        anchor_text = "; ".join(b.anchors) if b.anchors else "-"
        lines.append(f"- `{b.block_id}` `{hex(b.start)}-{hex(b.end)}`: {b.label}")
        lines.append(f"  - anchors: {anchor_text}")

    report_out.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote {csv_out}")
    print(f"Wrote {report_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

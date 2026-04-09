# Entry Unpacker Block Correlation (First 100 Events)

This note maps the first 100 unpacker trace events (start offset `0x1274F`) to
provisional semantic blocks in `config/entry_unpacker_blocks_1274f_12845.json`.

## Generation Commands

```bash
make unpacker-trace-blocks TRACE_OFFSET=0x1274F TRACE_MAX_EVENTS=100 TRACE_MODE=1
```

Outputs:

- `build/unpacker_trace.csv`
- `build/unpacker_trace_blocks.csv`
- `build/unpacker_trace_blocks.md`

## Run Summary

- mapped events: `100`
- unmapped events: `0`
- trace status: `ok`
- trace cursors: `src_used=158`, `dst_written=720`

## Key Findings

- Strong output growth concentrates in:
  - `B04_short_len_build` (`out_bytes=246`)
  - `B05_short_backref_setup` (`out_bytes=245`)
- Segment/window slide block `B09_window_slide` is active in this sample:
  - `events=25`, dominant token class in mapped events: `short`
- Tail placeholder block `B10_tail_uncertain` still receives events
  (`events=3`), so this region remains a priority for refining code-vs-data
  interpretation.

## Top Block Totals (from report)

| Block | Events | Src Bytes | Out Bytes |
|---|---:|---:|---:|
| `B09_window_slide` | 25 | 37 | 75 |
| `B04_short_len_build` | 15 | 26 | 246 |
| `B06_long_token_decode` | 13 | 18 | 30 |
| `B03_second_gate_setup` | 9 | 14 | 26 |
| `B01_bit_gate_top` | 8 | 13 | 27 |

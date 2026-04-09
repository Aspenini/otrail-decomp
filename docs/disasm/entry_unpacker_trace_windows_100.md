# Entry Unpacker Trace Correlation (First 100 Events)

This note captures the first 100 unpacker events traced from `0x1274F` in
heuristic mode and binned into source-offset windows across the disassembly
focus range `0x1274F`-`0x12845`.

## Generation Commands

```bash
make unpacker-trace TRACE_OFFSET=0x1274F TRACE_MAX_EVENTS=100 TRACE_MODE=1
make unpacker-trace-windows TRACE_WINDOW_START=0x1274F TRACE_WINDOW_END=0x12845 TRACE_WINDOW_SIZE=0x10
```

Artifacts:

- `build/unpacker_trace.csv`
- `build/unpacker_trace_windows.md`

## Observed Summary

- Trace run result: `status=ok`, `events_written=100`, `src_used=158`,
  `dst_written=720`.
- Early event mix is interleaved literal + short/long copy traffic, matching
  a bit-gated token loop hypothesis.
- All first 100 events consume bytes in roughly `0x1274F`-`0x127EE`; no events
  landed in later windows up to `0x12845` in this sample.
- Highest expansion windows in this run:
  - `0x1278F-0x1279E`: `out_bytes=259`
  - `0x1276F-0x1277E`: `out_bytes=233`

## Window Table (Copied from Report)

| Window | Events | Src Bytes | Out Bytes | Dominant Type | Type Mix |
|---|---:|---:|---:|---|---|
| `0x1274f-0x1275e` | 10 | 16 | 22 | literal | literal:6, long:2, short:2 |
| `0x1275f-0x1276e` | 10 | 17 | 39 | long | literal:3, long:5, short:2 |
| `0x1276f-0x1277e` | 11 | 17 | 233 | literal | literal:6, long:3, short:2 |
| `0x1277f-0x1278e` | 9 | 15 | 23 | literal | literal:4, long:4, short:1 |
| `0x1278f-0x1279e` | 9 | 16 | 259 | short | literal:3, long:2, short:4 |
| `0x1279f-0x127ae` | 12 | 15 | 21 | literal | literal:8, long:1, short:3 |
| `0x127af-0x127be` | 10 | 17 | 33 | short | literal:3, long:3, short:4 |
| `0x127bf-0x127ce` | 11 | 17 | 33 | short | literal:2, long:2, short:7 |
| `0x127cf-0x127de` | 11 | 16 | 33 | short | literal:3, long:1, short:7 |
| `0x127df-0x127ee` | 7 | 12 | 24 | long | literal:2, long:3, short:2 |
| `0x127ef-0x127fe` | 0 | 0 | 0 | - | - |
| `0x127ff-0x1280e` | 0 | 0 | 0 | - | - |
| `0x1280f-0x1281e` | 0 | 0 | 0 | - | - |
| `0x1281f-0x1282e` | 0 | 0 | 0 | - | - |
| `0x1282f-0x1283e` | 0 | 0 | 0 | - | - |
| `0x1283f-0x12845` | 0 | 0 | 0 | - | - |

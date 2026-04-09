# Recovery Notes

## Contiguous Unit Chain (Entrypoint)

- contiguous range: `unit_0001` through `unit_0060`
- start offset: `0x126FE` (`75518`)
- end offset (exclusive): `0x135DA` (`79322`)
- total contiguous bytes configured in chain: `3804`
- binary tail remainder after contiguous slicing: `0` bytes

Next contiguous target should branch by code-flow boundaries (or new segment regions), not raw file-end continuation.

## Semantic Lift Milestone

- Added code/data boundary map:
  - `docs/disasm/entrypoint_chain_code_data_map.md`
- Added first inferred decoder model (non-authoritative, for behavior experiments):
  - `logic/entry_unpacker_model.c`
- Added harness to scan candidate decode offsets and compare decode characteristics:
  - `tools/entry_unpacker_scan.c`
  - `make unpacker-scan`
- Added targeted single-offset replay harness:
  - `tools/entry_unpacker_replay.c`
  - `make unpacker-replay REPLAY_OFFSET=0x1274F`
- Added replay fingerprint ranker:
  - `tools/entry_unpacker_fingerprint.py`
  - `make unpacker-fingerprint`
- Added confidence-band analyzer around candidate entry offsets:
  - `tools/entry_unpacker_confidence_band.py`
  - `make unpacker-band CENTER_OFFSET=0x12778 BAND_RADIUS=64 REPLAY_MODE=1`
- Latest harness report:
  - `docs/disasm/entry_unpacker_scan_results.md`
  - current focused scan window: `0x126C0`-`0x12880`
  - strongest candidates cluster around `0x126D*`
  - targeted replay comparison: `docs/disasm/entry_unpacker_replay_results.md`
  - current ranked replay entry candidate: `0x12778`
  - confidence-band winner (heuristic mode): `0x12778`
  - strict-mode band now also stabilizes around `0x12778` after preseeded history modeling
- Added full-chain disassembly and notes:
  - `docs/disasm/entrypoint_chain_0001_0060.ndisasm`
  - `docs/disasm/entrypoint_chain_0001_0060.notes.md`

## Additional Routine Lift

- Added inferred pre-unpack loader stage model:
  - `logic/entry_loader_model.c`
  - mapped notes: `docs/disasm/entry_loader_model_notes.md`

## Deterministic Validation Added

- Added unpacker fixture config:
  - `config/unpacker_fixtures.json`
- Added fixture checker:
  - `tools/check_unpacker_fixtures.py`
  - `make unpacker-fixtures`
- Current fixture set verifies strict mode behavior at:
  - 10 top confidence-band offsets near `0x12778`
- Current fixture set also verifies heuristic mode parity at the same offsets.
- Latest run:
  - `Fixture summary: total=20 fail=0`

## Unpacker Lift Refactor

- `logic/entry_unpacker_model.c` now has explicit internal stages:
  - literal emit path
  - long-token decode/copy path
  - short-token decode/copy path
  - shared history-copy primitive and centralized failure bookkeeping
- Refactor preserved behavior under the 20-fixture suite and baseline match checks.

## API / Build Cleanup

- Added public unpacker header:
  - `logic/entry_unpacker_model.h`
- Added internal engine split:
  - `logic/entry_unpacker_internal.h`
  - `logic/entry_unpacker_engine.c`
  - `logic/entry_unpacker_model.c` now acts as API wrapper layer
- Updated C tools to include header instead of source inclusion:
  - `tools/entry_unpacker_replay.c`
  - `tools/entry_unpacker_scan.c`
- Updated build commands to link `logic/entry_unpacker_model.c` explicitly.
- Added strict/heur fixture parity checker:
  - `tools/check_unpacker_mode_parity.py`
  - `make unpacker-parity`
  - latest parity run: `pairs=10 fail=0`
- Post-split validation:
  - `make unpacker-fixtures` -> `total=20 fail=0`
  - `make verify` / `make progress` unchanged and green

## Coverage Expansion Batch

- Added new matched coverage band in pre-entry region:
  - units: `unit_0061` through `unit_0092`
  - offsets: `0x0000` to `0x1FFF` (8192 bytes total)
  - chunk size: `256` bytes
- Validation:
  - `make verify` summary: `total=93 pass=92 fail=0 skip=1`
  - whole-binary coverage moved from `4.80%` to `15.12%`

## Coverage Expansion Batch 2

- Added new matched mid-file coverage band:
  - units: `unit_0093` through `unit_0156`
  - offsets: `0x2000` to `0x5FFF` (16384 bytes total)
  - chunk size: `256` bytes
- Validation:
  - `make verify` summary: `total=157 pass=156 fail=0 skip=1`
  - whole-binary coverage moved from `15.12%` to `35.78%`

## Coverage Expansion Batch 3

- Added next matched upper-mid coverage band:
  - units: `unit_0157` through `unit_0220`
  - offsets: `0x6000` to `0x9FFF` (16384 bytes total)
  - chunk size: `256` bytes
- Validation:
  - `make verify` summary: `total=221 pass=220 fail=0 skip=1`
  - whole-binary coverage moved from `35.78%` to `56.43%`

## Coverage Expansion Batch 4

- Added next matched high-mid coverage band:
  - units: `unit_0221` through `unit_0284`
  - offsets: `0xA000` to `0xDFFF` (16384 bytes total)
  - chunk size: `256` bytes
- Validation:
  - `make verify` summary: `total=285 pass=284 fail=0 skip=1`
  - whole-binary coverage moved from `56.43%` to `77.09%`

## Coverage Expansion Batch 5

- Added next matched late-file coverage band:
  - units: `unit_0285` through `unit_0348`
  - offsets: `0xE000` to `0x11FFF` (16384 bytes total)
  - chunk size: `256` bytes
- Validation:
  - `make verify` summary: `total=349 pass=348 fail=0 skip=1`
  - whole-binary coverage moved from `77.09%` to `97.74%`

## Coverage Expansion Batch 6 (Final Gap Closure)

- Closed the final uncovered pre-entry gap:
  - units: `unit_0349` through `unit_0355`
  - offsets: `0x12000` to `0x126FD` (1790 bytes total)
  - layout: six `256`-byte units plus one `254`-byte tail unit
- Validation:
  - `make verify` summary: `total=356 pass=355 fail=0 skip=1`
  - whole-binary coverage moved from `97.74%` to `100.00%`

## Token Trace Correlation Milestone

- Added event-level trace replay tool:
  - `tools/entry_unpacker_trace.c`
  - emits CSV rows with event type, source cursor movement, output growth,
    token/literal fields, and copy parameters
- Added trace window summarizer:
  - `tools/entry_unpacker_trace_windows.py`
  - bins trace events by source-offset windows to correlate with disassembly
    windows (`0x1274F`-`0x12845` focus)
- Added Make targets:
  - `make unpacker-trace`
  - `make unpacker-trace-windows`
- Added correlation note:
  - `docs/disasm/entry_unpacker_trace_windows_100.md`
- Latest trace run:
  - `status=ok`, `events_written=100`, `src_used=158`, `dst_written=720`
  - first 100 events concentrate in windows `0x1274F`-`0x127EE`
  - strongest output-expansion windows in this sample:
    - `0x1278F`-`0x1279E`
    - `0x1276F`-`0x1277E`

## Block-Level Correlation Milestone

- Added provisional unpacker block map:
  - `config/entry_unpacker_blocks_1274f_12845.json`
  - companion notes: `docs/disasm/entry_unpacker_block_hypotheses.md`
- Added trace-to-block correlator:
  - `tools/entry_unpacker_trace_blocks.py`
  - Make target: `make unpacker-trace-blocks`
- Added first block-correlation run notes:
  - `docs/disasm/entry_unpacker_trace_blocks_100.md`
- Latest block mapping result (`first 100 events @ 0x1274F`):
  - mapped events: `100`, unmapped: `0`
  - highest output-growth blocks: `B04_short_len_build`, `B05_short_backref_setup`
  - high-activity control block: `B09_window_slide` (`25` mapped events)

## Engine Structure Lift (Stage Helpers)

- Refactored unpacker engine dispatch loop into named stage helpers in:
  - `logic/entry_unpacker_engine.c`
- New helper boundaries mirror provisional block families:
  - `decode_literal_path()` (literal emit path)
  - `decode_second_gate()` (second-stage branch dispatcher)
  - `decode_long_path()` (long-token/extension control path)
- Intent:
  - preserve byte-for-byte behavior while making later readable-source lift
    easier to align with block hypotheses and trace reports.
- Post-refactor validation:
  - `make verify`: `total=356 pass=355 fail=0 skip=1`
  - `make progress`: `79322/79322 (100.00%)`
  - `make unpacker-fixtures`: `total=20 fail=0`
  - `make unpacker-parity`: `pairs=10 fail=0`
  - `make unpacker-trace-blocks`: trace run still `status=ok` with
    `events_written=100`

## Engine Structure Lift (Copy-Control Split)

- Further split copy/control semantics in `logic/entry_unpacker_engine.c`:
  - `run_copy_loop_backref()` (shared copy loop accounting, `B07` intent)
  - `decode_long_token_header()` (long token displacement/len-low decode, `B06`)
  - `resolve_long_copy_length()` (extension/control marker handling, `B08`)
  - `preseed_strict_window_history()` (strict prehistory seeding, `B09` intent)
- `decode_long_token()` and `decode_short_token()` now delegate shared copy
  accounting through one helper while preserving fail-code behavior.
- Validation after split:
  - `make verify`: `total=356 pass=355 fail=0 skip=1`
  - `make progress`: `79322/79322 (100.00%)`
  - `make unpacker-fixtures`: `total=20 fail=0`
  - `make unpacker-parity`: `pairs=10 fail=0`
  - `make unpacker-trace-windows`: trace still `status=ok`, `events_written=100`
  - `make unpacker-trace-blocks`: `mapped events=100`, `unmapped events=0`

## Readable Model Equivalence Milestone

- Added block-aligned readable unpacker implementation:
  - `logic/entry_unpacker_readable.h`
  - `logic/entry_unpacker_readable.c`
- Added readable replay harness:
  - `tools/entry_unpacker_replay_readable.c`
- Added fixture-level equivalence checker:
  - `tools/check_unpacker_readable_equivalence.py`
  - Make target: `make unpacker-readable-equivalence`
- Equivalence status:
  - `Readable equivalence summary: total=20 fail=0`
  - Reference and readable models match on status, src/dst counters, fail code,
    and output SHA256 for all fixtures.

## Readable Stats Comparison Milestone

- Added readable/reference stats report tool:
  - `tools/report_unpacker_readable_stats.py`
  - Make target: `make unpacker-readable-stats`
  - report output: `build/unpacker_readable_stats_report.md`
- Latest report summary:
  - `fixtures=20`
  - `mismatched fixtures=0`
  - `maximum absolute counter delta=0`
- This confirms operation-level parity (literal/short/long op counts and
  copied bytes), not only output parity.

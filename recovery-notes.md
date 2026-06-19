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

## Loader Regression Milestone

- Added public loader header:
  - `logic/entry_loader_model.h`
- Added deterministic loader fixture runner:
  - `tools/entry_loader_fixture.c`
- Added loader fixture checker and config:
  - `tools/check_entry_loader_fixtures.py`
  - `config/entry_loader_fixtures.json`
- Added Make target:
  - `make loader-fixtures`
- Intent:
  - protect the inferred stage-0 relocate copy and stage-1 window-slide logic
    while segment arithmetic is still being hardened
  - keep overlap and chunked-copy behavior stable during further readable lift

## Composed Bootstrap Replay Milestone

- Added loader bootstrap planning helper:
  - `otrail_entry_loader_plan_bootstrap()` in `logic/entry_loader_model.c`
- Added composed bootstrap replay tool:
  - `tools/entry_bootstrap_replay.c`
  - `make entry-bootstrap-replay`
- Added deterministic end-to-end bootstrap fixtures:
  - `config/entry_bootstrap_fixtures.json`
  - `tools/check_entry_bootstrap_fixtures.py`
  - `make entry-bootstrap-fixtures`
- Derived constants from the real EXE loader preamble:
  - stage-1 total paragraphs: `0x126D`
  - stage-0 relocation delta: `0x1116`
  - stage-0 relocation byte count: `0x0EEA`
- Current normalized composed replay (`load_seg=0xA000`) reaches:
  - relocated entry: `CS:IP = 0xC383:0x002B`
  - unpacker source: `DS = 0xB116`
  - unpacker output segment: `ES = 0xA000`
  - stage-1 pass schedule: `0x1000`, `0x026D`
- Current end-to-end validation:
  - strict bootstrap replay: `src_used=3245`, `dst_written=12658`
  - heuristic bootstrap replay: same output and SHA-256
  - readable unpacker equivalence on the composed bootstrap path: `total=2 fail=0`

## Bootstrap Logic Refactor Milestone

- Added reusable bootstrap module:
  - `logic/entry_bootstrap.h`
  - `logic/entry_bootstrap.c`
- Scope:
  - MZ header parse for entrypoint math
  - normalized image load into simulated DOS memory
  - loader relocation/segment-plan execution
  - handoff into inferred or readable unpacker models
- Follow-up cleanup:
  - loader bootstrap execution now lives in `logic/entry_loader_model.c`
    via `otrail_entry_loader_execute_bootstrap()`
  - `logic/entry_bootstrap.c` now orchestrates around shared loader logic
- `tools/entry_bootstrap_replay.c` now calls the shared logic module instead of
  duplicating the bootstrap simulation inline.
  - `make unpacker-trace-windows`: trace still `status=ok`, `events_written=100`
  - `make unpacker-trace-blocks`: `mapped events=100`, `unmapped events=0`

## Readable Loader Milestone

- Added readable loader implementation:
  - `logic/entry_loader_readable.h`
  - `logic/entry_loader_readable.c`
- Scope:
  - readable stage-0 backward relocation copy
  - readable stage-1 backward word-window slide
  - readable bootstrap plan construction and execution helpers
- Added loader readable-equivalence checker:
  - `tools/check_entry_loader_readable_equivalence.py`
  - Make target: `make loader-readable-equivalence`
- Updated composed bootstrap logic:
  - `OTRAIL_ENTRY_BOOTSTRAP_READABLE` now uses readable loader relocation and
    readable unpacking, not just readable unpacking on top of the inferred
    loader
- Current validation:
  - `make loader-fixtures`: `total=3 fail=0`
  - `make loader-readable-equivalence`: `total=3 fail=0`
  - `make entry-bootstrap-readable-equivalence`: `total=2 fail=0`

## First Semantic Source Units

- Replaced placeholder entrypoint stubs with semantic lift source in:
  - `src/unit_0001_entrypoint_64.c`
  - `src/unit_0002_entrypoint_next_64.c`
- Scope:
  - `unit_0001`: initial self-relocation copy plus first stage-1 pass setup
  - `unit_0002`: stage-1 completion, loader handoff, and first unpacker seed
- Marked both units in `config/functions.json` with:
  - `"source_state": "semantic_lift"`
- `tools/match_progress.py` now reports explicit source-state tags so semantic
  lift progress is visible even before authored rebuild artifacts exist.
- Current status:
  - `make progress`: `semantic_lift: 2 units / 128 bytes`
  - source inventory: `placeholder-tagged=354`, `non-placeholder=2`
  - baseline verification remains green

## Early Unpacker Slice Lift

- Replaced placeholder unpacker-adjacent stubs with semantic lift source in:
  - `src/unit_0003_entrypoint_contig_64.c`
  - `src/unit_0004_entrypoint_contig_64.c`
- Scope:
  - `unit_0003`: resumes non-literal token decode, covering short backref
    length/displacement completion plus long-token header/control-byte decode
  - `unit_0004`: resolves long-token control markers and models the
    high-confidence `0x2000`-window slide path
- Metadata:
  - `unit_0003` tagged as `"source_state": "semantic_lift"`
  - `unit_0004` tagged as `"source_state": "semantic_lift_partial"` because
    bytes after `0x127E7` still sit in the current low-confidence tail
- Validation:
  - `cc -std=c11 -Wall -Wextra -fsyntax-only src/unit_0001_entrypoint_64.c src/unit_0002_entrypoint_next_64.c src/unit_0003_entrypoint_contig_64.c src/unit_0004_entrypoint_contig_64.c`
  - `make verify-summary MATCH_FILTER='unit_000[1-4]*'`: `total=4 pass=4 fail=0 skip=0`
  - `make progress`: `semantic_lift=3 units / 192 bytes`, `semantic_lift_partial=1 unit / 64 bytes`
  - source inventory: `placeholder-tagged=352`, `non-placeholder=4`

## Tail Re-decode and Post-Slide Lift

- Re-decoded the ambiguous tail from raw bytes starting at `0x127EC` instead of
  trusting the plain linear pass through `0x127E7`.
- Result:
  - `0x127EC-0x12845` now reads as coherent executable logic
  - terminal sequence is:
    - `push cs ; pop ds`
    - `mov si,0x0158`
    - control loop at `0x127F9`
    - stack/segment handoff and far jump at `0x1283F-0x12845`
  - the likely code/data break for this chain starts at `0x12848`
- Replaced placeholders with partial semantic lift source in:
  - `src/unit_0005_entrypoint_contig_64.c`
  - `src/unit_0006_entrypoint_contig_64.c`
- Scope:
  - `unit_0005`: post-control-byte run-length accumulation, `DX`/`ES` segment
    updates, zero/non-zero `lodsw` control cases, and terminal-transfer prep
  - `unit_0006`: terminal handoff slice covering `cli`, `mov ss,si`,
    `mov sp,di`, `sti`, and far jump intent
- Metadata:
  - both units tagged as `"source_state": "semantic_lift_partial"`
  - `unit_0006` remains partial because its configured byte window begins on the
    second byte of the preceding `xor bx,bx`
- Validation:
  - `cc -std=c11 -Wall -Wextra -fsyntax-only src/unit_0001_entrypoint_64.c src/unit_0002_entrypoint_next_64.c src/unit_0003_entrypoint_contig_64.c src/unit_0004_entrypoint_contig_64.c src/unit_0005_entrypoint_contig_64.c src/unit_0006_entrypoint_contig_64.c`
  - `make verify-summary MATCH_FILTER='unit_000[1-6]*'`: `total=6 pass=6 fail=0 skip=0`
  - `make progress`: `semantic_lift=3 units / 192 bytes`, `semantic_lift_partial=3 units / 192 bytes`
  - source inventory: `placeholder-tagged=350`, `non-placeholder=6`

## First Authored Rebuild Units

- Added authored byte-array materializer:
  - `tools/materialize_authored_byte_arrays.py`
  - Make target: `make materialize-authored`
- Integrated authored byte-array materialization into the normal verify loop:
  - `make verify` / `make verify-summary` now materialize both baseline slices
    and source-authored byte-array units
- Converted the first validated data-region units to authored rebuilds:
  - `src/unit_0007_entrypoint_contig_64.c`
  - `src/unit_0008_entrypoint_contig_64.c`
  - `src/unit_0009_entrypoint_contig_64.c`
  - `src/unit_0010_entrypoint_contig_64.c`
- Metadata:
  - each tagged as `"source_state": "authored_data"`
  - each uses `"source_materializer": "c_byte_array"`
  - these units no longer use `"materialize_from_binary": true`
- Current status:
  - `make verify-summary MATCH_FILTER='unit_0007*'`: `total=1 pass=1 fail=0 skip=0`
  - `make verify-summary MATCH_FILTER='unit_0008*'`: `total=1 pass=1 fail=0 skip=0`
  - `make verify-summary MATCH_FILTER='unit_0009*'`: `total=1 pass=1 fail=0 skip=0`
  - `make verify-summary MATCH_FILTER='unit_0010*'`: `total=1 pass=1 fail=0 skip=0`
  - `make verify-summary`: `total=356 pass=355 fail=0 skip=1`
  - `make progress`: `authored rebuild units=4`, `authored rebuild bytes=256`

## Authored Data Batch Expansion

- Extended authored byte-array rebuild coverage through:
  - `src/unit_0011_entrypoint_contig_64.c`
  - `src/unit_0012_entrypoint_contig_64.c`
  - `src/unit_0013_entrypoint_contig_64.c`
  - `src/unit_0014_entrypoint_contig_64.c`
  - `src/unit_0015_entrypoint_contig_64.c`
  - `src/unit_0016_entrypoint_contig_64.c`
  - `src/unit_0017_entrypoint_contig_64.c`
  - `src/unit_0018_entrypoint_contig_64.c`
  - `src/unit_0019_entrypoint_contig_64.c`
  - `src/unit_0020_entrypoint_contig_64.c`
  - `src/unit_0021_entrypoint_contig_64.c`
  - `src/unit_0022_entrypoint_contig_64.c`
  - `src/unit_0023_entrypoint_contig_64.c`
  - `src/unit_0024_entrypoint_contig_64.c`
  - `src/unit_0025_entrypoint_contig_64.c`
  - `src/unit_0026_entrypoint_contig_64.c`
- These units are all tagged as:
  - `"source_state": "authored_data"`
  - `"source_materializer": "c_byte_array"`
- Validation:
  - `cc -std=c11 -Wall -Wextra -fsyntax-only src/unit_0011_entrypoint_contig_64.c src/unit_0012_entrypoint_contig_64.c src/unit_0013_entrypoint_contig_64.c src/unit_0014_entrypoint_contig_64.c src/unit_0015_entrypoint_contig_64.c src/unit_0016_entrypoint_contig_64.c src/unit_0017_entrypoint_contig_64.c src/unit_0018_entrypoint_contig_64.c src/unit_0019_entrypoint_contig_64.c src/unit_0020_entrypoint_contig_64.c src/unit_0021_entrypoint_contig_64.c src/unit_0022_entrypoint_contig_64.c src/unit_0023_entrypoint_contig_64.c src/unit_0024_entrypoint_contig_64.c src/unit_0025_entrypoint_contig_64.c src/unit_0026_entrypoint_contig_64.c`
  - `make verify-summary`: `total=356 pass=355 fail=0 skip=1`
  - `make progress`: `authored rebuild units=20`, `authored rebuild bytes=1280`
  - source inventory: `placeholder-tagged=330`, `non-placeholder=26`

## Authored Entry Tail Completion

- Extended authored byte-array rebuild coverage through the rest of the
  contiguous entry chain:
  - `src/unit_0027_entrypoint_contig_64.c` through
    `src/unit_0059_entrypoint_contig_64.c`
  - `src/unit_0060_entrypoint_tail_28.c`
- These units are all tagged as:
  - `"source_state": "authored_data"`
  - `"source_materializer": "c_byte_array"`
- Current status:
  - the contiguous entry chain is now fully non-placeholder through the final
    executable tail byte
  - `unit_0001` through `unit_0006` remain semantic lift source
  - `unit_0007` through `unit_0060` now rebuild from authored byte arrays
- Validation:
  - `cc -std=c11 -Wall -Wextra -fsyntax-only src/unit_0051_entrypoint_contig_64.c src/unit_0052_entrypoint_contig_64.c src/unit_0053_entrypoint_contig_64.c src/unit_0054_entrypoint_contig_64.c src/unit_0055_entrypoint_contig_64.c src/unit_0056_entrypoint_contig_64.c src/unit_0057_entrypoint_contig_64.c src/unit_0058_entrypoint_contig_64.c src/unit_0059_entrypoint_contig_64.c src/unit_0060_entrypoint_tail_28.c`
  - `make verify-summary`: `total=356 pass=355 fail=0 skip=1`
  - `make progress`: `authored rebuild units=54`, `authored rebuild bytes=3420`
  - source inventory: `placeholder-tagged=296`, `non-placeholder=60`

## Rebuildable Semantic Entry Slices

- Embedded exact authored `_bytes[]` arrays into the readable semantic source
  files:
  - `src/unit_0001_entrypoint_64.c`
  - `src/unit_0002_entrypoint_next_64.c`
  - `src/unit_0003_entrypoint_contig_64.c`
  - `src/unit_0004_entrypoint_contig_64.c`
  - `src/unit_0005_entrypoint_contig_64.c`
  - `src/unit_0006_entrypoint_contig_64.c`
- Metadata:
  - semantic `source_state` tags are unchanged
  - each unit now also uses `"source_materializer": "c_byte_array"`
  - these units no longer use `"materialize_from_binary": true`
- Result:
  - the full `unit_0001` through `unit_0060` entry chain now rebuilds from
    authored source files
  - readable semantic lift and rebuild verification now live in the same first
    six source files
- Validation:
  - `cc -std=c11 -Wall -Wextra -fsyntax-only src/unit_0001_entrypoint_64.c src/unit_0002_entrypoint_next_64.c src/unit_0003_entrypoint_contig_64.c src/unit_0004_entrypoint_contig_64.c src/unit_0005_entrypoint_contig_64.c src/unit_0006_entrypoint_contig_64.c`
  - `make verify-summary`: `total=356 pass=355 fail=0 skip=1`
  - `make progress`: `authored rebuild units=60`, `authored rebuild bytes=3804`

## First MZ Band Harvest Batch

- Converted the first eight pre-entry MZ bands into authored byte-array rebuild
  units:
  - `src/unit_0061_mz_band_256.c`
  - `src/unit_0062_mz_band_256.c`
  - `src/unit_0063_mz_band_256.c`
  - `src/unit_0064_mz_band_256.c`
  - `src/unit_0065_mz_band_256.c`
  - `src/unit_0066_mz_band_256.c`
  - `src/unit_0067_mz_band_256.c`
  - `src/unit_0068_mz_band_256.c`
- Metadata:
  - each tagged as `"source_state": "authored_data"`
  - each uses `"source_materializer": "c_byte_array"`
  - these units no longer use `"materialize_from_binary": true`
- Validation:
  - `cc -std=c11 -Wall -Wextra -fsyntax-only src/unit_0061_mz_band_256.c src/unit_0062_mz_band_256.c src/unit_0063_mz_band_256.c src/unit_0064_mz_band_256.c src/unit_0065_mz_band_256.c src/unit_0066_mz_band_256.c src/unit_0067_mz_band_256.c src/unit_0068_mz_band_256.c`
  - `make verify-summary`: `total=356 pass=355 fail=0 skip=1`
  - `make progress`: `authored rebuild units=68`, `authored rebuild bytes=5852`

## Second MZ Band Harvest Batch

- Converted the next eight pre-entry MZ bands into authored byte-array rebuild
  units:
  - `src/unit_0069_mz_band_256.c`
  - `src/unit_0070_mz_band_256.c`
  - `src/unit_0071_mz_band_256.c`
  - `src/unit_0072_mz_band_256.c`
  - `src/unit_0073_mz_band_256.c`
  - `src/unit_0074_mz_band_256.c`
  - `src/unit_0075_mz_band_256.c`
  - `src/unit_0076_mz_band_256.c`
- Metadata:
  - each tagged as `"source_state": "authored_data"`
  - each uses `"source_materializer": "c_byte_array"`
  - these units no longer use `"materialize_from_binary": true`
- Validation:
  - `cc -std=c11 -Wall -Wextra -fsyntax-only src/unit_0069_mz_band_256.c src/unit_0070_mz_band_256.c src/unit_0071_mz_band_256.c src/unit_0072_mz_band_256.c src/unit_0073_mz_band_256.c src/unit_0074_mz_band_256.c src/unit_0075_mz_band_256.c src/unit_0076_mz_band_256.c`
  - `make verify-summary`: `total=356 pass=355 fail=0 skip=1`
  - `make progress`: `authored rebuild units=76`, `authored rebuild bytes=7900`

## Bulk Promotion Tooling

- Added a reusable promotion tool:
  - `tools/promote_baseline_units_to_authored.py`
  - Make target: `make promote-baseline MATCH_FILTER='...'`
- Purpose:
  - read baseline-matched unit bytes directly from the configured binary
  - rewrite placeholder `src/*.c` units into authored `const uint8_t ...[]`
    arrays
  - switch metadata from `"materialize_from_binary": true` to
    `"source_materializer": "c_byte_array"`
- This removes the need to hand-edit each authored-data batch.

## Third Pre-entry Harvest Batch

- Used the promotion tool to convert the next eight pre-entry bands:
  - `src/unit_0077_mz_band_256.c`
  - `src/unit_0078_mz_band_256.c`
  - `src/unit_0079_mz_band_256.c`
  - `src/unit_0080_mz_band_256.c`
  - `src/unit_0081_mz_band_256.c`
  - `src/unit_0082_mz_band_256.c`
  - `src/unit_0083_mz_band_256.c`
  - `src/unit_0084_mz_band_256.c`
- Validation:
  - `cc -std=c11 -Wall -Wextra -fsyntax-only src/unit_0077_mz_band_256.c src/unit_0078_mz_band_256.c src/unit_0079_mz_band_256.c src/unit_0080_mz_band_256.c src/unit_0081_mz_band_256.c src/unit_0082_mz_band_256.c src/unit_0083_mz_band_256.c src/unit_0084_mz_band_256.c`
  - `make verify-summary`: `total=356 pass=355 fail=0 skip=1`
  - `make progress`: `authored rebuild units=84`, `authored rebuild bytes=9948`

## Bulk Promotion Expansion

- Used `tools/promote_baseline_units_to_authored.py` to promote a larger
  contiguous authored-data block without hand-editing per-unit arrays.
- Converted the remaining pre-entry bands:
  - `src/unit_0085_mz_band_256.c`
  - `src/unit_0086_mz_band_256.c`
  - `src/unit_0087_mz_band_256.c`
  - `src/unit_0088_mz_band_256.c`
  - `src/unit_0089_mz_band_256.c`
  - `src/unit_0090_mz_band_256.c`
  - `src/unit_0091_mz_band_256.c`
  - `src/unit_0092_mz_band_256.c`
- Converted the first mid-file authored coverage bands:
  - `src/unit_0093_mid_band_256.c`
  - `src/unit_0094_mid_band_256.c`
  - `src/unit_0095_mid_band_256.c`
  - `src/unit_0096_mid_band_256.c`
  - `src/unit_0097_mid_band_256.c`
  - `src/unit_0098_mid_band_256.c`
  - `src/unit_0099_mid_band_256.c`
  - `src/unit_0100_mid_band_256.c`
  - `src/unit_0101_mid_band_256.c`
  - `src/unit_0102_mid_band_256.c`
  - `src/unit_0103_mid_band_256.c`
  - `src/unit_0104_mid_band_256.c`
  - `src/unit_0105_mid_band_256.c`
  - `src/unit_0106_mid_band_256.c`
  - `src/unit_0107_mid_band_256.c`
  - `src/unit_0108_mid_band_256.c`
  - `src/unit_0109_mid_band_256.c`
  - `src/unit_0110_mid_band_256.c`
  - `src/unit_0111_mid_band_256.c`
  - `src/unit_0112_mid_band_256.c`
  - `src/unit_0113_mid_band_256.c`
  - `src/unit_0114_mid_band_256.c`
  - `src/unit_0115_mid_band_256.c`
  - `src/unit_0116_mid_band_256.c`
- Validation:
  - `make verify-summary`: `total=356 pass=355 fail=0 skip=1`
  - `make progress`: `authored rebuild units=116`, `authored rebuild bytes=18140`

## Large Mid-band Expansion

- Continued using the promotion tool over larger contiguous blocks:
  - `src/unit_0133_mid_band_256.c` through
    `src/unit_0156_mid_band_256.c`
  - `src/unit_0157_upper_mid_band_256.c` through
    `src/unit_0176_upper_mid_band_256.c`
- Minor tooling improvement:
  - `tools/promote_baseline_units_to_authored.py` now labels `upper_mid_band`
    units distinctly in generated source comments
- Validation:
  - `make verify-summary`: `total=356 pass=355 fail=0 skip=1`
  - `make progress`: `authored rebuild units=160`, `authored rebuild bytes=29404`

## Full Authored Reconstruction Coverage

- Finished the bulk promotion pass over every remaining baseline-backed unit:
  - `src/unit_0117_mid_band_256.c` through
    `src/unit_0132_mid_band_256.c`
  - `src/unit_0177_upper_mid_band_256.c` through
    `src/unit_0220_upper_mid_band_256.c`
  - `src/unit_0221_high_mid_band_256.c` through
    `src/unit_0284_high_mid_band_256.c`
  - `src/unit_0285_late_band_256.c` through
    `src/unit_0348_late_band_256.c`
  - `src/unit_0349_final_gap_256.c` through
    `src/unit_0355_final_gap_tail_254.c`
- Tooling/reporting improvement:
  - `tools/match_progress.py` now labels zero-denominator baseline origin
    counts as `retired` rather than rendering a misleading `0/0 (0.00%)`
- Validation:
  - `make verify-summary`: `total=356 pass=355 fail=0 skip=1`
  - `make progress`:
    - `baseline materialized units=0/0 (retired)`
    - `authored rebuild units=355/355 (100.00%)`
    - `authored rebuild bytes=79322/79322 (100.00%)`
    - `source_state tags`: `authored_data=349 units / 78938 bytes`,
      `semantic_lift=3 units / 192 bytes`,
      `semantic_lift_partial=3 units / 192 bytes`
- Outcome:
  - every matched `OREGON.EXE` unit now rebuilds from authored C source files
  - remaining work is semantic: replacing byte-array lifts with readable,
    authoritative decompiled code

## Exact Loader Pass Trace

- Tightened the loader semantic lift around `0x1271B-0x1274D` by recording the
  exact stage-1 register progression per pass in `LoaderBootstrapPass`:
  - remaining paragraphs before/after (`BP`)
  - source/destination segment progression (`DX`/`BX`)
  - exact word counts (`CX`)
  - exact backward-copy tail offsets (`SI`/`DI`)
- `src/unit_0001_entrypoint_64.c` now consumes those exact trace values instead
  of recomputing the first-pass tail loosely.
- `tools/entry_bootstrap_replay.c` now emits a `pass_trace=` line, and the real
  EXE bootstrap fixtures lock the expected trace:
  - `BP=126D->026D DX=B26D->A26D BX=C383->B383 CX=8000 SI=DI=FFFE`
  - `BP=026D->0000 DX=A26D->A000 BX=B383->B116 CX=1368 SI=DI=26CE`
- Validation:
  - `make entry-bootstrap-fixtures`
  - `make entry-bootstrap-readable-equivalence`
  - `make loader-readable-equivalence`

## Exact Loader-to-Unpacker Handoff

- Added a reusable pre-loader-prep helper in `logic/entry_bootstrap.c` so the
  real EXE can be loaded and planned through the loader without immediately
  running the unpacker.
- Added public declarations for the lifted `unit_0002` handoff model:
  - `src/unit_0002_entrypoint_next_64.h`
- Added a deterministic handoff fixture runner:
  - `tools/entry_handoff_fixture.c`
  - `tools/check_entry_handoff_fixtures.py`
  - `config/entry_handoff_fixtures.json`
  - `make entry-handoff-fixtures`
- Locked the exact initial unpacker state reached from the real EXE:
  - `DS=0xB116`
  - `ES=0xA000`
  - `stream_src_offset=2`
  - `stream_dst_offset=0`
  - `seed_word=0xFFFF`
  - `seed_bits=0x7FFF`
  - `seed_bits_left=15`
  - `first_gate_is_literal=1`
- Validation:
  - `make entry-handoff-fixtures`

## Early Prepared-Stream Token Trace

- Added a reusable stream dump at the exact `unit_0002` handoff point:
  - `tools/entry_handoff_dump_stream.c`
  - Make target: `make entry-handoff-trace`
- Reused the existing unpacker trace runner on that prepared `DS:0` stream and
  added a dedicated fixture checker:
  - `tools/check_entry_handoff_trace.py`
  - `config/entry_handoff_trace_fixtures.json`
  - Make target: `make entry-handoff-trace-fixtures`
- Locked the first `31` heuristic prepared-stream events from the real EXE:
  - initial literal burst:
    `67 59 6F 75 20 6D 61 79 3A 5C 5C 20 20 31 2E FF 54 72`
  - first short-path tokens:
    `disp=-159 len=2`, `disp=-138 len=2`, `disp=-155 len=3`,
    `disp=-148 len=2`, `disp=-152 len=3`, `disp=-155 len=2`
  - first long-path tokens:
    `token=0xE18C disp=-884 len=3`
    `token=0x654C disp=-5044 len=7`
- This gives the lifted `unit_0003` logic a real prepared-stream baseline for
  both short and long path shapes, instead of only static local reasoning.
- Validation:
  - `make entry-handoff-trace-fixtures`

## Direct Unit_0003 / Unit_0004 Token Fixtures

- Added public headers for the lifted token slices:
  - `src/unit_0003_entrypoint_contig_64.h`
  - `src/unit_0004_entrypoint_contig_64.h`
- Added a resumed-token fixture runner that starts from the real `unit_0002`
  handoff state instead of from raw-file offset guesses:
  - `tools/entry_token_fixture.c`
  - `tools/check_entry_token_fixtures.py`
  - `config/entry_token_fixtures.json`
  - `make entry-token-fixtures`
- Locked three direct lifted-token milestones from the real EXE handoff:
  - first resumed short token:
    `disp=-159 len=2`
  - first resumed long token:
    `disp=-3588 len=6`
  - first resumed long-control token:
    `disp=-4497 control_byte=252 -> copy_len=253`
- Added the first direct resumed window-slide control fixture:
  - `disp=-6920 control_byte=1 -> CONTROL_WINDOW_SLIDE`
- Extended the full prepared-stream fixture to lock the terminal successful
  heuristic end marker as well:
  - final event: `long_end token=0xB8EA ext=0 back_disp=-2070`
- The full successful heuristic trace contains one observed `ext=1` control
  marker and one terminal `ext=0` marker.
- This means `unit_0003` short/long decode and the `unit_0004` copy-control
  plus window-slide branches are now tested directly, with stream end locked at
  the full-trace level instead of left implicit.
- Validation:
  - `make entry-token-fixtures`
  - `make entry-handoff-trace-fixtures`

## Unpacked Payload + Tail Relocation Search

- Added a stable unpacked-payload report workflow:
  - `tools/report_unpacked_payload.py`
  - `make unpacked-payload-report`
- The report now freezes the readable bootstrap payload identity:
  - size: `12658` bytes
  - SHA-256:
    `230c5a49671bc90c63a1e3212dcf59532818d859f94c99de2bb24befe24f52e2`
- Reframed the packed tail at `0x127EC-0x12845` as an exact relocation stream
  plus terminal handoff, and wrote down the decode in:
  - `docs/disasm/entry_tail_relocation_model.md`
- Added a repeatable candidate search for the unresolved runtime `CS` mapping:
  - `tools/report_entry_tail_candidates.py`
  - `make entry-tail-candidates`
- The search uses the exact tail semantics under a 64K relocation-window model:
  - stream source at `CS:0x0158`
  - non-zero bytes relocate `[ES:DI]` by `BX`
  - `0x0000` control means `DX += 0x0FFF`
  - `0x0001` control means terminal handoff via header words at `CS:0x0000`
- Current result:
  - the tail model now terminates cleanly under 64K semantics instead of
    exploding linearly
  - the repo can now rank candidate runtime entrypoints inside the unpacked
    payload instead of guessing from arbitrary offsets
  - top candidates remain low confidence, which is expected because the exact
    `CS` placement and `BX` basis are still unresolved
- Added a wider family sweep over `BX=0x0000..0x0100`:
  - Make target: `make entry-tail-family-sweep`
  - report output: `build/entry_tail_family_report.md`
- Current strongest low-confidence families from that sweep:
  - `0x2500` with best representative `cs_base=0x0030`, `bx=0x0060`
  - `0x2F00` with best representatives `cs_base=0x0030`, `bx=0x0100` and
    `cs_base=0x0D60`, `bx=0x00F0`
  - `0x081B/0x0821/0x082C` cluster
  - `0x0F46` / nearby `0x0F61` control-heavy window
  - broad repeated families at `0x0800`, `0x0600`, and `0x1000`
- These are still not authoritative entrypoints, but they are a much better
  next-pass target set than the earlier single-seed report.
- Validation:
  - `make unpacked-payload-report`
  - `make entry-tail-candidates`
  - `make entry-tail-family-sweep`
  - `make verify-summary`

## Exact Unit_0005 / Unit_0006 Tail Fixtures

- Replaced the older generic `unit_0005` post-control model with exact
  relocation-tail semantics in:
  - `src/unit_0005_entrypoint_contig_64.h`
  - `src/unit_0005_entrypoint_contig_64.c`
- Added public terminal-handoff declarations and exact handoff finalization in:
  - `src/unit_0006_entrypoint_contig_64.h`
  - `src/unit_0006_entrypoint_contig_64.c`
- Added dedicated tail fixture coverage:
  - `tools/entry_tail_fixture.c`
  - `tools/check_entry_tail_fixtures.py`
  - `config/entry_tail_fixtures.json`
  - Make target: `make entry-tail-fixtures`
- Locked four exact semantics:
  - direct non-zero patch byte dispatch
  - `0x0000` segment-bump control
  - non-terminal control-word low-byte reuse
  - terminal handoff relocation of `SS`, `SP`, and far-jump segment
- Metadata update:
  - `unit_0005` is now tagged as `"source_state": "semantic_lift"`
  - `unit_0006` remains `"semantic_lift_partial"` because the configured slice
    still begins on the second byte of `xor bx,bx` and runs into data after
    `0x12848`
- Validation:
  - `cc -std=c11 -Wall -Wextra -fsyntax-only src/unit_0005_entrypoint_contig_64.c src/unit_0006_entrypoint_contig_64.c tools/entry_tail_fixture.c`
  - `make entry-tail-fixtures`
  - `make progress`: `semantic_lift=4 units / 256 bytes`, `semantic_lift_partial=2 units / 128 bytes`

## Exact Unit_0004 Tail Bootstrap Correction

- Promoted the `unit_0004` long-control stream-end path from “jump to low-confidence tail” to an exact lift of the `0x127EC` bootstrap:
  - `src/unit_0004_entrypoint_contig_64.h`
  - `src/unit_0004_entrypoint_contig_64.c`
- Added exact bootstrap fixture coverage on top of the existing tail loop/handoff checks:
  - `tools/entry_tail_fixture.c`
  - `tools/check_entry_tail_fixtures.py`
  - `config/entry_tail_fixtures.json`
- Corrected the candidate search so it now runs through the real bootstrap before `unit_0005`:
  - `tools/entry_tail_search.c`
  - `tools/report_entry_tail_candidates.py`
  - `Makefile`
- Exact bootstrap semantics now covered:
  - derive the post-`add bx,0x10` seed from the popped value
  - seed `DX=BX`, clear `DI`, and consume the first relocation-stream byte at `0x0158`
  - dispatch the very first non-zero byte / `0x0000` segment bump / `0x0001` terminal case through the same lifted `unit_0005` / `unit_0006` path
- Metadata update:
  - `unit_0004` is now tagged as `"source_state": "semantic_lift"`
- Corrected current family sweep:
  - strongest families now center on `0x0820/0x081F/0x082C`, `0x2F00/0x2EF0`, `0x0F46/0x0F75`, and repeated pages at `0x0800/0x0600/0x1000`
- Validation:
  - `make entry-tail-fixtures`
  - `make entry-token-fixtures`
  - `make entry-tail-candidates`
  - `make entry-tail-family-sweep`
  - `make progress`: `semantic_lift=5 units / 320 bytes`, `semantic_lift_partial=1 unit / 64 bytes`

## Unpacked Window Provenance Reports

- Added reusable unpacked-window reporting:
  - `tools/report_unpacked_window.py`
  - Make target: `make unpacked-window-report WINDOW_START=... WINDOW_SIZE=...`
- Added reusable batch comparison:
  - `tools/report_unpacked_window_sweep.py`
  - Make target: `make unpacked-window-sweep`
- Added frozen provenance fixtures:
  - `tools/check_unpacked_window_fixtures.py`
  - `config/unpacked_window_fixtures.json`
  - Make target: `make unpacked-window-fixtures`
- Added recursive contributor tracing:
  - `tools/report_unpacked_contributor_chain.py`
  - Make target: `make unpacked-contributor-chain`
- The report correlates:
  - exact payload bytes
  - linear 16-bit disassembly
  - overlapping real handoff trace events
  - backref source windows, including wrapped 16-bit sources
- First generated reports:
  - `build/unpacked_window_0820_report.md` for `0x0818..0x0850`
  - `build/unpacked_window_2f00_report.md` for `0x2F00..0x2F40`
- Current conclusion:
  - the `0x0820` family is much weaker than its raw tail-search score suggests
  - `0x0818..0x0850` contains only `9` non-zero bytes out of `56`
  - the apparent `jmp` / `ret` cluster is assembled from `16` events (`5`
    literals, `5` short backrefs, `6` long backrefs) and is mostly copied zero
    padding
  - the first sweep report now ranks:
    - `0x2000..0x2040` first (`score=106`)
    - `0x2F00..0x2F40` second (`score=93`)
    - `0x0600..0x0660` third (`score=71`)
    - `0x0F46..0x0F96` fourth (`score=64`)
    - `0x0818..0x0850` last (`score=-16`)
  - `0x2000` is now the best immediate lift target; it is the densest current
    candidate and has no wrapped-copy overlaps in the sampled window
  - focused notes for that target now live in:
    - `docs/disasm/unpacked_window_2000_notes.md`
    - `build/unpacked_contributor_chain_report.md`
  - dominant pinned contributors:
    - `0x03ED..0x03F6`
    - `0x070B..0x070F`
    - `0x0DC3..0x0DC6`
    - `0x1DCB..0x1DD1`
    - `0x1F87..0x1F8C`
  - the first chain report now narrows those into:
    - dense motif windows to resolve first:
      - `0x070B..0x070F`
      - `0x0DC3..0x0DC6`
    - zero-rich carrier windows:
      - `0x03ED..0x03F6`
      - `0x1DCB..0x1DD1`
      - `0x1F87..0x1F8C`
  - current interpretation:
    - `0x2000` is more likely a hybrid fragment assembled from repeated motifs
      and carrier padding than a clean standalone entry block
- Validation:
  - `python3 -m py_compile tools/report_unpacked_window.py`
  - `make unpacked-window-report WINDOW_START=0x0818 WINDOW_SIZE=0x38 UNPACKED_WINDOW_REPORT=build/unpacked_window_0820_report.md`
  - `make unpacked-window-report WINDOW_START=0x2F00 WINDOW_SIZE=0x40 UNPACKED_WINDOW_REPORT=build/unpacked_window_2f00_report.md`
  - `python3 -m py_compile tools/report_unpacked_window_sweep.py`
  - `make unpacked-window-sweep`
  - `python3 -m py_compile tools/check_unpacked_window_fixtures.py`
  - `make unpacked-window-fixtures`
  - `python3 -m py_compile tools/report_unpacked_contributor_chain.py`
  - `make unpacked-contributor-chain`

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

## Unpacked Motif Family Milestone

- Added motif-family report and fixture tooling:
  - `tools/report_unpacked_motif_family.py`
  - `tools/check_unpacked_motif_fixtures.py`
  - `config/unpacked_motif_fixtures.json`
  - Make targets:
    - `make unpacked-motif-family`
    - `make unpacked-motif-fixtures`
- New durable notes:
  - `docs/disasm/unpacked_motif_family_notes.md`
- Current fixed report:
  - `build/unpacked_motif_family_report.md`
- Main semantic correction from the new report:
  - the dense `0x070B..0x070F` motif inside the `0x2000` root is not a seed
  - the exact `C7 C3 00 FA` family starts earlier at `0x05FF`
  - that family now has `9` exact occurrences:
    - `0x05FF`
    - `0x070B`
    - `0x0FEA`
    - `0x1B84`
    - `0x2006`
    - `0x203A`
    - `0x2081`
    - `0x224F`
    - `0x2283`
- The `0x0DC3..0x0DC6` motif remains a valid seed:
  - bytes: `DB 17 75`
  - exact occurrences:
    - `0x0DC3`
    - `0x1D08`
    - `0x200A`
    - `0x2085`
    - `0x2253`
- Updated next-lift target:
  - resolve `0x05FF` and `0x0DC3` first
  - stop treating the requested `0x070B` copy as a family origin
- Validation:
  - `python3 -m py_compile tools/unpacked_window_analysis.py`
  - `python3 -m py_compile tools/report_unpacked_motif_family.py`
  - `python3 -m py_compile tools/check_unpacked_motif_fixtures.py`
  - `make unpacked-motif-family`
  - `make unpacked-motif-fixtures`

## Exact Unit_0006 Mixed Slice Milestone

- Extended the public `unit_0006` API so the configured slice is described
  exactly as mixed code/data rather than left in partial status:
  - `src/unit_0006_entrypoint_contig_64.h`
  - `src/unit_0006_entrypoint_contig_64.c`
- Added explicit exact decomposition:
  - carried-over `xor bx,bx` tail byte: `0xDB`
  - handoff code bytes: `FA 8E D6 8B E7 FB 2E FF 2F`
  - first post-jump data prefix: `54` exact bytes starting at `0x12848`
- Extended the existing tail fixture path instead of creating a parallel check:
  - `tools/entry_tail_fixture.c`
  - `tools/check_entry_tail_fixtures.py`
  - `config/entry_tail_fixtures.json`
- Metadata update:
  - `unit_0006` is now tagged as `"source_state": "semantic_lift"`
- Current honest state:
  - the first six entrypoint units are all exact semantic lift
  - `unit_0006` is still mixed, but it is no longer unresolved
- Validation:
  - `cc -std=c11 -Wall -Wextra -fsyntax-only src/unit_0006_entrypoint_contig_64.c tools/entry_tail_fixture.c`
  - `make entry-tail-fixtures`

## Readable Post-Unpack Runtime Fragment Milestone

- Added the first readable checked source for post-unpack runtime fragments:
  - `logic/unpacked_runtime_fragments.h`
  - `logic/unpacked_runtime_fragments.c`
- Added fixture coverage:
  - `tools/unpacked_runtime_fixture.c`
  - `tools/check_unpacked_runtime_fixtures.py`
  - `config/unpacked_runtime_fixtures.json`
  - Make target:
    - `make unpacked-runtime-fixtures`
- Current readable fragments:
  - seed `0x05FF`
    - `C7 C3 00 FA`
    - modeled as literal `0xC7`, copied pair `C3 00` from `0x0506`, literal `0xFA`
  - seed `0x0DC3`
    - `DB 17 75`
    - modeled as literals `0xDB`, `0x17`, then copied byte `0x75` from `0x0DA8`
  - aligned block `0x05F0..0x0610`
    - `00 00 00 00 00 FC F3 BF CA CF A3 54 15 EB 06 C7`
    - `C3 00 FA 0A 00 83 00 00 FA 01 74 10 7F 00 00 F9`
  - aligned block `0x0DC0..0x0DD0`
    - `00 00 D6 DB 17 75 00 00 00 FD 1C A3 A5 E3 06 00`
  - direct `0x05FF` family fan-out windows
    - `0x05E5..0x06AD` and exact clone `0x0FD0..0x1098`
    - `0x05F5..0x06D6` and exact clone `0x1B7A..0x1C5B`
    - modeled as structured source windows built from the checked `0x05F0` block plus fixed prefixes/suffixes rather than raw dumps
  - `0x0DC3` family window
    - `0x0D1A..0x0E05` and exact clone `0x1C5F..0x1D4A`
  - full `0x2000` root window
    - `30 E4 80 7D F6 4A C7 C3 00 FA DB 17 75 4A C7 C3`
    - `D4 68 72 00 00 F8 00 00 00 42 00 00 00 11 F0 EB`
    - `00 00 63 16 73 00 00 00 00 00 00 00 F7 A5 99 00`
    - `00 00 42 EE E7 F0 F8 7E CD BE C7 C3 00 FA DB CC`
    - modeled as upstream `0x1230` tail bytes, literals, two explicit carrier fragments, zero carriers, the two seed motifs, and two short self-copy spans
  - `0x2000`-derived copy family
    - enclosing source `0x1F6D..0x206C` and exact clone `0x21B6..0x22B5`
    - direct root slice `0x2003..0x2021` and exact clone `0x207E..0x209C`
    - modeled as checked slices and clones built around the exact `0x2000` root window
  - second dense post-root family
    - exact source `0x110E..0x112E` and exact clone `0x2E90..0x2EB0`
    - exact root `0x2F00..0x2F40`
    - enclosing windows `0x2EF0..0x2F40` and `0x2EC0..0x2F40`
    - modeled with an opener copied from the checked `0x2E90` clone, an explicit short self-copy in the middle of the root, and an exact `0x1120` source slice reused in the tail
  - continuation of that family
    - exact root continuation `0x2F40..0x2F80`
    - contiguous checked span `0x2F00..0x2F80`
    - modeled with fixed contributor fragments from `0x2BD6`, `0x1E1D`, `0x2D38`, and `0x2961`, plus a trailing `27`-byte copy from the checked `0x2EF0` window
  - visible long-copy tail continuation
    - exact `0x2F80..0x2FC0` continuation built from the checked `0x2EF0`, `0x2E90`, and `0x2F40` windows plus the head of the `0x253E..0x263C` tail
    - exact `0x2FC0..0x3000` continuation from the next `64` bytes of that same long-copy tail
    - sparse `0x3000..0x3040` and `0x3040..0x3080` zero-heavy continuation pages
    - terminal sparse tail `0x3080..0x30AA`
    - contiguous checked span `0x2F00..0x30AA`
  - dense `0x0F20/0x0F46` family
    - source envelope `0x0EFF..0x0FAB`
    - structured source neighborhood `0x0F20..0x0F96`
    - exact dense root `0x0F46..0x0F96`
    - exact envelope clone `0x2A6E..0x2B1A`
    - second-level source envelope `0x2A3A..0x2B1A`
    - second-level exact copy head `0x2C7F..0x2D3F`
    - exact clone neighborhood `0x2A8F..0x2B05`
    - exact clone root `0x2AB5..0x2B05`
    - near-clone root `0x2CFA..0x2D4A` with the first `69` bytes matching the `0x0F46` root exactly and an 11-byte local tail variant
- This is still runtime semantic recovery, not new original-EXE matched bytes, but it is now a broader checked runtime spine rather than a single promoted block.
- Validation:
  - `cc -std=c11 -Wall -Wextra -fsyntax-only logic/unpacked_runtime_fragments.c tools/unpacked_runtime_fixture.c`
  - `python3 -m py_compile tools/check_unpacked_runtime_fixtures.py`
  - `make unpacked-runtime-fixtures` -> `total=41 fail=0`
  - `make verify-summary` -> `total=356 pass=355 fail=0 skip=1`

## 2026-04-15 - Runtime Progress Map Made Explicit

- Added a checked post-unpack runtime progress map:
  - `config/unpacked_runtime_map.json` is now used by `make progress`
  - current coverage reports `44` fixture-backed regions
  - current checked runtime bytes report `5423` bytes including clone/overlap spans
  - current unique checked runtime payload coverage reports `3257/12658 (25.73%)`
- Added C descriptors for the dense `0x0F46` family:
  - `logic/unpacked_runtime_map.h`
  - `logic/unpacked_runtime_map.c`
  - descriptor count: `9` regions
- Added map drift checking:
  - `tools/check_unpacked_runtime_map.py`
  - `tools/unpacked_runtime_fixture.c map0f46`
  - `make unpacked-runtime-map`
- This does not change packed EXE semantic-lift bytes yet. It makes the real post-unpack recovery progress measurable and fixture-backed instead of hidden in narrative notes.

## 2026-04-15 - Runtime Tail Clone Expansion

- Added three exact-copy runtime tail regions found by scanning checked fragments against the unpacked payload:
  - `0x2552..0x2592` exact clone of `0x2FC0..0x3000`
  - `0x1909..0x1933` exact clone of `0x3080..0x30AA`
  - `0x2612..0x263C` exact clone of `0x3080..0x30AA`
- Added readable helper wrappers:
  - `otrail_unpacked_build_window_2552`
  - `otrail_unpacked_build_window_1909`
  - `otrail_unpacked_build_window_2612`
- Updated runtime map/fixture totals:
  - fixture-backed regions: `44`
  - checked bytes including clone/overlap spans: `5423`
  - unique checked runtime payload coverage: `3257/12658 (25.73%)`
- Validation:
  - `make unpacked-runtime-fixtures` -> `total=44 fail=0`
  - `make progress` -> post-unpack runtime recovery `3257/12658 (25.73%)`

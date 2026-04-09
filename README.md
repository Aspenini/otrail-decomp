# OTrailDecomp (MS-DOS Bootstrap)

This repository is a Phase 1 (matching-first) bootstrap for decompiling the MS-DOS release of The Oregon Trail.

## Target

- Binary: `Oregon_The_1990/OREGON.EXE`
- Metadata: `config/functions.json`
- Workflow guide: `Agent-Playbook.md`

## Repository Layout

- `config/` match metadata and target definitions
- `src/` recovery units for matching
- `tools/` verification and progress scripts
- `build/` generated artifacts
- `logic/` reserved for later readable-source migration

## Prerequisites

- macOS, Linux, or WSL shell
- Python 3.10+ (tested with `python3`)
- The target executable present at `Oregon_The_1990/OREGON.EXE`

## Bootstrap and Verification Commands

Run bootstrap checks:

```bash
make bootstrap
```

Run unit verification:

```bash
make verify
```

Materialize baseline candidate artifacts from original binary slices:

```bash
make materialize
```

Show configured and whole-binary progress:

```bash
make progress
```

Human-readable “what we’re doing now” lives in `config/progress_status.json` and is printed at the top of `make progress` when that file exists. Edit it anytime to reflect current work; optional flag: `python3 tools/match_progress.py --status /path/to/status.json`.

Run inferred unpacker candidate scan:

```bash
make unpacker-scan
```

Replay inferred unpacker from a single offset:

```bash
make unpacker-replay
```

Replay from a specific candidate offset:

```bash
make unpacker-replay REPLAY_OFFSET=0x12778
```

Rank replay offsets by output fingerprint:

```bash
make unpacker-fingerprint
```

Probe a confidence band around a center offset:

```bash
make unpacker-band CENTER_OFFSET=0x12778 BAND_RADIUS=64 REPLAY_MODE=1
```

Mode values:

- `REPLAY_MODE=0` strict (preseeded history window, fail on unknown backref)
- `REPLAY_MODE=1` heuristic (more permissive exploratory behavior)

Run deterministic unpacker regression fixtures:

```bash
make unpacker-fixtures
```

Current fixture coverage:

- 20 deterministic replay fixtures across top confidence-band offsets
- both strict (`mode=0`) and heuristic (`mode=1`) paths

Check strict/heuristic fixture parity:

```bash
make unpacker-parity
```

Check readable-model equivalence against the reference engine:

```bash
make unpacker-readable-equivalence
```

Generate readable-vs-reference operation stats report:

```bash
make unpacker-readable-stats
```

Emit unpacker token trace (CSV) for event-level correlation:

```bash
make unpacker-trace TRACE_OFFSET=0x1274F TRACE_MAX_EVENTS=100 TRACE_MODE=1
```

Summarize trace events by source-offset windows:

```bash
make unpacker-trace-windows TRACE_WINDOW_START=0x1274F TRACE_WINDOW_END=0x12845 TRACE_WINDOW_SIZE=0x10
```

Map trace events into provisional semantic blocks:

```bash
make unpacker-trace-blocks TRACE_BLOCKS_JSON=config/entry_unpacker_blocks_1274f_12845.json
```

Clean generated artifacts:

```bash
make clean
```

## How to Add the Next Contiguous Target

1. Add a new source stub under `src/`.
2. Add a corresponding unit entry in `config/functions.json` with:
   - `id`, `logical_name`, `binary_id`
   - `original_offset`, `size_bytes`
   - `source_path`, `rebuilt_path`
   - expected signature (`expected_sha256` or `expected_bytes_hex`)
3. Build your candidate artifact into the configured `rebuilt_path`.
4. Run `make verify` and iterate until it passes.
5. Run `make progress` and record updated coverage.

## Current Status

- Phase 1 byte-coverage baseline is complete (`79322/79322`, `100.00%`).
- Match verification is green with `total=356 pass=355 fail=0 skip=1`.
- Entrypoint unpacker and pre-unpack loader have inferred C lift scaffolding.
- Token-level unpacker tracing is available via `make unpacker-trace` and
  `make unpacker-trace-windows`.
- A block-aligned readable unpacker model is available and checked for fixture
  equivalence via `make unpacker-readable-equivalence`.

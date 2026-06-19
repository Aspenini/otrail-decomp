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
- `logic/` inferred and readable subsystem models used by the analysis tools

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

Run a quieter verification summary, optionally scoped to a subset:

```bash
make verify-summary
make verify-summary MATCH_FILTER='entrypoint_*'
```

Materialize baseline candidate artifacts from original binary slices:

```bash
make materialize
```

Materialize authored byte-array units from C source:

```bash
make materialize-authored
```

Promote baseline-matched units into authored byte-array source:

```bash
make promote-baseline MATCH_FILTER='unit_007[7-9]_mz_band_256'
python3 tools/promote_baseline_units_to_authored.py --config config/functions.json --match 'unit_007[7-9]_mz_band_256' --match 'unit_008[0-4]_mz_band_256'
```

Show configured and whole-binary progress:

```bash
make progress
```

Human-readable “what we’re doing now” lives in `config/progress_status.json` and is printed at the top of `make progress` when that file exists. Edit it anytime to reflect current work; optional flag: `python3 tools/match_progress.py --status /path/to/status.json`.

`make progress` now also separates baseline materialized coverage from authored rebuild coverage and reports placeholder-tagged source inventory.
It also reports explicit `source_state` tags, which are useful for tracking semantic lift work before a unit has a real rebuild path.
It now also reports checked post-unpack runtime recovery from `config/unpacked_runtime_map.json`, so runtime-fragment progress is visible separately from packed EXE `src/unit_*` semantic-lift bytes.

Run deterministic loader regression fixtures:

```bash
make loader-fixtures
```

Check readable loader equivalence against the inferred loader:

```bash
make loader-readable-equivalence
```

Check the lifted `unit_0002` loader-to-unpacker handoff against the real EXE:

```bash
make entry-handoff-fixtures
```

Check the first prepared-stream unpacker events reached from that handoff:

```bash
make entry-handoff-trace-fixtures
```

Check direct lifted-token fixtures for `unit_0003` / `unit_0004` against the
real resumed handoff state:

```bash
make entry-token-fixtures
```

Replay the composed entry bootstrap path (loader relocation + unpacker):

```bash
make entry-bootstrap-replay
```

Check deterministic end-to-end bootstrap fixtures:

```bash
make entry-bootstrap-fixtures
```

Check readable-model equivalence on the composed bootstrap path:

```bash
make entry-bootstrap-readable-equivalence
```

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
- The first six entrypoint units now have semantic source instead of plain
  placeholders, and `make progress` reports them via `source_state`.
- `unit_0004` is now exact semantic lift: its `control_byte=0` path is tied to
  the exact `0x127EC` relocation-tail bootstrap rather than a low-confidence
  mixed-tail placeholder. `unit_0006` is now also exact lift as a mixed slice:
  one carried-over `xor bx,bx` tail byte, the full terminal handoff, and the
  first exact post-`0x12848` data-prefix bytes are all fixture-checked.
- `0x127EC-0x12845` has now been re-decoded as coherent executable tail logic;
  the likely code/data split for this chain currently starts at `0x12848`.
- `unit_0001` through `unit_0355` now rebuild from authored source, so the
  repo has `355` authored rebuild units covering `79322` bytes (`100.00%` of
  the configured `OREGON.EXE` target).
- That `100%` authored-rebuild state means the EXE can now be reconstructed
  from source-controlled C files, but it does not mean the whole program has
  been converted into readable, maintainable decompiled logic.
- The first `60` entry-chain units are now non-placeholder source:
  `unit_0001` through `unit_0006` remain readable semantic lift and now also
  carry exact rebuild byte arrays, while `unit_0007` through `unit_0060` are
  authored-data rebuilds.
- `unit_0061` through `unit_0355` now cover the remaining matched regions as
  authored byte-array lifts across the MZ, mid, upper-mid, high-mid, late-band,
  and final-gap ranges.
- `tools/promote_baseline_units_to_authored.py` and `make promote-baseline`
  now provide the bulk path that completed the baseline-to-authored conversion
  for every matched slice in `OREGON.EXE`.
- Loader regression fixtures now cover overlap and chunked window-slide semantics.
- Composed bootstrap replay now derives real loader constants from the EXE and
  yields a stable `12658`-byte unpacked output in both strict and heuristic
  modes.
- Bootstrap fixtures now also lock the exact stage-1 loader register trace from
  the real EXE, including `BP`/`DX`/`BX` progression and the `SI=DI` tail
  offsets used by each backward `rep movsw` pass.
- The lifted `unit_0002` handoff now has its own deterministic real-EXE
  fixture, locking `DS=0xB116`, `ES=0xA000`, `seed_word=0xFFFF`,
  `seed_bits=0x7FFF`, and a literal first gate.
- The first `31` prepared-stream unpacker events are now fixture-checked from
  that handoff, locking the initial literal burst plus the first real short and
  long token shapes used by the lifted `unit_0003` path.
- Direct lifted-token fixtures now check the first resumed `unit_0003` short
  token, first resumed long token, and the first `unit_0004` copy-control long
  token from the real EXE handoff state.
- The resumed `unit_0004` window-slide control case is now fixture-checked
  directly (`control_byte=1`), and the full successful heuristic trace now
  locks the terminal `ext=0` stream-end marker.
- The exact packed tail at `0x127EC-0x12845` is now documented as a relocation
  stream and terminal handoff rather than a generic mixed tail; see
  `docs/disasm/entry_tail_relocation_model.md`.
- `make unpacked-payload-report` now gives a first structured report over the
  stable `12658`-byte readable bootstrap payload, and `make entry-tail-candidates`
  searches segment-aligned `CS` placements under the exact relocation-tail
  model to rank candidate runtime entrypoints inside that unpacked image.
- `make unpacked-window-report` now defaults to the current best provenance
  target, `0x2000..0x2040`, and `WINDOW_START` / `WINDOW_SIZE` can still be
  overridden for other regions. The report
  emits a byte/disassembly/provenance report for a chosen unpacked payload
  window by replaying the real handoff trace and correlating overlapping
  literal/short/long events.
- `make unpacked-window-sweep` now compares the current candidate set in one
  pass. On the current heuristic trace, it ranks `0x2000..0x2040` first,
  `0x2F00..0x2F40` second, `0x0600..0x0660` third, `0x0F46..0x0F96` fourth,
  and `0x0818..0x0850` last.
- `make unpacked-window-fixtures` now freezes the current summary metrics for
  the leading candidate windows (`0x2000`, `0x2F00`, and the demoted `0x0818`
  padding cluster) so the post-unpack triage state can regress cleanly.
- `make entry-tail-family-sweep` now runs through the exact `unit_0004`
  bootstrap before `unit_0005`, so the `BX` basis is interpreted as the
  post-`add bx,0x10` seed and the initial `DX=BX` state is no longer skipped.
  Under that corrected model, the strongest current low-confidence families are
  centered around unpacked offsets `0x0820/0x081F/0x082C`, `0x2F00/0x2EF0`,
  `0x0F46/0x0F75`, and the broader `0x0800/0x0600/0x1000` regions.
- The first provenance report over `0x0818..0x0850` shows that the `0x0820`
  cluster is mostly zero padding: `47/56` bytes are zero, and the apparent
  `jmp`/`ret` bytes are assembled from a small mix of literals plus copied
  backrefs. That makes `0x0820` a weaker direct-entry target than the family
  score alone suggests.
- The current provenance sweep puts `0x2000..0x2040` ahead of every other
  sampled region: it has the best density, the highest literal contribution of
  the top candidates, and no wrapped-copy overlaps. `0x2F00..0x2F40` remains
  the second-best follow-up, while `0x0F46` and `0x0600` are weaker.
- A focused note for the current top target now lives in
  `docs/disasm/unpacked_window_2000_notes.md`, including the dominant copy
  contributors (`0x03ED`, `0x070B`, `0x0DC3`, `0x1DCB`, `0x1F87`) that need to
  be resolved before the first real post-unpack lift.
- `make unpacked-contributor-chain` now traces those contributors recursively.
  The first chain report shows `0x070B` and `0x0DC3` are the dense motifs worth
  following first, while `0x03ED`, `0x1DCB`, and `0x1F87` are mostly zero-rich
  carrier windows.
- `make unpacked-motif-family` now freezes the next level of that work into a
  direct motif-family report. The key correction is that `0x070B..0x070F` is
  not a seed window at all: the exact `C7 C3 00 FA` family starts earlier at
  `0x05FF` and then fans out through nine exact occurrences, while
  `0x0DC3..0x0DC6` remains a real seed with five exact occurrences.
- `make unpacked-motif-fixtures` now checks those family facts directly so the
  next lift target is stable: `0x05FF` plus `0x0DC3`, not the derivative
  `0x070B` copy inside the `0x2000` root window.
- `make unpacked-runtime-fixtures` now checks the first readable post-unpack
  runtime source in `logic/unpacked_runtime_fragments.*`: the exact `0x05FF`
  seed, the exact `0x0DC3` seed, the aligned seed blocks at
  `0x05F0..0x0610` and `0x0DC0..0x0DD0`, the direct fan-out windows at
  `0x05E5..0x06AD`, `0x0FD0..0x1098`, `0x05F5..0x06D6`, `0x1B7A..0x1C5B`,
  the root-derived copy windows at `0x1F6D..0x206C`, `0x21B6..0x22B5`,
  `0x2003..0x2021`, and `0x207E..0x209C`, plus the second dense post-root
  family at `0x110E..0x112E`, `0x2E90..0x2EB0`, `0x2F00..0x2F40`,
  `0x2EF0..0x2F40`, `0x2EC0..0x2F40`, the exact `0x2F40..0x2F80`
  continuation, the exact `0x2F80..0x2FC0` and `0x2FC0..0x3000` windows, the
  sparse `0x3000..0x3040`, `0x3040..0x3080`, and `0x3080..0x30AA` tail
  windows, and the contiguous `0x2F00..0x30AA` span, plus a newly lifted
  `0x0EFF..0x0FAB` / `0x0F20..0x0F96` / `0x0F46..0x0F96` family with exact
  clone/envelope windows at `0x2A3A..0x2B1A`, `0x2A6E..0x2B1A`,
  `0x2A8F..0x2B05`, `0x2AB5..0x2B05`, and `0x2C7F..0x2D3F`, plus a near-clone
  tail variant at `0x2CFA..0x2D4A`, plus exact `0x2F00`-family tail clones at
  `0x2552..0x2592`, `0x1909..0x1933`, and `0x2612..0x263C`. The runtime
  fixture gate is now `44` checked cases, and `logic/unpacked_runtime_map.*`
  names the dense `0x0F46` family so the map can be checked against
  `config/unpacked_runtime_map.json`.
- `make entry-tail-fixtures` now directly checks the exact lifted relocation
  tail bootstrap in `unit_0004`, the relocation loop in `unit_0005`, and the
  mixed handoff/data slice in `unit_0006`. `unit_0004`, `unit_0005`, and
  `unit_0006` are now all exact semantic lift; `unit_0006` stays honest by
  modeling the carried-over `xor bx,bx` tail byte and the first post-jump data
  prefix explicitly instead of pretending the whole slice is linear code.
- Readable bootstrap mode now uses both the readable loader and readable
  unpacker, and matches the inferred bootstrap path.
- The composed bootstrap path now lives in reusable `logic/entry_bootstrap.*`
  code rather than only in the replay tool.
- Token-level unpacker tracing is available via `make unpacker-trace` and
  `make unpacker-trace-windows`.
- A block-aligned readable unpacker model is available and checked for fixture
  equivalence via `make unpacker-readable-equivalence`.

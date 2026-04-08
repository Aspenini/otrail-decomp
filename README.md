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

- Bootstrap scaffold is complete.
- One placeholder unit (`unit_0000_entry_stub`) is configured as `unmatched`.
- First real unit (`unit_0001_entrypoint_64`) is configured from the EXE entrypoint
  slice at offset `0x126FE` with `64` bytes.
- `make verify` now materializes baseline unit candidates before matching checks.

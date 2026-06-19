# Unpacked Motif Family Notes

Scope: dense motif families currently feeding the top post-unpack candidate
window at `0x2000..0x2040`.

## Current Fixed Outputs

- `build/unpacked_motif_family_report.md`
- `config/unpacked_motif_fixtures.json`
- `docs/disasm/unpacked_runtime_fragments.md`

## Family `0x070B..0x070F`

- bytes: `C7 C3 00 FA`
- exact occurrence count: `9`
- requested occurrence: `0x070B`
- earliest exact seed: `0x05FF`
- current interpretation:
  - `0x070B` is not a seed window
  - it is a downstream copy of an earlier motif family that already exists at
    `0x05FF`
  - the family later propagates through `0x0FEA`, `0x1B84`, `0x2006`,
    `0x203A`, `0x2081`, `0x224F`, and `0x2283`
- direct seed fan-out currently pinned by fixtures:
  - event `365` -> `0x06B0..0x07AD`
  - event `830` -> `0x0FD0..0x1098`
  - event `1309` -> `0x1B7A..0x1C5B`
  - event `1551` -> `0x232C..0x2335`

## Family `0x0DC3..0x0DC6`

- bytes: `DB 17 75`
- exact occurrence count: `5`
- requested occurrence: `0x0DC3`
- earliest exact seed: `0x0DC3`
- current interpretation:
  - this requested window is also the family seed
  - the seed is assembled from two literals plus a short copy
  - the family currently propagates through `0x1D08`, `0x200A`, `0x2085`, and
    `0x2253`
- direct seed fan-out currently pinned by fixtures:
  - event `1311` -> `0x1C5F..0x1D4A`
  - event `1410` -> `0x200A..0x200D`

## Why This Matters

- The `0x2000` root window should not be treated as a clean standalone entry
  block yet.
- One of its two dense motifs (`0x070B`) is itself derivative; lifting the
  requested `0x070B` copy would skip the earlier family origin at `0x05FF`.
- The most defensible next post-unpack semantic targets are now:
  1. `0x05FF` as the actual seed of the `C7 C3 00 FA` family
  2. `0x0DC3` as the seed of the `DB 17 75` family
  3. only then, a re-evaluation of `0x2000` as code, data, or hybrid

## Readable Runtime Lift

- The first step of that plan is now implemented in checked source:
  - `logic/unpacked_runtime_fragments.h`
  - `logic/unpacked_runtime_fragments.c`
- Covered readable fragments:
  - seed `0x05FF`
  - seed `0x0DC3`
  - aligned block `0x05F0..0x0610`
  - aligned block `0x0DC0..0x0DD0`
  - `0x05FF` family fan-out windows:
    - `0x05A4..0x06A1`
    - `0x06B0..0x07AD`
    - `0x05E5..0x06AD`
    - `0x0FD0..0x1098`
    - `0x05F5..0x06D6`
    - `0x1B7A..0x1C5B`
  - `0x0DC3` family windows:
    - `0x0D1A..0x0E05`
    - `0x1C5F..0x1D4A`
  - `0x2000`-derived copy family windows:
    - `0x1F6D..0x206C`
    - `0x21B6..0x22B5`
    - `0x2003..0x2021`
    - `0x207E..0x209C`
  - second dense post-root family:
    - `0x110E..0x112E`
    - `0x2E90..0x2EB0`
    - `0x2F00..0x2F40`
    - `0x2EF0..0x2F40`
    - `0x2EC0..0x2F40`
    - `0x2F40..0x2F80`
    - `0x2F00..0x2F80`
    - `0x2F80..0x2FC0`
    - `0x2FC0..0x3000`
    - `0x3000..0x3040`
    - `0x3040..0x3080`
    - `0x3080..0x30AA`
    - `0x2F00..0x30AA`
    - exact tail clones `0x2552..0x2592`, `0x1909..0x1933`, and `0x2612..0x263C`
  - dense `0x0F20/0x0F46` family:
    - `0x0EFF..0x0FAB`
    - `0x0F20..0x0F96`
    - `0x0F46..0x0F96`
    - `0x2A3A..0x2B1A`
    - `0x2A6E..0x2B1A`
    - `0x2A8F..0x2B05`
    - `0x2AB5..0x2B05`
    - `0x2C7F..0x2D3F`
    - `0x2CFA..0x2D4A`
  - the full `64` bytes of the `0x2000` root window
- Regression gate:
  - `make unpacked-runtime-fixtures`
  - current readable runtime cases: `44`

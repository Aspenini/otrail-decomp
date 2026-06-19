# Unpacked Window `0x2000..0x2040`

Scope: readable bootstrap payload window `0x2000..0x2040`.

## Current Status

- This is the highest-ranked current post-unpack provenance target.
- The current fixed report is:
  - `build/unpacked_window_report.md`
- The current sweep report is:
  - `build/unpacked_window_sweep_report.md`

## Summary Metrics

- size: `64` bytes
- non-zero bytes: `42`
- zero bytes: `22`
- printable bytes: `12`
- control-like bytes: `6`
- overlapping trace events: `30`
- event mix:
  - `19` literal events
  - `6` short-copy events
  - `5` long-copy events
- literal bytes contributed: `19`
- short-copy bytes contributed: `21`
- long-copy bytes contributed: `24`
- wrapped-copy events: `0`
- current sweep score: `106`

## Dominant Copy Contributors

Top current copy-source ranges by contributed bytes:

- `0x03ED..0x03F6`
  - contributes `9` bytes via long token `0x1FD4`
  - emits `42 00 00 00 11 F0 EB 00 00`
- `0x1DCB..0x1DD1`
  - contributes `6` bytes via long token `0xECA1`
  - emits `00 00 F7 A5 99 00`
- `0x1F87..0x1F8C`
  - contributes `5` bytes via short copy
  - emits zero padding
- `0x2006..0x200B`
  - contributes `5` bytes via short self-copy
  - repeats the `C7 C3 00 FA DB` motif already present earlier in the window
- `0x070B..0x070F`
  - contributes `4` bytes via long token `0x3A05`
  - emits `C7 C3 00 FA`
- `0x0DC3..0x0DC6`
  - contributes `3` bytes via long token `0x69B9`
  - emits `DB 17 75`

## Immediate Observations

- The window is denser than `0x0818`, but it is still not clean linear code.
- The opening bytes are assembled from both literals and copied motifs:
  - literal bytes: `80 7D F6 4A`
  - copied motif from `0x070B`: `C7 C3 00 FA`
  - copied motif from `0x0DC3`: `DB 17 75`
- The window contains short self-copy reuse inside the same range:
  - `0x200D..0x2010` copies from `0x2005..0x2008`
  - `0x203A..0x203F` copies from `0x2006..0x200B`
- Unlike `0x0818`, the current window has no wrapped-copy overlaps, which is one
  reason it ranks first in the sweep.
- The contributor chain now separates the root into two different classes:
  - dense motif sources worth following immediately:
    - `0x070B..0x070F`
    - `0x0DC3..0x0DC6`
  - mostly zero-rich carrier sources:
    - `0x03ED..0x03F6`
    - `0x1DCB..0x1DD1`
    - `0x1F87..0x1F8C`

## Working Interpretation

- Treat this as a high-value mixed block rather than confirmed executable code.
- It may be:
  - a real code fragment contaminated by copied zero/data padding
  - a structured table with opcode-like bytes
  - a relocation- or dispatch-adjacent blob assembled from reused motifs
- The provenance is now strong enough to stop treating it as an anonymous byte
  island.
- Current best interpretation:
  - `0x2000` looks like a hybrid fragment assembled from a few repeated motifs
    plus zero-padding carriers, not a clean standalone entry block.

## Motif Family Update

- The new fixed motif-family report is:
  - `build/unpacked_motif_family_report.md`
- Durable notes for those families now live in:
  - `docs/disasm/unpacked_motif_family_notes.md`
- Most important correction from that report:
  - `0x070B..0x070F` is not the family seed
  - the exact `C7 C3 00 FA` family starts earlier at `0x05FF`
  - `0x070B` is a downstream copy reused later at `0x2006`, `0x203A`,
    `0x2081`, `0x224F`, and `0x2283`
- `0x0DC3..0x0DC6` remains a real seed window:
  - its `DB 17 75` family currently propagates to `0x1D08`, `0x200A`,
    `0x2085`, and `0x2253`

## Readable Fragment Update

- The full `64` bytes of this root window are now modeled in checked source:
  - `logic/unpacked_runtime_fragments.h`
  - `logic/unpacked_runtime_fragments.c`
- The checked full window is:
  - `30 E4 80 7D F6 4A C7 C3 00 FA DB 17 75 4A C7 C3`
  - `D4 68 72 00 00 F8 00 00 00 42 00 00 00 11 F0 EB`
  - `00 00 63 16 73 00 00 00 00 00 00 00 F7 A5 99 00`
  - `00 00 42 EE E7 F0 F8 7E CD BE C7 C3 00 FA DB CC`
- The checked runtime lift now also includes the first exact source and clone
  families hanging off this root:
  - enclosing source `0x1F6D..0x206C`
  - exact downstream clone `0x21B6..0x22B5`
  - root-derived slice `0x2003..0x2021`
  - exact downstream clone `0x207E..0x209C`
- That keeps the root in context: it is still treated as mixed/runtime
  provenance, not confirmed executable code, but it is no longer only
  documented in reports or isolated from its first direct copy family.

## Next Lift Plan

1. Extend beyond the now-lifted root-derived copy chain into the next dense
   post-root targets beyond the closed `0x2F00..0x30AA` family and the now
   lifted `0x0EFF..0x0FAB` / `0x2A3A..0x2B1A` envelope family. The
   `0x3000..0x3080` continuation is sparse zero-heavy tail, so the next lift
   should favor another dense sibling window.
2. Treat `0x03ED`, `0x1DCB`, and `0x1F87` as carrier/support windows unless
   later provenance shows they hide denser structure.
3. Re-evaluate whether `0x2000` is best modeled as code, table data, or a
   hybrid fragment now that its first direct source/clone families are fixed in
   readable source.

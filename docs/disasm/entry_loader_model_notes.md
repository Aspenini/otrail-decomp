# Entry Loader Model Notes

Model file: `logic/entry_loader_model.c`
Readable companion: `logic/entry_loader_readable.c`

Target assembly window: `0x126FE`-`0x1274D`

## Mapped blocks

- `0x126FE`-`0x1271A`
  - segment prep + backward `rep movsb`
  - modeled by `otrail_entry_stage0_reloc_copy()`
- `0x1271B`-`0x1274D`
  - repeated window/segment slide with backward `rep movsw`
  - modeled by `otrail_entry_stage1_window_slide()`

## Intent

This model isolates pre-unpack relocation behavior so the unpacker stage can be
validated independently. It preserves overlap-safe backward copying and chunked
movement structure from the assembly, while abstracting segmented address math
into flat indices.

## Derived constants from the real EXE

- embedded loader words at entry `CS`:
  - `[cs:0x08] = 0x126D` total stage-1 paragraphs
  - `[cs:0x0A] = 0x1116` stage-0 relocation segment delta
  - `[cs:0x0C] = 0x0EEA` stage-0 relocation byte count
- stage-0 `push bx; push 0x002B; retf` transfers into the relocated copy at:
  - `relocated_cs:0x002B`
- stage-1 resolves into two backward-copy passes:
  - `0x1000` paragraphs
  - `0x026D` paragraphs
- exact stage-1 register trace is now captured per pass:
  - pass 0: `BP 0x126D -> 0x026D`, `DX 0xB26D -> 0xA26D`,
    `BX 0xC383 -> 0xB383`, `CX 0x8000`, `SI=DI=0xFFFE`
  - pass 1: `BP 0x026D -> 0x0000`, `DX 0xA26D -> 0xA000`,
    `BX 0xB383 -> 0xB116`, `CX 0x1368`, `SI=DI=0x26CE`
- with normalized `load_seg=0xA000`, the composed bootstrap path reaches:
  - unpacker `DS=0xB116`
  - unpacker `ES=0xA000`
- the first unpacker control seed at `DS:0` is now fixture-checked through the
  lifted `unit_0002` handoff:
  - `seed_word = 0xFFFF`
  - `seed_bits = 0x7FFF`
  - `first_gate_is_literal = 1`
- the resulting post-loader stream decodes successfully in both strict and
  heuristic modes to the same `12658`-byte output.
- the readable loader is fixture-checked for equivalence on stage-0/stage-1
  relocation fixtures.
- readable bootstrap mode now runs readable loader relocation plus readable
  unpacking, and matches the inferred composed path in both strict and
  heuristic modes.

## Remaining gaps

- Exact segment arithmetic (`ds`/`es` paragraph math) is still abstracted.
- Per-pass `BP`/`DX`/`BX`/`CX`/`SI` progression is now fixture-checked, but it is
  still derived from static lift rather than traced from a live DOS execution.
- Deterministic regression fixtures now protect overlap and chunk-shape behavior
  in both inferred and readable loader implementations.
- Needs trace-driven validation against runtime memory snapshots and eventual
  authored-source promotion once segment math is fully understood.

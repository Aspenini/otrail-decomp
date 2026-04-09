# Entry Loader Model Notes

Model file: `logic/entry_loader_model.c`

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

## Remaining gaps

- Exact segment arithmetic (`ds`/`es` paragraph math) is abstracted.
- Word-count derivation from original register flow is approximated.
- Needs trace-driven validation against runtime memory snapshots.

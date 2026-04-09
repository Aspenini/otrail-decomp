# Entrypoint Chain Code/Data Map

Scope: contiguous chain `unit_0001` through `unit_0060`
(`0x126FE` through `0x135DA`, exclusive).

## High-confidence executable region

- `0x126FE`-`0x1271A`: relocation/setup stub
  - segment setup
  - backward copy with `rep movsb`
  - `retf` handoff
- `0x1271B`-`0x1274D`: relocation/window movement loop
  - adjusts `ds`/`es` and copies words in chunks
- `0x1274F`-`0x12845`: compressed stream decode routine
  - bit-buffer refill from `lodsw`
  - literal path via `movsb`
  - back-reference copy path via `mov al,[es:bx+di]` then `stosb`
  - terminal transfer uses stack/segment switch and far jump

## Probable mixed data / non-linear decode region

- `0x12848`-`0x135DA`
  - linear disassembly quickly loses plausibility
  - repeated immediate-heavy patterns (`0x05`, `0x0A`, `0x0D`, etc.)
  - likely includes encoded data tables and/or payload bytes
  - should be treated as non-authoritative code until validated by control-flow tracing

## Practical decomp guidance

1. Treat `0x1274F`-`0x12845` as the first semantic lift target.
2. Keep `0x12848+` as byte-verified payload/data until dynamic tracing
   identifies actual execution paths.
3. Avoid lifting post-`0x12848` bytes to C as executable logic without
   additional evidence.

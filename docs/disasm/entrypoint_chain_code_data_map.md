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
  - long-control tail re-decodes cleanly from `0x127EC`
  - terminal transfer uses stack/segment switch and far jump

## Probable mixed data / non-linear decode region

- `0x12848`-`0x135DA`
  - linear disassembly quickly loses plausibility
  - repeated immediate-heavy patterns (`0x05`, `0x0A`, `0x0D`, etc.)
  - likely includes encoded data tables and/or payload bytes
  - should be treated as non-authoritative code until validated by control-flow tracing

## Practical decomp guidance

1. Treat `0x1274F`-`0x12845` as the first semantic lift target.
2. When inspecting the tail, re-decode from `0x127EC`; the plain linear pass
   through `0x127E7` crosses an alternate instruction alignment.
3. The first `31` prepared-stream events from the real loader handoff are now
   fixture-checked, anchoring the initial literal burst and first short/long
   token shapes before deeper block recovery.
4. The resumed `unit_0003` short path, resumed long path, and first
   `unit_0004` copy-control long token are now directly fixture-checked from
   the real EXE handoff state.
5. The resumed `unit_0004` `control_byte=1` window-slide case is also now
   fixture-checked, and the full successful heuristic trace ends on a terminal
   `ext=0` long-control marker.
6. Treat `0x127EC-0x12845` specifically as the exact relocation-stream tail
   documented in `docs/disasm/entry_tail_relocation_model.md`, not as generic
   mixed tail bytes.
7. Keep `0x12848+` as byte-verified payload/data until dynamic tracing
   identifies actual execution paths.
8. Avoid lifting post-`0x12848` bytes to C as executable logic without
   additional evidence.

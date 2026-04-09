# Entrypoint Chain Notes (`unit_0001`-`unit_0060`)

This note accompanies `docs/disasm/entrypoint_chain_0001_0060.ndisasm`, which is a
16-bit disassembly of the contiguous byte range from file offset `0x126FE` to
`0x135DA` (exclusive).

## High-confidence observations

- The first block (`0x126FE` onward) performs relocation/copy setup:
  - segment register setup (`push es`, `push cs`, `pop ds`)
  - backward `rep movsb` copy sequence
  - far return (`retf`) into another location
- The next major block (starting near `0x1274F`) appears to be a bitstream
  decoder loop:
  - repeated refill/check of a bit buffer (`lodsw`, `shr`, carry-driven paths)
  - literal copy path (`movsb`)
  - back-reference copy path (`mov al,[es:bx+di]`, `stosb`, `loop`)
- Toward the end of this range, instruction plausibility drops and patterns
  suggest mixed data/code interpretation in a linear disassembly pass.

## Caveats

- This is a raw linear disassembly (`ndisasm -b 16`) of a byte range, not a
  control-flow recovered function map.
- Some `ndisasm` warnings (`unknown byte code: 0300`) occurred during decode,
  indicating bytes that do not map cleanly in this context.
- The tail region likely contains tables or compressed data interpreted as code.

## Next analysis steps

1. Split this chain into control-flow basic regions (code vs probable data).
2. Mark candidate decoder routine boundaries and inputs/outputs.
3. Lift the decoder region into a standalone C model for behavior tests.
4. Tie modeled behavior to asset loading paths in the executable.

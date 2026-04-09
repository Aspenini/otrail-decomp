# Entry Unpacker Block Hypotheses (`0x1274F`-`0x12845`)

This document defines provisional semantic blocks for the unpacker loop region
using the linear disassembly in `entrypoint_chain_0001_0060.ndisasm`.

Ground truth file for automated mapping:

- `config/entry_unpacker_blocks_1274f_12845.json`

## Block Set

| ID | Range | Provisional Role |
|---|---|---|
| `B00_init_and_seed` | `0x1274F-0x12756` | initialize decode cursors and first bit-buffer seed |
| `B01_bit_gate_top` | `0x12757-0x12764` | top-level bit gate and literal-vs-second-stage branch |
| `B02_literal_emit` | `0x12765-0x12767` | literal emit (`movsb`) and loop-back |
| `B03_second_gate_setup` | `0x12768-0x12775` | second-stage gate that splits short/long token paths |
| `B04_short_len_build` | `0x12776-0x1278F` | short-token length assembly from carry chain |
| `B05_short_backref_setup` | `0x12790-0x12797` | short back-reference displacement setup |
| `B06_long_token_decode` | `0x12798-0x127AA` | long-token decode and inline length extraction |
| `B07_copy_loop` | `0x127AB-0x127B1` | shared copy loop into output |
| `B08_long_ext_control` | `0x127B3-0x127C0` | long-token extension/control markers |
| `B09_window_slide` | `0x127C1-0x127E4` | segment/window slide control path |
| `B10_tail_uncertain` | `0x127E5-0x12845` | low-confidence tail (possible mixed code/data) |

## Correlation Workflow

```bash
make unpacker-trace TRACE_OFFSET=0x1274F TRACE_MAX_EVENTS=100 TRACE_MODE=1
make unpacker-trace-blocks TRACE_BLOCKS_JSON=config/entry_unpacker_blocks_1274f_12845.json
```

Outputs:

- `build/unpacker_trace.csv`
- `build/unpacker_trace_blocks.csv`
- `build/unpacker_trace_blocks.md`

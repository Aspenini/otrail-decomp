/*
 * Public declarations for the exact unit_0006 mixed code/data slice.
 */

#ifndef UNIT_0006_ENTRYPOINT_CONTIG_64_H
#define UNIT_0006_ENTRYPOINT_CONTIG_64_H

#include <stdint.h>

#define UNIT_0006_HANDOFF_CODE_SIZE 9u
#define UNIT_0006_POSTJUMP_DATA_SIZE 54u

typedef struct Unit0006TerminalHeader {
    uint16_t jump_off;
    uint16_t jump_seg_word;
    uint16_t stack_off;
    uint16_t stack_seg_rel;
} Unit0006TerminalHeader;

typedef struct Unit0006TerminalHandoff {
    uint16_t final_ds;
    uint16_t final_es;
    uint16_t final_ss;
    uint16_t final_sp;
    uint16_t jump_off;
    uint16_t jump_seg;
    uint8_t cli_seen;
    uint8_t sti_seen;
    uint8_t bx_cleared;
} Unit0006TerminalHandoff;

typedef struct Unit0006MixedSlice {
    uint8_t begins_mid_instruction;
    uint8_t leading_xor_tail_byte;
    uint8_t handoff_code_size;
    uint8_t postjump_data_size;
} Unit0006MixedSlice;

extern const uint8_t unit_0006_entrypoint_handoff_code_bytes[UNIT_0006_HANDOFF_CODE_SIZE];
extern const uint8_t unit_0006_entrypoint_postjump_data_prefix_bytes[UNIT_0006_POSTJUMP_DATA_SIZE];

int unit_0006_entrypoint_finalize_terminal_handoff(
    uint16_t bx_seed,
    const Unit0006TerminalHeader *header,
    Unit0006TerminalHandoff *out
);

int unit_0006_entrypoint_describe_mixed_slice(Unit0006MixedSlice *out);

#endif

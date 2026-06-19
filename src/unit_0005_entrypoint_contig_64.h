/*
 * Public declarations for the lifted unit_0005 relocation-tail semantics.
 */

#ifndef UNIT_0005_ENTRYPOINT_CONTIG_64_H
#define UNIT_0005_ENTRYPOINT_CONTIG_64_H

#include <stdint.h>

typedef struct Unit0005TailState {
    uint16_t bx_seed;
    uint16_t dx_rel;
    uint16_t di;
} Unit0005TailState;

typedef enum Unit0005TailKind {
    UNIT0005_TAIL_PATCH = 0,
    UNIT0005_TAIL_SEGMENT_BUMP = 1,
    UNIT0005_TAIL_TERMINAL = 2
} Unit0005TailKind;

typedef struct Unit0005TailDispatch {
    Unit0005TailKind kind;
    Unit0005TailState next_state;
    uint16_t patch_off;
    uint16_t patch_increment;
    uint8_t consumed_low_byte_only;
} Unit0005TailDispatch;

int unit_0005_entrypoint_tail_apply_byte(
    const Unit0005TailState *in,
    uint8_t value,
    Unit0005TailDispatch *out
);

int unit_0005_entrypoint_tail_apply_control(
    const Unit0005TailState *in,
    uint16_t control_word,
    Unit0005TailDispatch *out
);

#endif

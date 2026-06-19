/*
 * Public declarations for the exact unit_0004 long-control and tail-bootstrap lift.
 */

#ifndef UNIT_0004_ENTRYPOINT_CONTIG_64_H
#define UNIT_0004_ENTRYPOINT_CONTIG_64_H

#include <stdint.h>

#include "unit_0005_entrypoint_contig_64.h"

typedef struct Unit0004WindowState {
    uint16_t ds;
    uint16_t es;
    uint16_t si;
    uint16_t di;
} Unit0004WindowState;

typedef enum Unit0004ControlKind {
    UNIT0004_CONTROL_COPY = 0,
    UNIT0004_CONTROL_WINDOW_SLIDE = 1,
    UNIT0004_CONTROL_STREAM_END = 2
} Unit0004ControlKind;

typedef struct Unit0004ControlResult {
    Unit0004ControlKind kind;
    uint16_t copy_len;
    Unit0004WindowState next_window;
    uint32_t tail_target_offset;
    uint8_t reenter_top_gate;
    uint8_t tail_target_is_exact;
} Unit0004ControlResult;

typedef struct Unit0004TailBootstrap {
    Unit0005TailState initial_state;
    Unit0005TailDispatch dispatch;
    uint16_t next_stream_offset;
    uint8_t first_stream_byte;
    uint16_t first_control_word;
    uint8_t consumed_control_word;
} Unit0004TailBootstrap;

int unit_0004_entrypoint_resolve_long_control(
    uint8_t control_byte,
    const Unit0004WindowState *window,
    Unit0004ControlResult *out
);

int unit_0004_entrypoint_bootstrap_relocation_tail(
    uint16_t popped_bx,
    uint8_t first_stream_byte,
    uint16_t first_control_word,
    Unit0004TailBootstrap *out
);

#endif

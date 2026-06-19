/*
 * Exact lift for entry bytes 0x127BE-0x127FD.
 *
 * This slice resolves long-token control markers, models the 0x2000-byte
 * window slide, and now also owns the exact 0x127EC tail bootstrap:
 *   push cs ; pop ds ; mov si,0x0158 ; pop bx ; add bx,0x10 ; mov dx,bx ;
 *   xor di,di ; lodsb ; or al,al ; ...
 */

#include <stddef.h>
#include <stdint.h>

#include "unit_0004_entrypoint_contig_64.h"

const uint8_t unit_0004_entrypoint_contig_64_bytes[] = {
    0x41, 0xEB, 0xEA, 0x89, 0xFB, 0x83, 0xE7, 0x0F, 0x81, 0xC7, 0x00, 0x20, 0xB1, 0x04, 0xD3, 0xEB,
    0x8C, 0xC0, 0x01, 0xD8, 0x2D, 0x00, 0x02, 0x8E, 0xC0, 0x89, 0xF3, 0x83, 0xE6, 0x0F, 0xD3, 0xEB,
    0x8C, 0xD8, 0x01, 0xD8, 0x8E, 0xD8, 0xE9, 0x72, 0xFF, 0x2A, 0x46, 0x41, 0x42, 0x2A, 0x0E, 0x1F,
    0xBE, 0x58, 0x01, 0x5B, 0x83, 0xC3, 0x10, 0x89, 0xDA, 0x31, 0xFF, 0xAC, 0x08, 0xC0, 0x74, 0x16,
};

static void unit_0004_apply_window_slide(
    const Unit0004WindowState *in,
    Unit0004WindowState *out
) {
    uint16_t bx;
    uint16_t ax;

    *out = *in;

    bx = out->di;
    out->di &= 0x000Fu;
    out->di = (uint16_t)(out->di + 0x2000u);

    bx >>= 4;
    ax = out->es;
    ax = (uint16_t)(ax + bx);
    ax = (uint16_t)(ax - 0x0200u);
    out->es = ax;

    bx = out->si;
    out->si &= 0x000Fu;
    bx >>= 4;
    ax = out->ds;
    ax = (uint16_t)(ax + bx);
    out->ds = ax;
}

int unit_0004_entrypoint_resolve_long_control(
    uint8_t control_byte,
    const Unit0004WindowState *window,
    Unit0004ControlResult *out
) {
    if (window == NULL || out == NULL) {
        return 0;
    }

    out->copy_len = 0u;
    out->next_window = *window;
    out->tail_target_offset = 0u;
    out->reenter_top_gate = 0u;
    out->tail_target_is_exact = 0u;

    if (control_byte == 0u) {
        out->kind = UNIT0004_CONTROL_STREAM_END;
        out->tail_target_offset = 0x127ECu;
        out->tail_target_is_exact = 1u;
        return 1;
    }

    if (control_byte == 1u) {
        out->kind = UNIT0004_CONTROL_WINDOW_SLIDE;
        unit_0004_apply_window_slide(window, &out->next_window);
        out->reenter_top_gate = 1u;
        return 1;
    }

    out->kind = UNIT0004_CONTROL_COPY;
    out->copy_len = (uint16_t)(control_byte + 1u);
    return 1;
}

int unit_0004_entrypoint_bootstrap_relocation_tail(
    uint16_t popped_bx,
    uint8_t first_stream_byte,
    uint16_t first_control_word,
    Unit0004TailBootstrap *out
) {
    uint16_t bx_seed;

    if (out == NULL) {
        return 0;
    }

    bx_seed = (uint16_t)(popped_bx + 0x0010u);
    out->initial_state.bx_seed = bx_seed;
    out->initial_state.dx_rel = bx_seed;
    out->initial_state.di = 0u;
    out->first_stream_byte = first_stream_byte;
    out->first_control_word = 0u;
    out->consumed_control_word = 0u;
    out->next_stream_offset = 0x0159u;

    if (first_stream_byte == 0u) {
        out->first_control_word = first_control_word;
        out->consumed_control_word = 1u;
        out->next_stream_offset = 0x015Bu;
        return unit_0005_entrypoint_tail_apply_control(
            &out->initial_state,
            first_control_word,
            &out->dispatch
        );
    }

    return unit_0005_entrypoint_tail_apply_byte(
        &out->initial_state,
        first_stream_byte,
        &out->dispatch
    );
}

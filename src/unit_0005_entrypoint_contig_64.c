/*
 * Exact relocation-tail lift for entry bytes 0x127FE-0x1283D.
 *
 * This slice is the second half of the 0x127EC tail re-decode. It does not
 * describe generic run-length accumulation anymore; it models the exact
 * relocation stream semantics:
 *   - non-zero bytes relocate a word at [ES:DI] by BX
 *   - control word 0x0000 performs a paragraph-wrap segment bump
 *   - control word 0x0001 transfers into the terminal handoff sequence
 *   - any other control word reuses AL (the low byte) as the next patch delta
 */

#include <stdint.h>

#include "unit_0005_entrypoint_contig_64.h"

const uint8_t unit_0005_entrypoint_contig_64_bytes[] = {
    0xB4, 0x00, 0x01, 0xC7, 0x8B, 0xC7, 0x83, 0xE7, 0x0F, 0xB1, 0x04, 0xD3, 0xE8, 0x01, 0xC2, 0x8E,
    0xC2, 0x26, 0x01, 0x1D, 0xEB, 0xE5, 0xAD, 0x09, 0xC0, 0x75, 0x08, 0x81, 0xC2, 0xFF, 0x0F, 0x8E,
    0xC2, 0xEB, 0xD8, 0x3D, 0x01, 0x00, 0x75, 0xDA, 0x8B, 0xC3, 0x8B, 0x3E, 0x04, 0x00, 0x8B, 0x36,
    0x06, 0x00, 0x01, 0xC6, 0x01, 0x06, 0x02, 0x00, 0x2D, 0x10, 0x00, 0x8E, 0xD8, 0x8E, 0xC0, 0x31,
};

static int unit_0005_dispatch_patch(
    const Unit0005TailState *in,
    uint8_t value,
    uint8_t consumed_low_byte_only,
    Unit0005TailDispatch *out
) {
    uint16_t di_accum;
    uint16_t paragraph_carry;

    if (in == 0 || out == 0) {
        return 0;
    }

    out->kind = UNIT0005_TAIL_PATCH;
    out->next_state = *in;
    out->patch_increment = in->bx_seed;
    out->consumed_low_byte_only = consumed_low_byte_only;

    di_accum = (uint16_t)(in->di + value);
    paragraph_carry = (uint16_t)(di_accum >> 4);
    out->next_state.di = (uint16_t)(di_accum & 0x000Fu);
    out->next_state.dx_rel = (uint16_t)(in->dx_rel + paragraph_carry);
    out->patch_off = (uint16_t)((((uint32_t)out->next_state.dx_rel) << 4) + out->next_state.di);
    return 1;
}

int unit_0005_entrypoint_tail_apply_byte(
    const Unit0005TailState *in,
    uint8_t value,
    Unit0005TailDispatch *out
) {
    return unit_0005_dispatch_patch(in, value, 0u, out);
}

int unit_0005_entrypoint_tail_apply_control(
    const Unit0005TailState *in,
    uint16_t control_word,
    Unit0005TailDispatch *out
) {
    if (in == 0 || out == 0) {
        return 0;
    }

    out->next_state = *in;
    out->patch_off = 0u;
    out->patch_increment = 0u;
    out->consumed_low_byte_only = 0u;

    if (control_word == 0u) {
        out->kind = UNIT0005_TAIL_SEGMENT_BUMP;
        out->next_state.dx_rel = (uint16_t)(in->dx_rel + 0x0FFFu);
        return 1;
    }

    if (control_word == 1u) {
        out->kind = UNIT0005_TAIL_TERMINAL;
        return 1;
    }

    return unit_0005_dispatch_patch(in, (uint8_t)(control_word & 0x00FFu), 1u, out);
}

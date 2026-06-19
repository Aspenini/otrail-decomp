/*
 * Lifted semantics for entry bytes 0x1277E-0x127BD.
 *
 * This slice resumes the unpacker's non-literal token body after the second
 * gate has already selected either the short or long path.
 */

#include <stddef.h>
#include <stdint.h>

#include "unit_0003_entrypoint_contig_64.h"

const uint8_t unit_0003_entrypoint_contig_64_bytes[] = {
    0xB2, 0x10, 0xD1, 0xD1, 0xD1, 0xED, 0x4A, 0x75, 0x05, 0xAD, 0x89, 0xC5, 0xB2, 0x10, 0xD1, 0xD1,
    0x41, 0x41, 0xAC, 0xB7, 0xFF, 0x8A, 0xD8, 0xE9, 0x13, 0x00, 0xAD, 0x8B, 0xD8, 0xB1, 0x03, 0xD2,
    0xEF, 0x80, 0xCF, 0xE0, 0x80, 0xE4, 0x07, 0x74, 0x0C, 0x88, 0xE1, 0x41, 0x41, 0x26, 0x8A, 0x01,
    0xAA, 0xE2, 0xFA, 0xEB, 0xA6, 0xAC, 0x08, 0xC0, 0x74, 0x34, 0x3C, 0x01, 0x74, 0x05, 0x88, 0xC1,
};

static int unit_0003_refill(Unit0003BitCursor *cursor) {
    if (cursor->src_pos + 2u > cursor->src_len) {
        return 0;
    }
    cursor->bitbuf = (uint16_t)cursor->src[cursor->src_pos] |
        ((uint16_t)cursor->src[cursor->src_pos + 1u] << 8);
    cursor->src_pos += 2u;
    cursor->bits_left = 16u;
    return 1;
}

static int unit_0003_get_bit(Unit0003BitCursor *cursor, uint8_t *out_bit) {
    if (cursor->bits_left == 0u && !unit_0003_refill(cursor)) {
        return 0;
    }
    *out_bit = (uint8_t)(cursor->bitbuf & 1u);
    cursor->bitbuf >>= 1;
    cursor->bits_left--;
    return 1;
}

static int unit_0003_get_u8(Unit0003BitCursor *cursor, uint8_t *out_byte) {
    if (cursor->src_pos >= cursor->src_len) {
        return 0;
    }
    *out_byte = cursor->src[cursor->src_pos++];
    return 1;
}

static int unit_0003_get_u16le(Unit0003BitCursor *cursor, uint16_t *out_word) {
    if (cursor->src_pos + 2u > cursor->src_len) {
        return 0;
    }
    *out_word = (uint16_t)cursor->src[cursor->src_pos] |
        ((uint16_t)cursor->src[cursor->src_pos + 1u] << 8);
    cursor->src_pos += 2u;
    return 1;
}

static int unit_0003_finish_short_path(
    Unit0003BitCursor *cursor,
    Unit0003TokenState *out
) {
    uint8_t bit_a;
    uint8_t bit_b;
    uint8_t low_disp;

    if (!unit_0003_get_bit(cursor, &bit_a) || !unit_0003_get_bit(cursor, &bit_b)) {
        return 0;
    }
    if (!unit_0003_get_u8(cursor, &low_disp)) {
        return 0;
    }

    out->kind = UNIT0003_TOKEN_SHORT_BACKREF;
    out->copy_len = (uint16_t)(((bit_a << 1) | bit_b) + 2u);
    out->backref_disp = (int16_t)(uint16_t)(0xFF00u | low_disp);
    out->control_byte = 0u;
    out->needs_unit_0004_resolution = 0u;
    return 1;
}

static int unit_0003_finish_long_path(
    Unit0003BitCursor *cursor,
    Unit0003TokenState *out
) {
    uint16_t token;
    uint8_t low;
    uint8_t high;
    uint8_t len_low3;

    if (!unit_0003_get_u16le(cursor, &token)) {
        return 0;
    }

    low = (uint8_t)(token & 0xFFu);
    high = (uint8_t)(token >> 8);
    len_low3 = (uint8_t)(high & 0x07u);

    out->kind = UNIT0003_TOKEN_LONG_BACKREF;
    out->backref_disp = (int16_t)(uint16_t)(
        (((uint16_t)(0xE0u | (high >> 3)) << 8) | low)
    );
    out->control_byte = 0u;
    out->needs_unit_0004_resolution = 0u;

    if (len_low3 != 0u) {
        out->copy_len = (uint16_t)(len_low3 + 2u);
        return 1;
    }

    if (!unit_0003_get_u8(cursor, &out->control_byte)) {
        return 0;
    }

    out->kind = UNIT0003_TOKEN_LONG_CONTROL;
    out->copy_len = 0u;
    out->needs_unit_0004_resolution = 1u;
    return 1;
}

int unit_0003_entrypoint_resume_nonliteral_token(
    Unit0003BitCursor *cursor,
    int take_long_path,
    Unit0003TokenState *out
) {
    if (cursor == NULL || out == NULL) {
        return 0;
    }
    if (take_long_path) {
        return unit_0003_finish_long_path(cursor, out);
    }
    return unit_0003_finish_short_path(cursor, out);
}

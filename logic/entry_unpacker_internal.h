/*
 * Internal unpacker engine declarations.
 */

#ifndef OTRAIL_ENTRY_UNPACKER_INTERNAL_H
#define OTRAIL_ENTRY_UNPACKER_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#include "entry_unpacker_model.h"

typedef struct BitReader16 {
    const uint8_t *src;
    size_t src_len;
    size_t pos;
    uint16_t bitbuf;
    uint8_t bits_left;
} BitReader16;

typedef struct UnpackState {
    uint8_t *dst;
    size_t dst_cap;
    size_t out_pos;
    uint8_t *history;
    uint8_t *touched;
    uint16_t hpos;
    int mode;
    OtrailUnpackStats *stats;
} UnpackState;

int br_get_bit(BitReader16 *br, uint8_t *out_bit);
int br_get_u8(BitReader16 *br, uint8_t *out);
int br_get_u16le(BitReader16 *br, uint16_t *out);

int unpack_fail(
    UnpackState *st,
    BitReader16 *br,
    size_t *src_used,
    size_t *dst_written,
    int fail_code
);

int emit_literal(UnpackState *st, uint8_t literal);
int copy_from_history(UnpackState *st, ptrdiff_t back_disp, size_t copy_len, int fail_code);

/* Returns: 1=continue, 0=stream-end, -1=fail */
int decode_long_token(BitReader16 *br, UnpackState *st);
int decode_short_token(BitReader16 *br, UnpackState *st);

int otrail_entry_unpacker_run(
    const uint8_t *src,
    size_t src_len,
    uint8_t *dst,
    size_t dst_cap,
    size_t *src_used,
    size_t *dst_written,
    int mode,
    OtrailUnpackStats *stats
);

#endif

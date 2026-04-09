/*
 * Readable unpacker model aligned to block hypotheses.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "entry_unpacker_readable.h"

typedef struct ReadableBitReader {
    const uint8_t *src;
    size_t src_len;
    size_t pos;
    uint16_t bitbuf;
    uint8_t bits_left;
} ReadableBitReader;

typedef struct ReadableState {
    uint8_t *dst;
    size_t dst_cap;
    size_t out_pos;
    uint8_t *history;
    uint8_t *touched;
    uint16_t hpos;
    int mode;
    OtrailUnpackStats *stats;
} ReadableState;

static int rb_refill(ReadableBitReader *br) {
    if (br->pos + 2 > br->src_len) {
        return 0;
    }
    br->bitbuf = (uint16_t)br->src[br->pos] | ((uint16_t)br->src[br->pos + 1] << 8);
    br->pos += 2;
    br->bits_left = 16;
    return 1;
}

static int rb_get_bit(ReadableBitReader *br, uint8_t *out_bit) {
    if (br->bits_left == 0 && !rb_refill(br)) {
        return 0;
    }
    *out_bit = (uint8_t)(br->bitbuf & 1u);
    br->bitbuf >>= 1;
    br->bits_left--;
    return 1;
}

static int rb_get_u8(ReadableBitReader *br, uint8_t *out) {
    if (br->pos >= br->src_len) {
        return 0;
    }
    *out = br->src[br->pos++];
    return 1;
}

static int rb_get_u16le(ReadableBitReader *br, uint16_t *out) {
    if (br->pos + 2 > br->src_len) {
        return 0;
    }
    *out = (uint16_t)br->src[br->pos] | ((uint16_t)br->src[br->pos + 1] << 8);
    br->pos += 2;
    return 1;
}

static int fail_and_return(
    ReadableState *st,
    ReadableBitReader *br,
    size_t *src_used,
    size_t *dst_written,
    int fail_code
) {
    if (st->stats != NULL) {
        st->stats->fail_code = fail_code;
        st->stats->fail_src_pos = br->pos;
        st->stats->fail_out_pos = st->out_pos;
    }
    if (src_used != NULL) {
        *src_used = br->pos;
    }
    if (dst_written != NULL) {
        *dst_written = st->out_pos;
    }
    return 0;
}

/* B09 intent: strict mode prehistory seeding for +0x2000 window slides. */
static void seed_strict_prehistory(uint8_t *touched, int mode) {
    size_t i;
    if (mode != OTRAIL_UNPACK_STRICT) {
        return;
    }
    for (i = 0; i < 0x2000; i++) {
        touched[(uint16_t)(0xE000u + (uint16_t)i)] = 1u;
    }
}

/* B07 intent: shared backref copy loop. */
static int copy_backref(
    ReadableState *st,
    ptrdiff_t back_disp,
    size_t copy_len,
    int strict_fail_code,
    int generic_fail_code,
    int is_long_path
) {
    size_t i;
    if (st->out_pos + copy_len > st->dst_cap) {
        if (st->stats != NULL && st->stats->fail_code == 0) {
            st->stats->fail_code = generic_fail_code;
        }
        return 0;
    }

    for (i = 0; i < copy_len; i++) {
        uint16_t src_pos = (uint16_t)((uint16_t)st->hpos + (uint16_t)(int16_t)back_disp);
        uint8_t value;
        if (st->mode == OTRAIL_UNPACK_STRICT && !st->touched[src_pos]) {
            if (st->stats != NULL) {
                st->stats->fail_code = strict_fail_code;
            }
            return 0;
        }
        value = st->history[src_pos];
        st->dst[st->out_pos] = value;
        st->history[st->hpos] = value;
        st->touched[st->hpos] = 1u;
        st->hpos = (uint16_t)(st->hpos + 1u);
        st->out_pos++;
    }

    if (st->stats != NULL) {
        if (is_long_path) {
            st->stats->long_copy_ops++;
        } else {
            st->stats->short_copy_ops++;
        }
        st->stats->copied_bytes += copy_len;
    }
    return 1;
}

static int emit_literal_byte(ReadableState *st, uint8_t literal) {
    if (st->out_pos >= st->dst_cap) {
        return 0;
    }
    st->dst[st->out_pos++] = literal;
    st->history[st->hpos] = literal;
    st->touched[st->hpos] = 1u;
    st->hpos = (uint16_t)(st->hpos + 1u);
    if (st->stats != NULL) {
        st->stats->literal_ops++;
    }
    return 1;
}

/* B06 intent: long token displacement + low-length parse. */
static int parse_long_header(
    ReadableBitReader *br,
    ptrdiff_t *out_back_disp,
    size_t *out_len_low3
) {
    uint16_t token;
    uint8_t low;
    uint8_t high;

    if (!rb_get_u16le(br, &token)) {
        return 0;
    }
    low = (uint8_t)(token & 0xFFu);
    high = (uint8_t)((token >> 8) & 0xFFu);
    *out_back_disp = (ptrdiff_t)(int16_t)((uint16_t)(((uint16_t)(0xE0u | (high >> 3)) << 8) | low));
    *out_len_low3 = (size_t)(high & 0x7u);
    return 1;
}

/* B08 intent: extension/control decode (end/nop/extended copy len). */
static int resolve_long_length(
    ReadableBitReader *br,
    size_t len_low3,
    size_t *out_copy_len
) {
    if (len_low3 == 0) {
        uint8_t ext;
        if (!rb_get_u8(br, &ext)) {
            return -1;
        }
        if (ext == 0) {
            return 0; /* stream end */
        }
        if (ext == 1) {
            return 2; /* nop token */
        }
        *out_copy_len = (size_t)ext + 1u;
        return 1;
    }
    *out_copy_len = len_low3 + 2u;
    return 1;
}

/* Returns: 1=continue, 0=stream-end, -1=fail */
static int run_long_path(ReadableBitReader *br, ReadableState *st) {
    ptrdiff_t back_disp;
    size_t len_low3;
    size_t copy_len = 0;
    int len_state;

    if (!parse_long_header(br, &back_disp, &len_low3)) {
        if (st->stats != NULL) st->stats->fail_code = 4;
        return -1;
    }

    len_state = resolve_long_length(br, len_low3, &copy_len);
    if (len_state < 0) {
        if (st->stats != NULL) st->stats->fail_code = 5;
        return -1;
    }
    if (len_state == 0) {
        return 0;
    }
    if (len_state == 2) {
        return 1;
    }

    if (!copy_backref(st, back_disp, copy_len, 7, 6, 1)) {
        if (st->stats != NULL && st->stats->fail_code == 0) st->stats->fail_code = 6;
        return -1;
    }
    return 1;
}

/* B04 + B05 intent: short-token decode and copy. */
static int run_short_path(ReadableBitReader *br, ReadableState *st) {
    uint8_t b;
    uint8_t bit_a;
    uint8_t bit_b;
    size_t copy_len;
    ptrdiff_t back_disp;

    if (!rb_get_u8(br, &b)) {
        if (st->stats != NULL) st->stats->fail_code = 8;
        return 0;
    }
    if (!rb_get_bit(br, &bit_a) || !rb_get_bit(br, &bit_b)) {
        if (st->stats != NULL) st->stats->fail_code = 9;
        return 0;
    }

    copy_len = (size_t)(((bit_a << 1) | bit_b) + 2u);
    back_disp = (ptrdiff_t)(int16_t)(uint16_t)(0xFF00u | b);
    if (!copy_backref(st, back_disp, copy_len, 11, 10, 0)) {
        if (st->stats != NULL && st->stats->fail_code == 0) st->stats->fail_code = 10;
        return 0;
    }
    return 1;
}

/* Returns: 1=continue, 0=stream-end, -1=fail */
static int run_second_gate(ReadableBitReader *br, ReadableState *st) {
    uint8_t bit1;
    if (!rb_get_bit(br, &bit1)) {
        if (st->stats != NULL) st->stats->fail_code = 3;
        return -1;
    }
    if (bit1) {
        return run_long_path(br, st);
    }
    if (!run_short_path(br, st)) {
        if (st->stats != NULL && st->stats->fail_code == 0) st->stats->fail_code = 8;
        return -1;
    }
    return 1;
}

int otrail_entry_unpacker_readable_mode(
    const uint8_t *src,
    size_t src_len,
    uint8_t *dst,
    size_t dst_cap,
    size_t *src_used,
    size_t *dst_written,
    int mode,
    OtrailUnpackStats *stats
) {
    static uint8_t history[65536];
    static uint8_t touched[65536];
    ReadableBitReader br;
    ReadableState st;

    if (src == NULL || dst == NULL) {
        return 0;
    }

    br.src = src;
    br.src_len = src_len;
    br.pos = 0;
    br.bitbuf = 0;
    br.bits_left = 0;

    st.dst = dst;
    st.dst_cap = dst_cap;
    st.out_pos = 0;
    st.history = history;
    st.touched = touched;
    st.hpos = 0;
    st.mode = mode;
    st.stats = stats;

    memset(history, 0, sizeof(history));
    memset(touched, 0, sizeof(touched));
    seed_strict_prehistory(touched, mode);
    if (stats != NULL) {
        stats->literal_ops = 0;
        stats->short_copy_ops = 0;
        stats->long_copy_ops = 0;
        stats->copied_bytes = 0;
        stats->fail_code = 0;
        stats->fail_src_pos = 0;
        stats->fail_out_pos = 0;
    }

    while (1) {
        uint8_t bit0;
        if (!rb_get_bit(&br, &bit0)) {
            return fail_and_return(&st, &br, src_used, dst_written, 1);
        }

        if (bit0) {
            uint8_t literal;
            if (!rb_get_u8(&br, &literal) || !emit_literal_byte(&st, literal)) {
                return fail_and_return(&st, &br, src_used, dst_written, 2);
            }
            continue;
        }

        {
            int dispatch = run_second_gate(&br, &st);
            if (dispatch < 0) {
                return fail_and_return(
                    &st,
                    &br,
                    src_used,
                    dst_written,
                    (stats != NULL && stats->fail_code != 0) ? stats->fail_code : 3
                );
            }
            if (dispatch == 0) {
                break;
            }
        }
    }

    if (src_used != NULL) {
        *src_used = br.pos;
    }
    if (dst_written != NULL) {
        *dst_written = st.out_pos;
    }
    return 1;
}

int otrail_entry_unpacker_readable_ex(
    const uint8_t *src,
    size_t src_len,
    uint8_t *dst,
    size_t dst_cap,
    size_t *src_used,
    size_t *dst_written,
    OtrailUnpackStats *stats
) {
    return otrail_entry_unpacker_readable_mode(
        src,
        src_len,
        dst,
        dst_cap,
        src_used,
        dst_written,
        OTRAIL_UNPACK_HEURISTIC,
        stats
    );
}

int otrail_entry_unpacker_readable(
    const uint8_t *src,
    size_t src_len,
    uint8_t *dst,
    size_t dst_cap,
    size_t *src_used,
    size_t *dst_written
) {
    return otrail_entry_unpacker_readable_mode(
        src,
        src_len,
        dst,
        dst_cap,
        src_used,
        dst_written,
        OTRAIL_UNPACK_STRICT,
        NULL
    );
}

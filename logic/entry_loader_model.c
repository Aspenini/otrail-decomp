/*
 * Inferred entry loader stage model from 0x126FE-0x1274D.
 *
 * This approximates the relocation/window-moving behavior seen before the
 * unpacker loop starts. It is not yet a verified binary-equivalent rewrite.
 */

#include <stddef.h>
#include <stdint.h>

typedef struct LoaderWindow {
    uint8_t *src;
    uint8_t *dst;
    size_t bytes_to_move;
    size_t chunk_limit;
} LoaderWindow;

static void reverse_copy_bytes(uint8_t *dst, const uint8_t *src, size_t count) {
    while (count > 0) {
        count--;
        dst[count] = src[count];
    }
}

/*
 * Stage 0:
 * Mirrors initial backward REP MOVSB block (STD + REP MOVSB).
 */
int otrail_entry_stage0_reloc_copy(LoaderWindow *w) {
    if (w == NULL || w->src == NULL || w->dst == NULL) {
        return 0;
    }
    reverse_copy_bytes(w->dst, w->src, w->bytes_to_move);
    return 1;
}

/*
 * Stage 1:
 * Mirrors loop that repeatedly shifts source/destination windows by up to
 * 0x1000 "paragraph-like" units and performs backward REP MOVSW copies.
 *
 * The original uses segmented addressing and word counts derived from AX/CL
 * shifts; this C model keeps equivalent chunked movement shape.
 */
int otrail_entry_stage1_window_slide(
    uint8_t *base_src,
    uint8_t *base_dst,
    size_t total_words,
    size_t max_words_per_pass
) {
    size_t words_remaining = total_words;
    size_t src_index = total_words;
    size_t dst_index = total_words;

    if (base_src == NULL || base_dst == NULL) {
        return 0;
    }
    if (max_words_per_pass == 0) {
        return 0;
    }

    while (words_remaining > 0) {
        size_t pass_words = words_remaining;
        if (pass_words > max_words_per_pass) {
            pass_words = max_words_per_pass;
        }

        /*
         * Backward word copy to preserve overlap semantics from REP MOVSW with
         * direction flag set.
         */
        while (pass_words > 0) {
            uint16_t v;
            pass_words--;
            src_index--;
            dst_index--;
            v = (uint16_t)base_src[src_index * 2]
                | ((uint16_t)base_src[src_index * 2 + 1] << 8);
            base_dst[dst_index * 2] = (uint8_t)(v & 0xFFu);
            base_dst[dst_index * 2 + 1] = (uint8_t)(v >> 8);
        }

        if (words_remaining > max_words_per_pass) {
            words_remaining -= max_words_per_pass;
        } else {
            words_remaining = 0;
        }
    }
    return 1;
}

/*
 * Inferred entry loader stage model from 0x126FE-0x1274D.
 *
 * This approximates the relocation/window-moving behavior seen before the
 * unpacker loop starts. It is not yet a verified binary-equivalent rewrite.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "entry_loader_model.h"

static void reverse_copy_bytes(uint8_t *dst, const uint8_t *src, size_t count) {
    while (count > 0) {
        count--;
        dst[count] = src[count];
    }
}

static uint32_t linear_addr(uint16_t segment, uint16_t offset) {
    return ((uint32_t)segment << 4) + (uint32_t)offset;
}

static void reverse_copy_words(uint8_t *dst, const uint8_t *src, size_t word_count) {
    while (word_count > 0) {
        size_t byte_index;
        uint16_t value;

        word_count--;
        byte_index = word_count * 2u;
        value = (uint16_t)src[byte_index] | ((uint16_t)src[byte_index + 1u] << 8);
        dst[byte_index] = (uint8_t)(value & 0xFFu);
        dst[byte_index + 1u] = (uint8_t)(value >> 8);
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

        src_index -= pass_words;
        dst_index -= pass_words;
        reverse_copy_words(base_dst + (dst_index * 2u), base_src + (src_index * 2u), pass_words);

        if (words_remaining > max_words_per_pass) {
            words_remaining -= max_words_per_pass;
        } else {
            words_remaining = 0;
        }
    }
    return 1;
}

int otrail_entry_loader_plan_bootstrap(
    uint16_t entry_cs,
    uint16_t stage0_segment_delta,
    uint16_t stage0_copy_bytes,
    uint16_t stage1_total_paragraphs,
    LoaderBootstrapPlan *plan
) {
    uint16_t remaining;
    uint16_t dx;
    uint16_t bx;

    if (plan == NULL) {
        return 0;
    }

    memset(plan, 0, sizeof(*plan));
    plan->entry_cs = entry_cs;
    plan->relocated_cs = (uint16_t)(entry_cs + stage0_segment_delta);
    plan->relocated_ip = 0x002Bu;
    plan->stage0_copy_bytes = stage0_copy_bytes;
    plan->stage0_segment_delta = stage0_segment_delta;
    plan->stage1_total_paragraphs = stage1_total_paragraphs;

    remaining = stage1_total_paragraphs;
    dx = entry_cs;
    bx = plan->relocated_cs;
    while (remaining > 0) {
        uint16_t move_paragraphs = remaining;
        uint16_t remaining_before = remaining;
        uint16_t dx_before = dx;
        uint16_t bx_before = bx;
        LoaderBootstrapPass *pass;

        if (move_paragraphs > 0x1000u) {
            move_paragraphs = 0x1000u;
        }
        if (plan->pass_count >= OTRAIL_ENTRY_LOADER_MAX_PASSES) {
            return 0;
        }

        remaining = (uint16_t)(remaining - move_paragraphs);
        dx = (uint16_t)(dx - move_paragraphs);
        bx = (uint16_t)(bx - move_paragraphs);

        pass = &plan->passes[plan->pass_count++];
        pass->paragraphs = move_paragraphs;
        pass->src_segment = dx;
        pass->dst_segment = bx;
        pass->remaining_before = remaining_before;
        pass->remaining_after = remaining;
        pass->src_segment_before = dx_before;
        pass->dst_segment_before = bx_before;
        pass->copy_words = (uint16_t)((uint32_t)move_paragraphs << 3);
        pass->tail_offset = (uint16_t)((((uint32_t)move_paragraphs << 4) - 2u) & 0xFFFFu);
    }

    plan->unpacker_es = dx;
    plan->unpacker_ds = bx;
    return 1;
}

int otrail_entry_loader_execute_bootstrap(
    uint8_t *mem,
    size_t mem_size,
    const LoaderBootstrapPlan *plan
) {
    LoaderWindow stage0_window;
    size_t i;

    if (mem == NULL || plan == NULL) {
        return 0;
    }

    if (linear_addr(plan->entry_cs, 0u) + plan->stage0_copy_bytes > mem_size ||
        linear_addr(plan->relocated_cs, 0u) + plan->stage0_copy_bytes > mem_size) {
        return 0;
    }

    stage0_window.src = mem + linear_addr(plan->entry_cs, 0u);
    stage0_window.dst = mem + linear_addr(plan->relocated_cs, 0u);
    stage0_window.bytes_to_move = plan->stage0_copy_bytes;
    stage0_window.chunk_limit = 0u;
    if (!otrail_entry_stage0_reloc_copy(&stage0_window)) {
        return 0;
    }

    for (i = 0; i < plan->pass_count; i++) {
        const LoaderBootstrapPass *pass = &plan->passes[i];
        uint32_t src_linear = linear_addr(pass->src_segment, 0u);
        uint32_t dst_linear = linear_addr(pass->dst_segment, 0u);
        size_t total_words = (size_t)pass->paragraphs * 8u;
        size_t total_bytes = (size_t)pass->paragraphs * 16u;

        if (src_linear + total_bytes > mem_size || dst_linear + total_bytes > mem_size) {
            return 0;
        }
        if (!otrail_entry_stage1_window_slide(
                mem + src_linear,
                mem + dst_linear,
                total_words,
                total_words
            )) {
            return 0;
        }
    }

    return 1;
}

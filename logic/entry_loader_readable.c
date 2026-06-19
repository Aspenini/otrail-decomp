/*
 * Readable entry loader model aligned to the bootstrap relocation flow.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "entry_loader_readable.h"

typedef struct ReadableLoaderSpan {
    uint8_t *dst;
    const uint8_t *src;
    size_t bytes;
} ReadableLoaderSpan;

static uint32_t linear_addr(uint16_t segment, uint16_t offset) {
    return ((uint32_t)segment << 4) + (uint32_t)offset;
}

static void copy_bytes_backward(ReadableLoaderSpan *span) {
    while (span->bytes > 0) {
        span->bytes--;
        span->dst[span->bytes] = span->src[span->bytes];
    }
}

static void copy_words_backward(ReadableLoaderSpan *span) {
    while (span->bytes > 0) {
        size_t lo_index;
        uint16_t word;

        span->bytes -= 2u;
        lo_index = span->bytes;
        word = (uint16_t)span->src[lo_index] |
            ((uint16_t)span->src[lo_index + 1u] << 8);
        span->dst[lo_index] = (uint8_t)(word & 0xFFu);
        span->dst[lo_index + 1u] = (uint8_t)(word >> 8);
    }
}

int otrail_entry_loader_readable_stage0_reloc_copy(LoaderWindow *w) {
    ReadableLoaderSpan span;

    if (w == NULL || w->src == NULL || w->dst == NULL) {
        return 0;
    }

    span.src = w->src;
    span.dst = w->dst;
    span.bytes = w->bytes_to_move;
    copy_bytes_backward(&span);
    return 1;
}

int otrail_entry_loader_readable_stage1_window_slide(
    uint8_t *base_src,
    uint8_t *base_dst,
    size_t total_words,
    size_t max_words_per_pass
) {
    size_t words_remaining = total_words;
    size_t src_word_index = total_words;
    size_t dst_word_index = total_words;

    if (base_src == NULL || base_dst == NULL || max_words_per_pass == 0) {
        return 0;
    }

    while (words_remaining > 0) {
        ReadableLoaderSpan span;
        size_t pass_words = words_remaining;

        if (pass_words > max_words_per_pass) {
            pass_words = max_words_per_pass;
        }

        src_word_index -= pass_words;
        dst_word_index -= pass_words;
        span.src = base_src + (src_word_index * 2u);
        span.dst = base_dst + (dst_word_index * 2u);
        span.bytes = pass_words * 2u;
        copy_words_backward(&span);
        words_remaining -= pass_words;
    }

    return 1;
}

int otrail_entry_loader_readable_plan_bootstrap(
    uint16_t entry_cs,
    uint16_t stage0_segment_delta,
    uint16_t stage0_copy_bytes,
    uint16_t stage1_total_paragraphs,
    LoaderBootstrapPlan *plan
) {
    uint16_t remaining;
    uint16_t source_segment;
    uint16_t target_segment;

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
    source_segment = entry_cs;
    target_segment = plan->relocated_cs;
    while (remaining > 0) {
        LoaderBootstrapPass *pass;
        uint16_t pass_paragraphs = remaining;
        uint16_t remaining_before = remaining;
        uint16_t source_segment_before = source_segment;
        uint16_t target_segment_before = target_segment;

        if (pass_paragraphs > 0x1000u) {
            pass_paragraphs = 0x1000u;
        }
        if (plan->pass_count >= OTRAIL_ENTRY_LOADER_MAX_PASSES) {
            return 0;
        }

        remaining = (uint16_t)(remaining - pass_paragraphs);
        source_segment = (uint16_t)(source_segment - pass_paragraphs);
        target_segment = (uint16_t)(target_segment - pass_paragraphs);

        pass = &plan->passes[plan->pass_count++];
        pass->paragraphs = pass_paragraphs;
        pass->src_segment = source_segment;
        pass->dst_segment = target_segment;
        pass->remaining_before = remaining_before;
        pass->remaining_after = remaining;
        pass->src_segment_before = source_segment_before;
        pass->dst_segment_before = target_segment_before;
        pass->copy_words = (uint16_t)((uint32_t)pass_paragraphs << 3);
        pass->tail_offset = (uint16_t)((((uint32_t)pass_paragraphs << 4) - 2u) & 0xFFFFu);
    }

    plan->unpacker_es = source_segment;
    plan->unpacker_ds = target_segment;
    return 1;
}

int otrail_entry_loader_readable_execute_bootstrap(
    uint8_t *mem,
    size_t mem_size,
    const LoaderBootstrapPlan *plan
) {
    LoaderWindow stage0_window;
    size_t pass_index;

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
    if (!otrail_entry_loader_readable_stage0_reloc_copy(&stage0_window)) {
        return 0;
    }

    for (pass_index = 0; pass_index < plan->pass_count; pass_index++) {
        const LoaderBootstrapPass *pass = &plan->passes[pass_index];
        uint32_t src_linear = linear_addr(pass->src_segment, 0u);
        uint32_t dst_linear = linear_addr(pass->dst_segment, 0u);
        size_t pass_bytes = (size_t)pass->paragraphs * 16u;
        size_t pass_words = pass_bytes / 2u;

        if (src_linear + pass_bytes > mem_size || dst_linear + pass_bytes > mem_size) {
            return 0;
        }
        if (!otrail_entry_loader_readable_stage1_window_slide(
                mem + src_linear,
                mem + dst_linear,
                pass_words,
                pass_words
            )) {
            return 0;
        }
    }

    return 1;
}

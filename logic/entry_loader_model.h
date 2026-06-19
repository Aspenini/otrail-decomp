/*
 * Public API for inferred entry loader stage model.
 */

#ifndef OTRAIL_ENTRY_LOADER_MODEL_H
#define OTRAIL_ENTRY_LOADER_MODEL_H

#include <stddef.h>
#include <stdint.h>

#define OTRAIL_ENTRY_LOADER_MAX_PASSES 16

typedef struct LoaderWindow {
    uint8_t *src;
    uint8_t *dst;
    size_t bytes_to_move;
    size_t chunk_limit;
} LoaderWindow;

typedef struct LoaderBootstrapPass {
    uint16_t paragraphs;
    uint16_t src_segment;
    uint16_t dst_segment;
    uint16_t remaining_before;
    uint16_t remaining_after;
    uint16_t src_segment_before;
    uint16_t dst_segment_before;
    uint16_t copy_words;
    uint16_t tail_offset;
} LoaderBootstrapPass;

typedef struct LoaderBootstrapPlan {
    uint16_t entry_cs;
    uint16_t relocated_cs;
    uint16_t relocated_ip;
    uint16_t unpacker_ds;
    uint16_t unpacker_es;
    uint16_t stage0_copy_bytes;
    uint16_t stage0_segment_delta;
    uint16_t stage1_total_paragraphs;
    size_t pass_count;
    LoaderBootstrapPass passes[OTRAIL_ENTRY_LOADER_MAX_PASSES];
} LoaderBootstrapPlan;

int otrail_entry_stage0_reloc_copy(LoaderWindow *w);

int otrail_entry_stage1_window_slide(
    uint8_t *base_src,
    uint8_t *base_dst,
    size_t total_words,
    size_t max_words_per_pass
);

int otrail_entry_loader_plan_bootstrap(
    uint16_t entry_cs,
    uint16_t stage0_segment_delta,
    uint16_t stage0_copy_bytes,
    uint16_t stage1_total_paragraphs,
    LoaderBootstrapPlan *plan
);

int otrail_entry_loader_execute_bootstrap(
    uint8_t *mem,
    size_t mem_size,
    const LoaderBootstrapPlan *plan
);

#endif

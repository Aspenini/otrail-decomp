/*
 * Readable entry loader model aligned to the bootstrap relocation flow.
 */

#ifndef OTRAIL_ENTRY_LOADER_READABLE_H
#define OTRAIL_ENTRY_LOADER_READABLE_H

#include <stddef.h>
#include <stdint.h>

#include "entry_loader_model.h"

int otrail_entry_loader_readable_stage0_reloc_copy(LoaderWindow *w);

int otrail_entry_loader_readable_stage1_window_slide(
    uint8_t *base_src,
    uint8_t *base_dst,
    size_t total_words,
    size_t max_words_per_pass
);

int otrail_entry_loader_readable_plan_bootstrap(
    uint16_t entry_cs,
    uint16_t stage0_segment_delta,
    uint16_t stage0_copy_bytes,
    uint16_t stage1_total_paragraphs,
    LoaderBootstrapPlan *plan
);

int otrail_entry_loader_readable_execute_bootstrap(
    uint8_t *mem,
    size_t mem_size,
    const LoaderBootstrapPlan *plan
);

#endif

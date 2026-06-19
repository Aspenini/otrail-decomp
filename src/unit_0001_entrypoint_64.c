/*
 * Lifted semantics for entry bytes 0x126FE-0x1273D.
 *
 * This slice performs the initial self-relocation copy, then prepares the
 * first stage-1 window-slide pass that continues in unit_0002.
 */

#include <stddef.h>
#include <stdint.h>

#include "../logic/entry_loader_readable.h"

const uint8_t unit_0001_entrypoint_64_bytes[] = {
    0x06, 0x0E, 0x1F, 0x8B, 0x0E, 0x0C, 0x00, 0x8B, 0xF1, 0x4E, 0x89, 0xF7, 0x8C, 0xDB, 0x03, 0x1E,
    0x0A, 0x00, 0x8E, 0xC3, 0xFD, 0xF3, 0xA4, 0x53, 0xB8, 0x2B, 0x00, 0x50, 0xCB, 0x2E, 0x8B, 0x2E,
    0x08, 0x00, 0x8C, 0xDA, 0x89, 0xE8, 0x3D, 0x00, 0x10, 0x76, 0x03, 0xB8, 0x00, 0x10, 0x29, 0xC5,
    0x29, 0xC2, 0x29, 0xC3, 0x8E, 0xDA, 0x8E, 0xC3, 0xB1, 0x03, 0xD3, 0xE0, 0x89, 0xC1, 0xD1, 0xE0,
};

typedef struct Unit0001LoaderSlice {
    LoaderBootstrapPlan plan;
    size_t first_pass_words;
    size_t first_pass_byte_span;
    uint16_t first_pass_tail_offset;
} Unit0001LoaderSlice;

static uint32_t unit_0001_linear_addr(uint16_t segment, uint16_t offset) {
    return ((uint32_t)segment << 4) + (uint32_t)offset;
}

int unit_0001_entrypoint_prepare_loader(
    uint8_t *mem,
    size_t mem_size,
    uint16_t entry_cs,
    uint16_t stage0_segment_delta,
    uint16_t stage0_copy_bytes,
    uint16_t stage1_total_paragraphs,
    Unit0001LoaderSlice *out
) {
    LoaderWindow stage0_window;

    if (mem == NULL || out == NULL) {
        return 0;
    }
    if (!otrail_entry_loader_readable_plan_bootstrap(
            entry_cs,
            stage0_segment_delta,
            stage0_copy_bytes,
            stage1_total_paragraphs,
            &out->plan
        )) {
        return 0;
    }
    if (out->plan.pass_count == 0) {
        return 0;
    }

    if (unit_0001_linear_addr(entry_cs, 0u) + stage0_copy_bytes > mem_size ||
        unit_0001_linear_addr(out->plan.relocated_cs, 0u) + stage0_copy_bytes > mem_size) {
        return 0;
    }

    stage0_window.src = mem + unit_0001_linear_addr(entry_cs, 0u);
    stage0_window.dst = mem + unit_0001_linear_addr(out->plan.relocated_cs, 0u);
    stage0_window.bytes_to_move = out->plan.stage0_copy_bytes;
    stage0_window.chunk_limit = 0u;
    if (!otrail_entry_loader_readable_stage0_reloc_copy(&stage0_window)) {
        return 0;
    }

    out->first_pass_words = out->plan.passes[0].copy_words;
    out->first_pass_byte_span = out->first_pass_words * 2u;
    out->first_pass_tail_offset = out->plan.passes[0].tail_offset;
    return 1;
}

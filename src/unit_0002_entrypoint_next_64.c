/*
 * Lifted semantics for entry bytes 0x1273E-0x1277D.
 *
 * This slice finishes the relocation/window-slide loop, restores forward
 * string state, and seeds the unpacker's first control word from DS:0.
 */

#include <stddef.h>
#include <stdint.h>

#include "../logic/entry_loader_readable.h"
#include "unit_0002_entrypoint_next_64.h"

const uint8_t unit_0002_entrypoint_next_64_bytes[] = {
    0x48, 0x48, 0x8B, 0xF0, 0x8B, 0xF8, 0xF3, 0xA5, 0x09, 0xED, 0x75, 0xD8, 0xFC, 0x8E, 0xC2, 0x8E,
    0xDB, 0x31, 0xF6, 0x31, 0xFF, 0xBA, 0x10, 0x00, 0xAD, 0x89, 0xC5, 0xD1, 0xED, 0x4A, 0x75, 0x05,
    0xAD, 0x89, 0xC5, 0xB2, 0x10, 0x73, 0x03, 0xA4, 0xEB, 0xF1, 0x31, 0xC9, 0xD1, 0xED, 0x4A, 0x75,
    0x05, 0xAD, 0x89, 0xC5, 0xB2, 0x10, 0x72, 0x22, 0xD1, 0xED, 0x4A, 0x75, 0x05, 0xAD, 0x89, 0xC5,
};

static uint32_t unit_0002_linear_addr(uint16_t segment, uint16_t offset) {
    return ((uint32_t)segment << 4) + (uint32_t)offset;
}

static uint16_t unit_0002_read_u16le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

int unit_0002_entrypoint_finish_loader(
    uint8_t *mem,
    size_t mem_size,
    const LoaderBootstrapPlan *plan,
    Unit0002LoaderHandoff *out
) {
    uint32_t stream_linear;

    if (mem == NULL || plan == NULL || out == NULL) {
        return 0;
    }
    if (!otrail_entry_loader_readable_execute_bootstrap(mem, mem_size, plan)) {
        return 0;
    }

    stream_linear = unit_0002_linear_addr(plan->unpacker_ds, 0u);
    if (stream_linear + 2u > mem_size) {
        return 0;
    }

    out->unpacker_ds = plan->unpacker_ds;
    out->unpacker_es = plan->unpacker_es;
    out->stream_src_offset = 2u;
    out->stream_dst_offset = 0u;
    out->seed_word = unit_0002_read_u16le(mem + stream_linear);
    out->seed_bits = (uint16_t)(out->seed_word >> 1);
    out->seed_bits_left = 15u;
    out->first_gate_is_literal = (uint8_t)(out->seed_word & 1u);
    return 1;
}

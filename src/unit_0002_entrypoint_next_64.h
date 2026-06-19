/*
 * Public declarations for the lifted unit_0002 loader-to-unpacker handoff.
 */

#ifndef UNIT_0002_ENTRYPOINT_NEXT_64_H
#define UNIT_0002_ENTRYPOINT_NEXT_64_H

#include <stddef.h>
#include <stdint.h>

#include "../logic/entry_loader_model.h"

typedef struct Unit0002LoaderHandoff {
    uint16_t unpacker_ds;
    uint16_t unpacker_es;
    size_t stream_src_offset;
    size_t stream_dst_offset;
    uint16_t seed_word;
    uint16_t seed_bits;
    uint8_t seed_bits_left;
    uint8_t first_gate_is_literal;
} Unit0002LoaderHandoff;

int unit_0002_entrypoint_finish_loader(
    uint8_t *mem,
    size_t mem_size,
    const LoaderBootstrapPlan *plan,
    Unit0002LoaderHandoff *out
);

#endif

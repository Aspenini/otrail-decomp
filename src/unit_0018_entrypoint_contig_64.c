/*
 * Semantic data lift for entry bytes 0x12B3E-0x12B7D.
 *
 * Post-jump payload/data continuation slice 12 after the exact
 * relocation-tail terminal handoff.  This records structured payload material
 * with per-slice byte-shape metadata; it does not claim executable code.
 */

#include <stdint.h>

const uint8_t unit_0018_entrypoint_contig_64_bytes[] = {
    0x1A, 0x08, 0x16, 0x1E, 0x08, 0x08, 0x05, 0x16, 0x1F, 0x08, 0x08, 0x05, 0x12, 0x22, 0x9A, 0x20,
    0x07, 0x12, 0x12, 0x05, 0x0B, 0x29, 0x1C, 0x12, 0x1A, 0x1F, 0x0E, 0x1B, 0x20, 0x08, 0x08, 0x16,
    0x1F, 0x08, 0x08, 0x05, 0x12, 0x22, 0x00, 0x98, 0x01, 0x20, 0x07, 0x1C, 0x05, 0x05, 0x0C, 0x1D,
    0x1A, 0x0A, 0x0A, 0x0A, 0x05, 0x12, 0x05, 0x19, 0x0B, 0x0A, 0x19, 0x1A, 0x07, 0x1C, 0x05, 0x05,
};

typedef struct Unit0018PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t zero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0018PostJumpPayloadSlice;

int unit_0018_entrypoint_describe_postjump_payload(Unit0018PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x12B3Eu;
    out->follows_offset = 0x12AFEu;
    out->byte_count = 64u;
    out->min_byte = 0x00u;
    out->max_byte = 0x9Au;
    out->nonzero_count = 63u;
    out->zero_count = 1u;
    out->printable_count = 6u;
    out->continues_terminal_payload = 1u;
    return 1;
}

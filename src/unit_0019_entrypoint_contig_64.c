/*
 * Semantic data lift for entry bytes 0x12B7E-0x12BBD.
 *
 * Post-jump payload/data continuation slice 13 after the exact
 * relocation-tail terminal handoff.  This records structured payload material
 * with per-slice byte-shape metadata; it does not claim executable code.
 */

#include <stdint.h>

const uint8_t unit_0019_entrypoint_contig_64_bytes[] = {
    0x12, 0x2F, 0x28, 0x08, 0x14, 0x0B, 0x0D, 0x3A, 0x2D, 0x16, 0x0E, 0x19, 0x1A, 0x15, 0x05, 0x17,
    0x05, 0x12, 0x22, 0x19, 0x00, 0x07, 0x01, 0x0D, 0x1D, 0x1A, 0x0A, 0x0A, 0x05, 0x12, 0x05, 0x19,
    0x2C, 0x0A, 0x0A, 0x05, 0x05, 0x05, 0x15, 0x0B, 0x0A, 0x19, 0x27, 0x1C, 0x16, 0x0A, 0x12, 0x16,
    0x0A, 0x12, 0x05, 0x20, 0x05, 0x05, 0x25, 0x4A, 0x16, 0x05, 0x08, 0x14, 0x0B, 0x0D, 0x00, 0x90,
};

typedef struct Unit0019PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t zero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0019PostJumpPayloadSlice;

int unit_0019_entrypoint_describe_postjump_payload(Unit0019PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x12B7Eu;
    out->follows_offset = 0x12B3Eu;
    out->byte_count = 64u;
    out->min_byte = 0x00u;
    out->max_byte = 0x90u;
    out->nonzero_count = 62u;
    out->zero_count = 2u;
    out->printable_count = 10u;
    out->continues_terminal_payload = 1u;
    return 1;
}

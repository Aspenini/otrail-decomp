/*
 * Semantic data lift for entry bytes 0x129FE-0x12A3D.
 *
 * Post-jump payload/data continuation slice 7 after the exact
 * relocation-tail terminal handoff.  This records structured payload material
 * with per-slice byte-shape metadata; it does not claim executable code.
 */

#include <stdint.h>

const uint8_t unit_0013_entrypoint_contig_64_bytes[] = {
    0x0E, 0x12, 0x49, 0x24, 0x09, 0x4C, 0x36, 0x37, 0x15, 0x17, 0x2F, 0x59, 0x40, 0x1B, 0x05, 0x0A,
    0x12, 0x17, 0x27, 0x14, 0x1A, 0x1A, 0x05, 0x0C, 0x21, 0x34, 0x16, 0x05, 0x05, 0x2B, 0x3D, 0x05,
    0x0E, 0x14, 0x07, 0x0C, 0x14, 0x0F, 0x11, 0x1B, 0x41, 0x40, 0x0E, 0x00, 0x28, 0x01, 0x08, 0x34,
    0xB5, 0x0B, 0x99, 0x15, 0x2A, 0x0C, 0x08, 0x5D, 0x0E, 0x0C, 0x08, 0x05, 0x0F, 0x0E, 0x0C, 0x08,
};

typedef struct Unit0013PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t zero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0013PostJumpPayloadSlice;

int unit_0013_entrypoint_describe_postjump_payload(Unit0013PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x129FEu;
    out->follows_offset = 0x129BEu;
    out->byte_count = 64u;
    out->min_byte = 0x00u;
    out->max_byte = 0xB5u;
    out->nonzero_count = 63u;
    out->zero_count = 1u;
    out->printable_count = 19u;
    out->continues_terminal_payload = 1u;
    return 1;
}

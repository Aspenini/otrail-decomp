/*
 * Semantic data lift for entry bytes 0x12CFE-0x12D3D.
 *
 * Post-jump payload/data continuation slice 19 after the exact
 * relocation-tail terminal handoff.  This records structured payload material
 * with per-slice byte-shape metadata; it does not claim executable code.
 */

#include <stdint.h>

const uint8_t unit_0025_entrypoint_contig_64_bytes[] = {
    0x10, 0x13, 0x0A, 0x0F, 0x11, 0x0B, 0x0B, 0x0A, 0x0F, 0x14, 0x10, 0x13, 0x0A, 0x0F, 0x11, 0x0B,
    0x0B, 0x0A, 0x0F, 0x11, 0x0A, 0x1A, 0x05, 0x0A, 0x0F, 0x12, 0x33, 0x14, 0x1B, 0x14, 0x26, 0x0B,
    0x0A, 0x0A, 0x0B, 0x0A, 0x05, 0x05, 0x05, 0x3F, 0x83, 0x29, 0x17, 0x12, 0x13, 0x1C, 0x2A, 0x29,
    0x27, 0x13, 0x0B, 0x12, 0x0A, 0x19, 0x37, 0x13, 0x0F, 0x0A, 0x0B, 0x19, 0x22, 0x0B, 0x0A, 0x0A,
};

typedef struct Unit0025PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t zero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0025PostJumpPayloadSlice;

int unit_0025_entrypoint_describe_postjump_payload(Unit0025PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x12CFEu;
    out->follows_offset = 0x12CBEu;
    out->byte_count = 64u;
    out->min_byte = 0x05u;
    out->max_byte = 0x83u;
    out->nonzero_count = 64u;
    out->zero_count = 0u;
    out->printable_count = 9u;
    out->continues_terminal_payload = 1u;
    return 1;
}

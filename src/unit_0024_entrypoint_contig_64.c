/*
 * Semantic data lift for entry bytes 0x12CBE-0x12CFD.
 *
 * Post-jump payload/data continuation slice 18 after the exact
 * relocation-tail terminal handoff.  This records structured payload material
 * with per-slice byte-shape metadata; it does not claim executable code.
 */

#include <stdint.h>

const uint8_t unit_0024_entrypoint_contig_64_bytes[] = {
    0x1D, 0x0C, 0x17, 0x0C, 0x05, 0x19, 0x18, 0x05, 0x0A, 0x14, 0x0B, 0x21, 0x05, 0x15, 0x17, 0x05,
    0x0A, 0x31, 0x39, 0x24, 0x09, 0x0D, 0x05, 0x1D, 0x09, 0x0D, 0x05, 0x1C, 0x05, 0x09, 0x18, 0x16,
    0x11, 0x0B, 0x0A, 0x0B, 0x05, 0x0A, 0x0F, 0x17, 0x16, 0x16, 0x11, 0x0B, 0x0B, 0x0A, 0x0F, 0x14,
    0x18, 0x0A, 0x0F, 0x11, 0x0B, 0x0B, 0x0A, 0x0F, 0x1B, 0x16, 0x11, 0x0B, 0x0B, 0x0A, 0x0F, 0x14,
};

typedef struct Unit0024PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t zero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0024PostJumpPayloadSlice;

int unit_0024_entrypoint_describe_postjump_payload(Unit0024PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x12CBEu;
    out->follows_offset = 0x12C7Eu;
    out->byte_count = 64u;
    out->min_byte = 0x05u;
    out->max_byte = 0x39u;
    out->nonzero_count = 64u;
    out->zero_count = 0u;
    out->printable_count = 4u;
    out->continues_terminal_payload = 1u;
    return 1;
}

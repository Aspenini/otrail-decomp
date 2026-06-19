/*
 * Semantic data lift for entry bytes 0x129BE-0x129FD.
 *
 * Post-jump payload/data continuation slice 6 after the exact
 * relocation-tail terminal handoff.  This records structured payload material
 * with per-slice byte-shape metadata; it does not claim executable code.
 */

#include <stdint.h>

const uint8_t unit_0012_entrypoint_contig_64_bytes[] = {
    0x05, 0x1D, 0x3A, 0x28, 0x13, 0x1A, 0x0A, 0x0E, 0x17, 0x18, 0x15, 0x1C, 0x13, 0x0A, 0x0E, 0x17,
    0x30, 0x10, 0x09, 0x14, 0x09, 0x1C, 0x10, 0x09, 0x09, 0x22, 0x06, 0x09, 0x0B, 0x22, 0x10, 0x2D,
    0x3B, 0x10, 0x24, 0x13, 0x23, 0x10, 0x0D, 0x05, 0x05, 0x1B, 0x0D, 0x6D, 0x1B, 0x18, 0x0C, 0x0E,
    0x38, 0x2A, 0x0C, 0x13, 0x2A, 0x39, 0x0A, 0x16, 0x05, 0x12, 0x39, 0x0A, 0x0E, 0x12, 0x16, 0x0A,
};

typedef struct Unit0012PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t zero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0012PostJumpPayloadSlice;

int unit_0012_entrypoint_describe_postjump_payload(Unit0012PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x129BEu;
    out->follows_offset = 0x1297Eu;
    out->byte_count = 64u;
    out->min_byte = 0x05u;
    out->max_byte = 0x6Du;
    out->nonzero_count = 64u;
    out->zero_count = 0u;
    out->printable_count = 15u;
    out->continues_terminal_payload = 1u;
    return 1;
}

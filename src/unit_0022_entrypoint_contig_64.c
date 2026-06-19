/*
 * Semantic data lift for entry bytes 0x12C3E-0x12C7D.
 *
 * Post-jump payload/data continuation slice 16 after the exact
 * relocation-tail terminal handoff.  This records structured payload material
 * with per-slice byte-shape metadata; it does not claim executable code.
 */

#include <stdint.h>

const uint8_t unit_0022_entrypoint_contig_64_bytes[] = {
    0x1D, 0xB0, 0x09, 0x36, 0x2D, 0x2C, 0x1E, 0x11, 0x11, 0x00, 0x95, 0x02, 0x31, 0x45, 0x26, 0x18,
    0x18, 0x1A, 0x10, 0x89, 0x39, 0xB7, 0x09, 0x05, 0x11, 0xA3, 0x40, 0x11, 0x54, 0x75, 0x0D, 0x00,
    0x44, 0x01, 0xDB, 0x45, 0x1B, 0x16, 0x15, 0x9F, 0x4A, 0x79, 0x4C, 0x1A, 0x8F, 0x11, 0x00, 0x9A,
    0x01, 0x51, 0x29, 0x3D, 0x20, 0xF2, 0x3F, 0x0E, 0x00, 0x38, 0x01, 0x13, 0x0F, 0x18, 0x1E, 0x10,
};

typedef struct Unit0022PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t zero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0022PostJumpPayloadSlice;

int unit_0022_entrypoint_describe_postjump_payload(Unit0022PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x12C3Eu;
    out->follows_offset = 0x12BFEu;
    out->byte_count = 64u;
    out->min_byte = 0x00u;
    out->max_byte = 0xF2u;
    out->nonzero_count = 60u;
    out->zero_count = 4u;
    out->printable_count = 21u;
    out->continues_terminal_payload = 1u;
    return 1;
}

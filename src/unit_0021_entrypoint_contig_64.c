/*
 * Semantic data lift for entry bytes 0x12BFE-0x12C3D.
 *
 * Post-jump payload/data continuation slice 15 after the exact
 * relocation-tail terminal handoff.  This records structured payload material
 * with per-slice byte-shape metadata; it does not claim executable code.
 */

#include <stdint.h>

const uint8_t unit_0021_entrypoint_contig_64_bytes[] = {
    0x0D, 0x67, 0x0A, 0x0A, 0x0A, 0x0A, 0x0F, 0x1D, 0x05, 0x05, 0x2D, 0x18, 0x0B, 0x05, 0x0A, 0x0A,
    0x0A, 0x0A, 0x0A, 0x0A, 0x05, 0x15, 0x15, 0x19, 0x19, 0x32, 0x0A, 0x05, 0x22, 0x0A, 0x05, 0x12,
    0x0B, 0x16, 0x10, 0x05, 0x0A, 0x05, 0x14, 0xA8, 0x00, 0x31, 0x01, 0xD0, 0xC8, 0xF5, 0x12, 0x12,
    0x12, 0x1A, 0x1A, 0x05, 0x0E, 0x11, 0x7E, 0x53, 0x4B, 0x2A, 0x1C, 0x0C, 0x4C, 0x84, 0x43, 0x0F,
};

typedef struct Unit0021PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t zero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0021PostJumpPayloadSlice;

int unit_0021_entrypoint_describe_postjump_payload(Unit0021PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x12BFEu;
    out->follows_offset = 0x12BBEu;
    out->byte_count = 64u;
    out->min_byte = 0x00u;
    out->max_byte = 0xF5u;
    out->nonzero_count = 63u;
    out->zero_count = 1u;
    out->printable_count = 11u;
    out->continues_terminal_payload = 1u;
    return 1;
}

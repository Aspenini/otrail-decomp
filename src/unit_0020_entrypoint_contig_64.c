/*
 * Semantic data lift for entry bytes 0x12BBE-0x12BFD.
 *
 * Post-jump payload/data continuation slice 14 after the exact
 * relocation-tail terminal handoff.  This records structured payload material
 * with per-slice byte-shape metadata; it does not claim executable code.
 */

#include <stdint.h>

const uint8_t unit_0020_entrypoint_contig_64_bytes[] = {
    0x01, 0x09, 0x1C, 0x05, 0x05, 0x0E, 0x1C, 0x05, 0x05, 0x0E, 0x1C, 0x05, 0x05, 0x1B, 0x30, 0x05,
    0x29, 0x1E, 0x19, 0x20, 0x37, 0x10, 0x2E, 0x22, 0x22, 0x09, 0x08, 0x2B, 0x10, 0x09, 0x2B, 0x0B,
    0x29, 0x0E, 0x2A, 0x11, 0x09, 0x36, 0x15, 0x1C, 0x05, 0x36, 0x11, 0x09, 0x0F, 0x18, 0x12, 0x0E,
    0x1D, 0x26, 0x16, 0x24, 0x05, 0x05, 0x0B, 0x12, 0x00, 0x7F, 0x01, 0x09, 0x0D, 0x0F, 0x0C, 0x0B,
};

typedef struct Unit0020PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t zero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0020PostJumpPayloadSlice;

int unit_0020_entrypoint_describe_postjump_payload(Unit0020PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x12BBEu;
    out->follows_offset = 0x12B7Eu;
    out->byte_count = 64u;
    out->min_byte = 0x00u;
    out->max_byte = 0x7Fu;
    out->nonzero_count = 63u;
    out->zero_count = 1u;
    out->printable_count = 15u;
    out->continues_terminal_payload = 1u;
    return 1;
}

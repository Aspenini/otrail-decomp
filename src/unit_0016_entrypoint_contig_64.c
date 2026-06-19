/*
 * Semantic data lift for entry bytes 0x12ABE-0x12AFD.
 *
 * Post-jump payload/data continuation slice 10 after the exact
 * relocation-tail terminal handoff.  This records structured payload material
 * with per-slice byte-shape metadata; it does not claim executable code.
 */

#include <stdint.h>

const uint8_t unit_0016_entrypoint_contig_64_bytes[] = {
    0x2D, 0x2F, 0x08, 0x0F, 0x18, 0x18, 0x18, 0x18, 0x18, 0x0B, 0x09, 0x05, 0x3E, 0x18, 0x18, 0x0A,
    0x0A, 0x05, 0x05, 0xFD, 0x30, 0x0C, 0x16, 0x0C, 0x0B, 0x0D, 0x0D, 0x15, 0x0C, 0x18, 0x0B, 0x05,
    0x0A, 0x0B, 0x05, 0x0A, 0x0B, 0x05, 0x0A, 0x0B, 0x05, 0x05, 0x0C, 0x12, 0x12, 0x13, 0x19, 0x1C,
    0x13, 0x14, 0x12, 0x0B, 0x15, 0x14, 0x46, 0x0C, 0x0C, 0x0C, 0x13, 0x07, 0x08, 0x0C, 0x44, 0x12,
};

typedef struct Unit0016PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t zero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0016PostJumpPayloadSlice;

int unit_0016_entrypoint_describe_postjump_payload(Unit0016PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x12ABEu;
    out->follows_offset = 0x12A7Eu;
    out->byte_count = 64u;
    out->min_byte = 0x05u;
    out->max_byte = 0xFDu;
    out->nonzero_count = 64u;
    out->zero_count = 0u;
    out->printable_count = 6u;
    out->continues_terminal_payload = 1u;
    return 1;
}

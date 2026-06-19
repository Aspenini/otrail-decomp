/*
 * Semantic data lift for entry bytes 0x12AFE-0x12B3D.
 *
 * Post-jump payload/data continuation slice 11 after the exact
 * relocation-tail terminal handoff.  This records structured payload material
 * with per-slice byte-shape metadata; it does not claim executable code.
 */

#include <stdint.h>

const uint8_t unit_0017_entrypoint_contig_64_bytes[] = {
    0x0F, 0x45, 0x1A, 0x0B, 0x32, 0x0B, 0x1E, 0x0B, 0x0E, 0x2D, 0x0B, 0x19, 0x05, 0x17, 0x18, 0x1D,
    0x08, 0x0D, 0x0C, 0x05, 0x05, 0x0D, 0x28, 0x1D, 0x08, 0x05, 0x21, 0x11, 0x2E, 0x29, 0x20, 0x00,
    0x57, 0x01, 0x2D, 0x1C, 0x35, 0x14, 0x08, 0x08, 0x19, 0x08, 0x19, 0x25, 0x0C, 0x14, 0x08, 0x08,
    0x1C, 0x0B, 0x0E, 0x0E, 0x08, 0x12, 0x1F, 0x0A, 0x12, 0x16, 0x0A, 0x12, 0x1F, 0x19, 0x08, 0x19,
};

typedef struct Unit0017PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t zero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0017PostJumpPayloadSlice;

int unit_0017_entrypoint_describe_postjump_payload(Unit0017PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x12AFEu;
    out->follows_offset = 0x12ABEu;
    out->byte_count = 64u;
    out->min_byte = 0x00u;
    out->max_byte = 0x57u;
    out->nonzero_count = 63u;
    out->zero_count = 1u;
    out->printable_count = 12u;
    out->continues_terminal_payload = 1u;
    return 1;
}

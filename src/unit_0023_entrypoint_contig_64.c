/*
 * Semantic data lift for entry bytes 0x12C7E-0x12CBD.
 *
 * Post-jump payload/data continuation slice 17 after the exact
 * relocation-tail terminal handoff.  This records structured payload material
 * with per-slice byte-shape metadata; it does not claim executable code.
 */

#include <stdint.h>

const uint8_t unit_0023_entrypoint_contig_64_bytes[] = {
    0x14, 0x0D, 0x0D, 0x19, 0x0C, 0x05, 0x1D, 0x1B, 0x21, 0x10, 0x0B, 0x0A, 0x0B, 0x0A, 0x0F, 0x19,
    0x0A, 0x0F, 0x3B, 0x11, 0x0A, 0x0B, 0x0A, 0x0F, 0x1C, 0x0A, 0x0A, 0x0F, 0x05, 0x0F, 0x1E, 0x0D,
    0x15, 0x0F, 0x13, 0x14, 0x12, 0x09, 0x15, 0x1A, 0x49, 0x14, 0x0E, 0x0A, 0x12, 0x45, 0x16, 0x21,
    0x16, 0x1A, 0x00, 0x5E, 0x02, 0x19, 0x12, 0x05, 0x05, 0x0F, 0x14, 0x0B, 0x06, 0x0C, 0x05, 0x11,
};

typedef struct Unit0023PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t zero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0023PostJumpPayloadSlice;

int unit_0023_entrypoint_describe_postjump_payload(Unit0023PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x12C7Eu;
    out->follows_offset = 0x12C3Eu;
    out->byte_count = 64u;
    out->min_byte = 0x00u;
    out->max_byte = 0x5Eu;
    out->nonzero_count = 63u;
    out->zero_count = 1u;
    out->printable_count = 6u;
    out->continues_terminal_payload = 1u;
    return 1;
}

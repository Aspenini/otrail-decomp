/*
 * Semantic data lift for entry bytes 0x128BE-0x128FD.
 *
 * This slice continues the post-jump payload/data stream immediately after
 * unit_0007.  It remains byte-authoritative, but now has a named descriptor so
 * the region is tracked as understood payload material instead of anonymous
 * authored bytes.
 */

#include <stdint.h>

const uint8_t unit_0008_entrypoint_contig_64_bytes[] = {
    0x0E, 0x22, 0x0A, 0x0A, 0x15, 0x0A, 0x17, 0x0A, 0x0A, 0x0A, 0x05, 0x05, 0x05, 0x3A, 0x1D, 0x12,
    0x17, 0x14, 0x28, 0x42, 0x19, 0x15, 0x1F, 0x1B, 0x18, 0x14, 0x15, 0x1F, 0x1B, 0x18, 0x15, 0x15,
    0x15, 0x09, 0x0C, 0x19, 0x05, 0x13, 0x05, 0x13, 0x05, 0x17, 0x18, 0x0A, 0x05, 0x1A, 0x18, 0x0A,
    0x05, 0x17, 0x18, 0x0A, 0x05, 0x0C, 0x11, 0x18, 0x0B, 0x1D, 0x4D, 0x15, 0x37, 0x21, 0x0D, 0x14,
};

typedef struct Unit0008PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0008PostJumpPayloadSlice;

int unit_0008_entrypoint_describe_postjump_payload(Unit0008PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x128BEu;
    out->follows_offset = 0x1287Eu;
    out->byte_count = 64u;
    out->min_byte = 0x05u;
    out->max_byte = 0x4Du;
    out->nonzero_count = 64u;
    out->printable_count = 7u;
    out->continues_terminal_payload = 1u;
    return 1;
}

/*
 * Semantic data lift for entry bytes 0x128FE-0x1293D.
 *
 * Third full 64-byte post-jump payload/data continuation after the exact
 * relocation-tail terminal handoff.  The descriptor records the slice shape
 * without claiming this payload material is executable code.
 */

#include <stdint.h>

const uint8_t unit_0009_entrypoint_contig_64_bytes[] = {
    0x12, 0x48, 0x11, 0x1A, 0x10, 0x0F, 0x2F, 0x18, 0x0B, 0x1E, 0x11, 0x0A, 0x0B, 0x0A, 0x0F, 0x18,
    0x0A, 0x0F, 0x13, 0x0A, 0x0F, 0x0E, 0x22, 0x05, 0x2F, 0x13, 0x20, 0x1E, 0x1B, 0x23, 0x15, 0x1D,
    0x0A, 0x0F, 0x26, 0x05, 0x05, 0x49, 0x20, 0x12, 0x0B, 0x0A, 0x05, 0x18, 0x11, 0x05, 0x2A, 0x28,
    0x16, 0x05, 0x05, 0x40, 0x05, 0x0C, 0x14, 0x05, 0x05, 0x43, 0x05, 0x0C, 0x62, 0x1E, 0x0B, 0x2F,
};

typedef struct Unit0009PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0009PostJumpPayloadSlice;

int unit_0009_entrypoint_describe_postjump_payload(Unit0009PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x128FEu;
    out->follows_offset = 0x128BEu;
    out->byte_count = 64u;
    out->min_byte = 0x05u;
    out->max_byte = 0x62u;
    out->nonzero_count = 64u;
    out->printable_count = 15u;
    out->continues_terminal_payload = 1u;
    return 1;
}

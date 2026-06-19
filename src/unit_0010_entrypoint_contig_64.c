/*
 * Semantic data lift for entry bytes 0x1293E-0x1297D.
 *
 * Fourth full 64-byte post-jump payload/data continuation.  The embedded zero
 * at byte offset +0x29 is now recorded by the descriptor, making this a checked
 * named data slice rather than an anonymous byte-array promotion.
 */

#include <stdint.h>

const uint8_t unit_0010_entrypoint_contig_64_bytes[] = {
    0x0A, 0x0C, 0x22, 0x06, 0x0F, 0x23, 0x29, 0x0A, 0x13, 0x05, 0x0F, 0x1E, 0x45, 0x26, 0x13, 0x05,
    0x05, 0x3D, 0x2C, 0x0E, 0x20, 0x0A, 0x1B, 0x0C, 0x63, 0x1B, 0x17, 0x1B, 0x05, 0x11, 0x16, 0x0A,
    0x12, 0x16, 0x0A, 0x12, 0x11, 0x05, 0x16, 0x1A, 0x05, 0x00, 0x32, 0x01, 0x3F, 0x3C, 0x0F, 0x1E,
    0x1A, 0x1A, 0x1B, 0x2E, 0x0A, 0x0A, 0x0A, 0x05, 0x05, 0x16, 0x0A, 0x0C, 0x19, 0x05, 0x0C, 0x1A,
};

typedef struct Unit0010PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t printable_count;
    uint8_t zero_count;
    uint8_t continues_terminal_payload;
} Unit0010PostJumpPayloadSlice;

int unit_0010_entrypoint_describe_postjump_payload(Unit0010PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x1293Eu;
    out->follows_offset = 0x128FEu;
    out->byte_count = 64u;
    out->min_byte = 0x00u;
    out->max_byte = 0x63u;
    out->nonzero_count = 63u;
    out->printable_count = 13u;
    out->zero_count = 1u;
    out->continues_terminal_payload = 1u;
    return 1;
}

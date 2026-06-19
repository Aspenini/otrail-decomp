/*
 * Semantic data lift for entry bytes 0x1287E-0x128BD.
 *
 * This is the first full 64-byte continuation after the terminal handoff's
 * post-jump payload/data prefix in unit_0006.  It is not executable loader
 * code; the descriptor below pins its role and byte-shape as structured
 * post-jump payload material.
 */

#include <stdint.h>

const uint8_t unit_0007_entrypoint_contig_64_bytes[] = {
    0x21, 0x13, 0x0D, 0x15, 0x08, 0x15, 0x15, 0x15, 0x0C, 0x0F, 0x0D, 0x0B, 0x0D, 0x0C, 0x0B, 0x10,
    0x0B, 0x05, 0x47, 0x05, 0x05, 0x15, 0x05, 0x14, 0x0B, 0x12, 0x1C, 0x25, 0x46, 0x2C, 0x22, 0x15,
    0x0A, 0x05, 0x05, 0x14, 0x0C, 0x0D, 0x05, 0x09, 0x0D, 0x0B, 0x05, 0x19, 0x45, 0x5B, 0x09, 0x0D,
    0x05, 0x17, 0xF5, 0x09, 0x0D, 0x17, 0x1A, 0x12, 0x0A, 0x0E, 0x1A, 0x17, 0x0A, 0x0E, 0x12, 0x0A,
};

typedef struct Unit0007PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0007PostJumpPayloadSlice;

int unit_0007_entrypoint_describe_postjump_payload(Unit0007PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x1287Eu;
    out->follows_offset = 0x12848u;
    out->byte_count = 64u;
    out->min_byte = 0x05u;
    out->max_byte = 0xF5u;
    out->nonzero_count = 64u;
    out->printable_count = 8u;
    out->continues_terminal_payload = 1u;
    return 1;
}

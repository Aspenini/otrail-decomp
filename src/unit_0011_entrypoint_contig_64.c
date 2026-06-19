/*
 * Semantic data lift for entry bytes 0x1297E-0x129BD.
 *
 * Post-jump payload/data continuation slice 5 after the exact
 * relocation-tail terminal handoff.  This records structured payload material
 * with per-slice byte-shape metadata; it does not claim executable code.
 */

#include <stdint.h>

const uint8_t unit_0011_entrypoint_contig_64_bytes[] = {
    0x28, 0x0A, 0x0A, 0x05, 0x05, 0x17, 0x20, 0x0F, 0x19, 0x20, 0x0E, 0x0A, 0x0A, 0x0A, 0x05, 0x05,
    0x32, 0x31, 0x31, 0x11, 0x07, 0x0C, 0x19, 0x20, 0x0A, 0x0A, 0x05, 0x05, 0x05, 0x05, 0x3C, 0x29,
    0x2A, 0x0D, 0x0D, 0x37, 0x0A, 0x12, 0x1A, 0x0A, 0x12, 0x16, 0x0A, 0x12, 0x11, 0x05, 0x05, 0x11,
    0x09, 0x0E, 0x0D, 0x0C, 0x3D, 0x24, 0x05, 0x0C, 0x1F, 0x05, 0x05, 0x21, 0x1C, 0x10, 0x0D, 0x05,
};

typedef struct Unit0011PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t zero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0011PostJumpPayloadSlice;

int unit_0011_entrypoint_describe_postjump_payload(Unit0011PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x1297Eu;
    out->follows_offset = 0x1293Eu;
    out->byte_count = 64u;
    out->min_byte = 0x05u;
    out->max_byte = 0x3Du;
    out->nonzero_count = 64u;
    out->zero_count = 0u;
    out->printable_count = 14u;
    out->continues_terminal_payload = 1u;
    return 1;
}

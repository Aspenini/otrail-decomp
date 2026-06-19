/*
 * Semantic data lift for entry bytes 0x12D3E-0x12D7D.
 *
 * Post-jump payload/data continuation slice 20 after the exact
 * relocation-tail terminal handoff.  This records structured payload material
 * with per-slice byte-shape metadata; it does not claim executable code.
 */

#include <stdint.h>

const uint8_t unit_0026_entrypoint_contig_64_bytes[] = {
    0x05, 0x05, 0x05, 0x09, 0x2E, 0x18, 0x10, 0x1A, 0x15, 0x0B, 0x12, 0x14, 0x0C, 0x15, 0x0D, 0xC8,
    0x28, 0x09, 0x0A, 0x22, 0x11, 0x0B, 0x0A, 0x19, 0x19, 0x11, 0x0B, 0x15, 0x1D, 0x1A, 0x0C, 0x14,
    0x11, 0x0B, 0x0A, 0x18, 0x12, 0x14, 0x12, 0x12, 0xC7, 0x0E, 0x05, 0x10, 0x1A, 0x1A, 0x1A, 0x1A,
    0x0D, 0x09, 0x08, 0x1A, 0x0C, 0x34, 0x22, 0x24, 0x22, 0x0E, 0x0E, 0x0C, 0x20, 0x14, 0x29, 0x05,
};

typedef struct Unit0026PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t zero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0026PostJumpPayloadSlice;

int unit_0026_entrypoint_describe_postjump_payload(Unit0026PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x12D3Eu;
    out->follows_offset = 0x12CFEu;
    out->byte_count = 64u;
    out->min_byte = 0x05u;
    out->max_byte = 0xC8u;
    out->nonzero_count = 64u;
    out->zero_count = 0u;
    out->printable_count = 9u;
    out->continues_terminal_payload = 1u;
    return 1;
}

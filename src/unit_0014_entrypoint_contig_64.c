/*
 * Semantic data lift for entry bytes 0x12A3E-0x12A7D.
 *
 * Post-jump payload/data continuation slice 8 after the exact
 * relocation-tail terminal handoff.  This records structured payload material
 * with per-slice byte-shape metadata; it does not claim executable code.
 */

#include <stdint.h>

const uint8_t unit_0014_entrypoint_contig_64_bytes[] = {
    0x05, 0x14, 0x15, 0x12, 0x08, 0x24, 0x05, 0x0C, 0x10, 0x0E, 0x19, 0x2C, 0x0C, 0x70, 0x31, 0x18,
    0x22, 0x0A, 0x22, 0x0A, 0x0A, 0x05, 0x0B, 0x15, 0x05, 0x13, 0x00, 0x4F, 0x01, 0x09, 0x0D, 0x09,
    0x1B, 0x0A, 0x05, 0x0B, 0x15, 0x19, 0x34, 0x05, 0x05, 0x0C, 0x0D, 0x0E, 0x1B, 0x17, 0x05, 0x0A,
    0x05, 0x0B, 0x0A, 0x19, 0x1B, 0x1B, 0x23, 0x05, 0x31, 0x6F, 0x95, 0x08, 0x21, 0x0F, 0x0D, 0x39,
};

typedef struct Unit0014PostJumpPayloadSlice {
    uint32_t original_offset;
    uint32_t follows_offset;
    uint8_t byte_count;
    uint8_t min_byte;
    uint8_t max_byte;
    uint8_t nonzero_count;
    uint8_t zero_count;
    uint8_t printable_count;
    uint8_t continues_terminal_payload;
} Unit0014PostJumpPayloadSlice;

int unit_0014_entrypoint_describe_postjump_payload(Unit0014PostJumpPayloadSlice *out) {
    if (out == 0) {
        return 0;
    }

    out->original_offset = 0x12A3Eu;
    out->follows_offset = 0x129FEu;
    out->byte_count = 64u;
    out->min_byte = 0x00u;
    out->max_byte = 0x95u;
    out->nonzero_count = 63u;
    out->zero_count = 1u;
    out->printable_count = 13u;
    out->continues_terminal_payload = 1u;
    return 1;
}

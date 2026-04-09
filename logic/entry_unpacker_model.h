/*
 * Public API for inferred OTrail entry unpacker model.
 */

#ifndef OTRAIL_ENTRY_UNPACKER_MODEL_H
#define OTRAIL_ENTRY_UNPACKER_MODEL_H

#include <stddef.h>
#include <stdint.h>

typedef struct OtrailUnpackStats {
    size_t literal_ops;
    size_t short_copy_ops;
    size_t long_copy_ops;
    size_t copied_bytes;
    int fail_code;
    size_t fail_src_pos;
    size_t fail_out_pos;
} OtrailUnpackStats;

enum {
    OTRAIL_UNPACK_STRICT = 0,
    OTRAIL_UNPACK_HEURISTIC = 1
};

int otrail_entry_unpacker_inferred_mode(
    const uint8_t *src,
    size_t src_len,
    uint8_t *dst,
    size_t dst_cap,
    size_t *src_used,
    size_t *dst_written,
    int mode,
    OtrailUnpackStats *stats
);

int otrail_entry_unpacker_inferred_ex(
    const uint8_t *src,
    size_t src_len,
    uint8_t *dst,
    size_t dst_cap,
    size_t *src_used,
    size_t *dst_written,
    OtrailUnpackStats *stats
);

int otrail_entry_unpacker_inferred(
    const uint8_t *src,
    size_t src_len,
    uint8_t *dst,
    size_t dst_cap,
    size_t *src_used,
    size_t *dst_written
);

#endif

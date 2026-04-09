/*
 * Public API wrappers for inferred entry unpacker model.
 */

#include <stddef.h>
#include <stdint.h>

#include "entry_unpacker_internal.h"
#include "entry_unpacker_model.h"

int otrail_entry_unpacker_inferred_mode(
    const uint8_t *src,
    size_t src_len,
    uint8_t *dst,
    size_t dst_cap,
    size_t *src_used,
    size_t *dst_written,
    int mode,
    OtrailUnpackStats *stats
) {
    return otrail_entry_unpacker_run(
        src,
        src_len,
        dst,
        dst_cap,
        src_used,
        dst_written,
        mode,
        stats
    );
}

int otrail_entry_unpacker_inferred_ex(
    const uint8_t *src,
    size_t src_len,
    uint8_t *dst,
    size_t dst_cap,
    size_t *src_used,
    size_t *dst_written,
    OtrailUnpackStats *stats
) {
    return otrail_entry_unpacker_run(
        src,
        src_len,
        dst,
        dst_cap,
        src_used,
        dst_written,
        OTRAIL_UNPACK_HEURISTIC,
        stats
    );
}

int otrail_entry_unpacker_inferred(
    const uint8_t *src,
    size_t src_len,
    uint8_t *dst,
    size_t dst_cap,
    size_t *src_used,
    size_t *dst_written
) {
    return otrail_entry_unpacker_run(
        src,
        src_len,
        dst,
        dst_cap,
        src_used,
        dst_written,
        OTRAIL_UNPACK_STRICT,
        NULL
    );
}

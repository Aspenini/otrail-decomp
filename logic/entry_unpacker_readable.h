/*
 * Readable, block-aligned unpacker model.
 */

#ifndef OTRAIL_ENTRY_UNPACKER_READABLE_H
#define OTRAIL_ENTRY_UNPACKER_READABLE_H

#include <stddef.h>
#include <stdint.h>

#include "entry_unpacker_model.h"

int otrail_entry_unpacker_readable_mode(
    const uint8_t *src,
    size_t src_len,
    uint8_t *dst,
    size_t dst_cap,
    size_t *src_used,
    size_t *dst_written,
    int mode,
    OtrailUnpackStats *stats
);

int otrail_entry_unpacker_readable_ex(
    const uint8_t *src,
    size_t src_len,
    uint8_t *dst,
    size_t dst_cap,
    size_t *src_used,
    size_t *dst_written,
    OtrailUnpackStats *stats
);

int otrail_entry_unpacker_readable(
    const uint8_t *src,
    size_t src_len,
    uint8_t *dst,
    size_t dst_cap,
    size_t *src_used,
    size_t *dst_written
);

#endif

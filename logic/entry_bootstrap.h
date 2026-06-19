/*
 * Reusable entry bootstrap helpers for MZ load, loader relocation, and unpacker handoff.
 */

#ifndef OTRAIL_ENTRY_BOOTSTRAP_H
#define OTRAIL_ENTRY_BOOTSTRAP_H

#include <stddef.h>
#include <stdint.h>

#include "entry_loader_model.h"
#include "entry_unpacker_model.h"

#define OTRAIL_ENTRY_BOOTSTRAP_MEM_SIZE 0x100000u

typedef enum OtrailEntryBootstrapModel {
    OTRAIL_ENTRY_BOOTSTRAP_INFERRED = 0,
    OTRAIL_ENTRY_BOOTSTRAP_READABLE = 1
} OtrailEntryBootstrapModel;

typedef struct OtrailMzHeader {
    uint16_t header_paragraphs;
    uint16_t entry_ip;
    uint16_t entry_cs_rel;
} OtrailMzHeader;

typedef struct OtrailEntryBootstrapResult {
    OtrailMzHeader mz;
    LoaderBootstrapPlan plan;
    uint16_t load_seg;
    uint16_t entry_cs;
    uint32_t entry_linear;
    uint32_t stream_linear;
    int unpack_ok;
    size_t src_used;
    size_t dst_written;
    OtrailUnpackStats unpack_stats;
} OtrailEntryBootstrapResult;

typedef struct OtrailEntryLoaderPrepareResult {
    OtrailMzHeader mz;
    LoaderBootstrapPlan plan;
    uint16_t load_seg;
    uint16_t entry_cs;
    uint32_t entry_linear;
} OtrailEntryLoaderPrepareResult;

int otrail_mz_parse_header(
    const uint8_t *exe,
    size_t exe_size,
    OtrailMzHeader *header
);

int otrail_entry_bootstrap_prepare_loader(
    const uint8_t *exe,
    size_t exe_size,
    uint16_t load_seg,
    OtrailEntryBootstrapModel model,
    uint8_t *mem,
    size_t mem_size,
    OtrailEntryLoaderPrepareResult *result
);

int otrail_entry_bootstrap_run_exe(
    const uint8_t *exe,
    size_t exe_size,
    uint16_t load_seg,
    int mode,
    OtrailEntryBootstrapModel model,
    uint8_t *mem,
    size_t mem_size,
    uint8_t *dst,
    size_t dst_cap,
    OtrailEntryBootstrapResult *result
);

#endif

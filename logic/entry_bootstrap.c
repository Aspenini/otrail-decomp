/*
 * Reusable entry bootstrap implementation.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "entry_bootstrap.h"
#include "entry_loader_readable.h"
#include "entry_unpacker_readable.h"

static uint16_t read_u16le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t linear_addr(uint16_t segment, uint16_t offset) {
    return ((uint32_t)segment << 4) + (uint32_t)offset;
}

int otrail_mz_parse_header(
    const uint8_t *exe,
    size_t exe_size,
    OtrailMzHeader *header
) {
    if (exe == NULL || header == NULL || exe_size < 0x1Cu) {
        return 0;
    }

    header->header_paragraphs = read_u16le(exe + 0x08);
    header->entry_ip = read_u16le(exe + 0x14);
    header->entry_cs_rel = read_u16le(exe + 0x16);
    return 1;
}

static int load_mz_image(
    const uint8_t *exe,
    size_t exe_size,
    uint16_t load_seg,
    const OtrailMzHeader *header,
    uint8_t *mem,
    size_t mem_size
) {
    size_t image_offset;
    size_t image_size;
    uint32_t load_linear;

    if (exe == NULL || header == NULL || mem == NULL) {
        return 0;
    }

    image_offset = (size_t)header->header_paragraphs * 16u;
    if (image_offset >= exe_size) {
        return 0;
    }

    image_size = exe_size - image_offset;
    load_linear = linear_addr(load_seg, 0u);
    if (load_linear + image_size > mem_size) {
        return 0;
    }

    memset(mem, 0, mem_size);
    memcpy(mem + load_linear, exe + image_offset, image_size);
    return 1;
}

int otrail_entry_bootstrap_prepare_loader(
    const uint8_t *exe,
    size_t exe_size,
    uint16_t load_seg,
    OtrailEntryBootstrapModel model,
    uint8_t *mem,
    size_t mem_size,
    OtrailEntryLoaderPrepareResult *result
) {
    uint16_t entry_cs;
    uint32_t entry_linear;
    uint16_t stage1_total_paragraphs;
    uint16_t stage0_segment_delta;
    uint16_t stage0_copy_bytes;

    if (exe == NULL || mem == NULL || result == NULL) {
        return 0;
    }
    if (!otrail_mz_parse_header(exe, exe_size, &result->mz)) {
        return 0;
    }
    if (!load_mz_image(exe, exe_size, load_seg, &result->mz, mem, mem_size)) {
        return 0;
    }

    entry_cs = (uint16_t)(load_seg + result->mz.entry_cs_rel);
    entry_linear = linear_addr(entry_cs, result->mz.entry_ip);
    if (entry_linear + 14u > mem_size) {
        return 0;
    }

    stage1_total_paragraphs = read_u16le(mem + linear_addr(entry_cs, 0x08u));
    stage0_segment_delta = read_u16le(mem + linear_addr(entry_cs, 0x0Au));
    stage0_copy_bytes = read_u16le(mem + linear_addr(entry_cs, 0x0Cu));
    if (model == OTRAIL_ENTRY_BOOTSTRAP_READABLE) {
        if (!otrail_entry_loader_readable_plan_bootstrap(
                entry_cs,
                stage0_segment_delta,
                stage0_copy_bytes,
                stage1_total_paragraphs,
                &result->plan
            )) {
            return 0;
        }
    } else {
        if (!otrail_entry_loader_plan_bootstrap(
                entry_cs,
                stage0_segment_delta,
                stage0_copy_bytes,
                stage1_total_paragraphs,
                &result->plan
            )) {
            return 0;
        }
    }

    result->load_seg = load_seg;
    result->entry_cs = entry_cs;
    result->entry_linear = entry_linear;
    return 1;
}

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
) {
    OtrailEntryLoaderPrepareResult prepared;
    uint32_t stream_linear;

    if (exe == NULL || mem == NULL || dst == NULL || result == NULL) {
        return 0;
    }
    if (!otrail_entry_bootstrap_prepare_loader(
            exe,
            exe_size,
            load_seg,
            model,
            mem,
            mem_size,
            &prepared
        )) {
        return 0;
    }

    result->mz = prepared.mz;
    result->plan = prepared.plan;
    if (model == OTRAIL_ENTRY_BOOTSTRAP_READABLE) {
        if (!otrail_entry_loader_readable_execute_bootstrap(mem, mem_size, &result->plan)) {
            return 0;
        }
    } else {
        if (!otrail_entry_loader_execute_bootstrap(mem, mem_size, &result->plan)) {
            return 0;
        }
    }

    stream_linear = linear_addr(result->plan.unpacker_ds, 0u);
    if (stream_linear >= mem_size) {
        return 0;
    }

    memset(&result->unpack_stats, 0, sizeof(result->unpack_stats));
    if (model == OTRAIL_ENTRY_BOOTSTRAP_READABLE) {
        result->unpack_ok = otrail_entry_unpacker_readable_mode(
            mem + stream_linear,
            mem_size - stream_linear,
            dst,
            dst_cap,
            &result->src_used,
            &result->dst_written,
            mode,
            &result->unpack_stats
        );
    } else {
        result->unpack_ok = otrail_entry_unpacker_inferred_mode(
            mem + stream_linear,
            mem_size - stream_linear,
            dst,
            dst_cap,
            &result->src_used,
            &result->dst_written,
            mode,
            &result->unpack_stats
        );
    }

    result->load_seg = prepared.load_seg;
    result->entry_cs = prepared.entry_cs;
    result->entry_linear = prepared.entry_linear;
    result->stream_linear = stream_linear;
    return 1;
}

/*
 * Named descriptors for checked post-unpack runtime regions.
 */

#include "unpacked_runtime_map.h"

#include <string.h>

const OtrailRuntimeRegionDescriptor otrail_runtime_0f46_regions[] = {
    {
        "window_0eff",
        "dense_0f46",
        "source_envelope",
        0x0EFFu,
        0x0FABu,
        OTRAIL_RUNTIME_REGION_SOURCE_ENVELOPE,
        "Large source envelope behind the 0x0F20/0x0F46 family.",
    },
    {
        "window_0f20",
        "dense_0f46",
        "source_neighborhood",
        0x0F20u,
        0x0F96u,
        OTRAIL_RUNTIME_REGION_SOURCE_NEIGHBORHOOD,
        "Structured source neighborhood containing the dense 0x0F46 root.",
    },
    {
        "window_0f46",
        "dense_0f46",
        "root_window",
        0x0F46u,
        0x0F96u,
        OTRAIL_RUNTIME_REGION_ROOT_WINDOW,
        "Dense 0x0F46 root candidate window.",
    },
    {
        "window_2a3a",
        "dense_0f46",
        "source_envelope",
        0x2A3Au,
        0x2B1Au,
        OTRAIL_RUNTIME_REGION_SOURCE_ENVELOPE,
        "Second-level source envelope around the 0x2A6E clone.",
    },
    {
        "window_2a6e",
        "dense_0f46",
        "exact_clone",
        0x2A6Eu,
        0x2B1Au,
        OTRAIL_RUNTIME_REGION_EXACT_CLONE,
        "Exact clone of the 0x0EFF source envelope.",
    },
    {
        "window_2a8f",
        "dense_0f46",
        "exact_clone",
        0x2A8Fu,
        0x2B05u,
        OTRAIL_RUNTIME_REGION_EXACT_CLONE,
        "Exact clone of the 0x0F20 source neighborhood.",
    },
    {
        "window_2ab5",
        "dense_0f46",
        "exact_clone",
        0x2AB5u,
        0x2B05u,
        OTRAIL_RUNTIME_REGION_EXACT_CLONE,
        "Exact clone of the 0x0F46 dense root.",
    },
    {
        "window_2c7f",
        "dense_0f46",
        "exact_clone_head",
        0x2C7Fu,
        0x2D3Fu,
        OTRAIL_RUNTIME_REGION_EXACT_CLONE_HEAD,
        "Exact copy of the checked 0x2A3A envelope head.",
    },
    {
        "window_2cfa",
        "dense_0f46",
        "near_clone",
        0x2CFAu,
        0x2D4Au,
        OTRAIL_RUNTIME_REGION_NEAR_CLONE,
        "Near-clone of the 0x0F46 dense root with a local tail variant.",
    },
};

const size_t otrail_runtime_0f46_region_count =
    sizeof(otrail_runtime_0f46_regions) / sizeof(otrail_runtime_0f46_regions[0]);

size_t otrail_runtime_region_size(const OtrailRuntimeRegionDescriptor *region) {
    if (!region || region->end < region->start) {
        return 0u;
    }
    return (size_t)(region->end - region->start);
}

const char *otrail_runtime_region_kind_name(OtrailRuntimeRegionKind kind) {
    switch (kind) {
        case OTRAIL_RUNTIME_REGION_SOURCE_ENVELOPE:
            return "source_envelope";
        case OTRAIL_RUNTIME_REGION_SOURCE_NEIGHBORHOOD:
            return "source_neighborhood";
        case OTRAIL_RUNTIME_REGION_ROOT_WINDOW:
            return "root_window";
        case OTRAIL_RUNTIME_REGION_EXACT_CLONE:
            return "exact_clone";
        case OTRAIL_RUNTIME_REGION_EXACT_CLONE_HEAD:
            return "exact_clone_head";
        case OTRAIL_RUNTIME_REGION_NEAR_CLONE:
            return "near_clone";
    }
    return "unknown";
}

const OtrailRuntimeRegionDescriptor *otrail_runtime_find_0f46_region(
    const char *id
) {
    size_t i;

    if (!id) {
        return 0;
    }
    for (i = 0; i < otrail_runtime_0f46_region_count; i++) {
        if (strcmp(otrail_runtime_0f46_regions[i].id, id) == 0) {
            return &otrail_runtime_0f46_regions[i];
        }
    }
    return 0;
}

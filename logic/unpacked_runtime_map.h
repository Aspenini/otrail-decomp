/*
 * Named descriptors for checked post-unpack runtime regions.
 */

#ifndef OTRAIL_UNPACKED_RUNTIME_MAP_H
#define OTRAIL_UNPACKED_RUNTIME_MAP_H

#include <stddef.h>
#include <stdint.h>

typedef enum OtrailRuntimeRegionKind {
    OTRAIL_RUNTIME_REGION_SOURCE_ENVELOPE,
    OTRAIL_RUNTIME_REGION_SOURCE_NEIGHBORHOOD,
    OTRAIL_RUNTIME_REGION_ROOT_WINDOW,
    OTRAIL_RUNTIME_REGION_EXACT_CLONE,
    OTRAIL_RUNTIME_REGION_EXACT_CLONE_HEAD,
    OTRAIL_RUNTIME_REGION_NEAR_CLONE,
} OtrailRuntimeRegionKind;

typedef struct OtrailRuntimeRegionDescriptor {
    const char *id;
    const char *family;
    const char *role;
    uint16_t start;
    uint16_t end;
    OtrailRuntimeRegionKind kind;
    const char *purpose;
} OtrailRuntimeRegionDescriptor;

extern const OtrailRuntimeRegionDescriptor otrail_runtime_0f46_regions[];
extern const size_t otrail_runtime_0f46_region_count;

size_t otrail_runtime_region_size(const OtrailRuntimeRegionDescriptor *region);

const char *otrail_runtime_region_kind_name(OtrailRuntimeRegionKind kind);

const OtrailRuntimeRegionDescriptor *otrail_runtime_find_0f46_region(
    const char *id
);

#endif

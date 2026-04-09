/*
 * Scan candidate payload offsets using the inferred entry unpacker model.
 *
 * This harness brute-forces start offsets in a selected range and reports
 * successful decodes with simple behavior signals (expansion ratio and
 * literal/copy operation counts).
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../logic/entry_unpacker_model.h"

typedef struct ScanResult {
    int ok;
    uint32_t offset;
    size_t src_used;
    size_t dst_written;
    OtrailUnpackStats stats;
} ScanResult;

static int cmp_result_desc(const void *a, const void *b) {
    const ScanResult *ra = (const ScanResult *)a;
    const ScanResult *rb = (const ScanResult *)b;
    double ratio_a = (ra->src_used == 0) ? 0.0 : (double)ra->dst_written / (double)ra->src_used;
    double ratio_b = (rb->src_used == 0) ? 0.0 : (double)rb->dst_written / (double)rb->src_used;
    if (ratio_a < ratio_b) return 1;
    if (ratio_a > ratio_b) return -1;
    if (ra->dst_written < rb->dst_written) return 1;
    if (ra->dst_written > rb->dst_written) return -1;
    return 0;
}

int main(int argc, char **argv) {
    const char *path = "Oregon_The_1990/OREGON.EXE";
    uint32_t scan_start = 0x126C0;
    uint32_t scan_end = 0x12880;
    int mode = OTRAIL_UNPACK_HEURISTIC;
    const size_t dst_cap = 65536;
    uint8_t *file_buf = NULL;
    uint8_t *dst = NULL;
    FILE *f = NULL;
    long file_size_long;
    size_t file_size;
    uint32_t off;
    ScanResult *results = NULL;
    size_t result_count = 0;
    size_t result_cap = 0;
    size_t i;
    size_t show_n;

    if (argc >= 2) {
        path = argv[1];
    }
    if (argc >= 4) {
        scan_start = (uint32_t)strtoul(argv[2], NULL, 0);
        scan_end = (uint32_t)strtoul(argv[3], NULL, 0);
    }
    if (argc >= 5) {
        mode = (int)strtol(argv[4], NULL, 0);
    }

    f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open %s\n", path);
        return 1;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 1;
    }
    file_size_long = ftell(f);
    if (file_size_long <= 0) {
        fclose(f);
        return 1;
    }
    file_size = (size_t)file_size_long;
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return 1;
    }

    file_buf = (uint8_t *)malloc(file_size);
    dst = (uint8_t *)malloc(dst_cap);
    if (!file_buf || !dst) {
        fclose(f);
        free(file_buf);
        free(dst);
        return 1;
    }
    if (fread(file_buf, 1, file_size, f) != file_size) {
        fclose(f);
        free(file_buf);
        free(dst);
        return 1;
    }
    fclose(f);

    if (scan_end > file_size) {
        scan_end = (uint32_t)file_size;
    }

    for (off = scan_start; off + 4 < scan_end; off++) {
        size_t src_used = 0;
        size_t dst_written = 0;
        OtrailUnpackStats stats;
        int ok = otrail_entry_unpacker_inferred_mode(
            file_buf + off,
            (size_t)(scan_end - off),
            dst,
            dst_cap,
            &src_used,
            &dst_written,
            mode,
            &stats
        );

        /* Keep only "plausible" decode candidates. */
        if ((ok || dst_written >= 64) && src_used >= 8 && dst_written >= 16) {
            if (result_count == result_cap) {
                size_t new_cap = (result_cap == 0) ? 64 : result_cap * 2;
                ScanResult *tmp = (ScanResult *)realloc(results, new_cap * sizeof(ScanResult));
                if (!tmp) {
                    free(results);
                    free(file_buf);
                    free(dst);
                    return 1;
                }
                results = tmp;
                result_cap = new_cap;
            }
            results[result_count].ok = ok;
            results[result_count].offset = off;
            results[result_count].src_used = src_used;
            results[result_count].dst_written = dst_written;
            results[result_count].stats = stats;
            result_count++;
        }
    }

    qsort(results, result_count, sizeof(ScanResult), cmp_result_desc);
    show_n = (result_count < 20) ? result_count : 20;

    printf("scan_start=0x%X\n", scan_start);
    printf("scan_end=0x%X\n", scan_end);
    printf("mode=%s\n", mode == OTRAIL_UNPACK_STRICT ? "strict" : "heuristic");
    printf("candidate_count=%zu\n", result_count);
    printf("showing_top=%zu\n", show_n);
    printf("status,offset_hex,src_used,dst_written,ratio,literals,short_copy,long_copy,copied_bytes\n");
    for (i = 0; i < show_n; i++) {
        const ScanResult *r = &results[i];
        double ratio = (double)r->dst_written / (double)r->src_used;
        printf(
            "%s,0x%X,%zu,%zu,%.3f,%zu,%zu,%zu,%zu\n",
            r->ok ? "ok" : "partial",
            r->offset,
            r->src_used,
            r->dst_written,
            ratio,
            r->stats.literal_ops,
            r->stats.short_copy_ops,
            r->stats.long_copy_ops,
            r->stats.copied_bytes
        );
    }

    free(results);
    free(file_buf);
    free(dst);
    return 0;
}

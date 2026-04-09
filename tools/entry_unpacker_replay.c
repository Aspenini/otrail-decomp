/*
 * Replay inferred unpacker from a single start offset and dump output.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../logic/entry_unpacker_model.h"

static void print_hex_preview(const uint8_t *buf, size_t len, size_t max_bytes) {
    size_t i;
    size_t n = (len < max_bytes) ? len : max_bytes;
    for (i = 0; i < n; i++) {
        printf("%02x", buf[i]);
        if ((i + 1) % 2 == 0 && i + 1 < n) {
            printf(" ");
        }
    }
    if (len > n) {
        printf(" ...");
    }
    printf("\n");
}

int main(int argc, char **argv) {
    const char *path = "Oregon_The_1990/OREGON.EXE";
    uint32_t offset = 0x1274F;
    size_t dst_cap = 65536;
    int mode = OTRAIL_UNPACK_HEURISTIC;
    uint8_t *file_buf = NULL;
    uint8_t *dst = NULL;
    FILE *f = NULL;
    long file_size_long;
    size_t file_size;
    size_t src_used = 0;
    size_t dst_written = 0;
    OtrailUnpackStats stats;
    int ok;
    char out_path[256];
    FILE *outf;

    if (argc >= 2) {
        path = argv[1];
    }
    if (argc >= 3) {
        offset = (uint32_t)strtoul(argv[2], NULL, 0);
    }
    if (argc >= 4) {
        dst_cap = (size_t)strtoull(argv[3], NULL, 0);
        if (dst_cap == 0) {
            dst_cap = 65536;
        }
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
    if (offset >= file_size) {
        fprintf(stderr, "Offset out of range: 0x%X\n", offset);
        fclose(f);
        return 1;
    }
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

    ok = otrail_entry_unpacker_inferred_mode(
        file_buf + offset,
        file_size - offset,
        dst,
        dst_cap,
        &src_used,
        &dst_written,
        mode,
        &stats
    );

    snprintf(out_path, sizeof(out_path), "build/replay_0x%X.bin", offset);
    outf = fopen(out_path, "wb");
    if (!outf) {
        free(file_buf);
        free(dst);
        return 1;
    }
    if (dst_written > 0 && fwrite(dst, 1, dst_written, outf) != dst_written) {
        fclose(outf);
        free(file_buf);
        free(dst);
        return 1;
    }
    fclose(outf);

    printf("status=%s\n", ok ? "ok" : "partial");
    printf("mode=%s\n", mode == OTRAIL_UNPACK_STRICT ? "strict" : "heuristic");
    printf("offset=0x%X\n", offset);
    printf("src_used=%zu\n", src_used);
    printf("dst_written=%zu\n", dst_written);
    printf("ratio=%.3f\n", src_used ? ((double)dst_written / (double)src_used) : 0.0);
    printf("literal_ops=%zu\n", stats.literal_ops);
    printf("short_copy_ops=%zu\n", stats.short_copy_ops);
    printf("long_copy_ops=%zu\n", stats.long_copy_ops);
    printf("copied_bytes=%zu\n", stats.copied_bytes);
    printf("fail_code=%d\n", stats.fail_code);
    printf("fail_src_pos=%zu\n", stats.fail_src_pos);
    printf("fail_out_pos=%zu\n", stats.fail_out_pos);
    printf("output_file=%s\n", out_path);
    printf("preview_hex=");
    print_hex_preview(dst, dst_written, 64);

    free(file_buf);
    free(dst);
    return 0;
}

/*
 * Simulate entry loader relocation and replay the unpacker from post-loader DS:0.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../logic/entry_bootstrap.h"
#include "../logic/entry_unpacker_model.h"

#define OTRAIL_BOOTSTRAP_OUTPUT_CAP 65536u

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

static int format_pass_schedule(const LoaderBootstrapPlan *plan, char *out, size_t out_size) {
    size_t i;
    size_t used = 0;

    if (out_size == 0) {
        return 0;
    }
    out[0] = '\0';
    for (i = 0; i < plan->pass_count; i++) {
        int written = snprintf(
            out + used,
            out_size - used,
            "%s0x%04X",
            (i == 0) ? "" : ",",
            plan->passes[i].paragraphs
        );
        if (written < 0 || (size_t)written >= out_size - used) {
            return 0;
        }
        used += (size_t)written;
    }
    return 1;
}

static int format_pass_trace(const LoaderBootstrapPlan *plan, char *out, size_t out_size) {
    size_t i;
    size_t used = 0;

    if (out_size == 0) {
        return 0;
    }
    out[0] = '\0';
    for (i = 0; i < plan->pass_count; i++) {
        const LoaderBootstrapPass *pass = &plan->passes[i];
        int written = snprintf(
            out + used,
            out_size - used,
            "%sBP=%04X->%04X DX=%04X->%04X BX=%04X->%04X CX=%04X SI=DI=%04X",
            (i == 0) ? "" : ";",
            pass->remaining_before,
            pass->remaining_after,
            pass->src_segment_before,
            pass->src_segment,
            pass->dst_segment_before,
            pass->dst_segment,
            pass->copy_words,
            pass->tail_offset
        );
        if (written < 0 || (size_t)written >= out_size - used) {
            return 0;
        }
        used += (size_t)written;
    }
    return 1;
}

int main(int argc, char **argv) {
    const char *path = "Oregon_The_1990/OREGON.EXE";
    const char *model = "inferred";
    uint16_t load_seg = 0xA000u;
    int mode = OTRAIL_UNPACK_HEURISTIC;
    OtrailEntryBootstrapModel bootstrap_model = OTRAIL_ENTRY_BOOTSTRAP_INFERRED;
    FILE *f = NULL;
    long exe_size_long;
    size_t exe_size;
    uint8_t *exe = NULL;
    uint8_t *mem = NULL;
    uint8_t *dst = NULL;
    OtrailEntryBootstrapResult result;
    char out_path[256];
    char pass_schedule[128];
    char pass_trace[256];
    FILE *outf = NULL;

    if (argc >= 2) {
        path = argv[1];
    }
    if (argc >= 3) {
        load_seg = (uint16_t)strtoul(argv[2], NULL, 0);
    }
    if (argc >= 4) {
        mode = (int)strtol(argv[3], NULL, 0);
    }
    if (argc >= 5) {
        model = argv[4];
    }
    if (strcmp(model, "readable") == 0) {
        bootstrap_model = OTRAIL_ENTRY_BOOTSTRAP_READABLE;
    } else if (strcmp(model, "inferred") != 0) {
        fprintf(stderr, "Unknown model selector: %s\n", model);
        return 1;
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
    exe_size_long = ftell(f);
    if (exe_size_long <= 0) {
        fclose(f);
        return 1;
    }
    exe_size = (size_t)exe_size_long;
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return 1;
    }

    exe = (uint8_t *)malloc(exe_size);
    mem = (uint8_t *)malloc(OTRAIL_ENTRY_BOOTSTRAP_MEM_SIZE);
    dst = (uint8_t *)malloc(OTRAIL_BOOTSTRAP_OUTPUT_CAP);
    if (exe == NULL || mem == NULL || dst == NULL) {
        fclose(f);
        free(exe);
        free(mem);
        free(dst);
        return 1;
    }
    if (fread(exe, 1, exe_size, f) != exe_size) {
        fclose(f);
        free(exe);
        free(mem);
        free(dst);
        return 1;
    }
    fclose(f);

    if (!otrail_entry_bootstrap_run_exe(
            exe,
            exe_size,
            load_seg,
            mode,
            bootstrap_model,
            mem,
            OTRAIL_ENTRY_BOOTSTRAP_MEM_SIZE,
            dst,
            OTRAIL_BOOTSTRAP_OUTPUT_CAP,
            &result
        )) {
        fprintf(stderr, "Failed to execute composed bootstrap path\n");
        free(exe);
        free(mem);
        free(dst);
        return 1;
    }

    if (!format_pass_schedule(&result.plan, pass_schedule, sizeof(pass_schedule))) {
        fprintf(stderr, "Failed to format pass schedule\n");
        free(exe);
        free(mem);
        free(dst);
        return 1;
    }
    if (!format_pass_trace(&result.plan, pass_trace, sizeof(pass_trace))) {
        fprintf(stderr, "Failed to format pass trace\n");
        free(exe);
        free(mem);
        free(dst);
        return 1;
    }

    snprintf(
        out_path,
        sizeof(out_path),
        "build/entry_bootstrap_replay_%s_%s.bin",
        model,
        mode == OTRAIL_UNPACK_STRICT ? "strict" : "heuristic"
    );
    outf = fopen(out_path, "wb");
    if (!outf) {
        free(exe);
        free(mem);
        free(dst);
        return 1;
    }
    if (result.dst_written > 0 &&
        fwrite(dst, 1, result.dst_written, outf) != result.dst_written) {
        fclose(outf);
        free(exe);
        free(mem);
        free(dst);
        return 1;
    }
    fclose(outf);

    printf("status=%s\n", result.unpack_ok ? "ok" : "partial");
    printf("model=%s\n", model);
    printf("mode=%s\n", mode == OTRAIL_UNPACK_STRICT ? "strict" : "heuristic");
    printf("load_seg=0x%04X\n", load_seg);
    printf("entry_cs=0x%04X\n", result.entry_cs);
    printf("entry_ip=0x%04X\n", result.mz.entry_ip);
    printf("entry_linear=0x%05X\n", result.entry_linear);
    printf("stage0_copy_bytes=0x%04X\n", result.plan.stage0_copy_bytes);
    printf("stage0_segment_delta=0x%04X\n", result.plan.stage0_segment_delta);
    printf("stage1_total_paragraphs=0x%04X\n", result.plan.stage1_total_paragraphs);
    printf("relocated_cs=0x%04X\n", result.plan.relocated_cs);
    printf("relocated_ip=0x%04X\n", result.plan.relocated_ip);
    printf("unpacker_ds=0x%04X\n", result.plan.unpacker_ds);
    printf("unpacker_es=0x%04X\n", result.plan.unpacker_es);
    printf("pass_count=%zu\n", result.plan.pass_count);
    printf("pass_schedule=%s\n", pass_schedule);
    printf("pass_trace=%s\n", pass_trace);
    printf("stream_linear=0x%05X\n", result.stream_linear);
    printf("src_used=%zu\n", result.src_used);
    printf("dst_written=%zu\n", result.dst_written);
    printf(
        "ratio=%.3f\n",
        result.src_used ? ((double)result.dst_written / (double)result.src_used) : 0.0
    );
    printf("literal_ops=%zu\n", result.unpack_stats.literal_ops);
    printf("short_copy_ops=%zu\n", result.unpack_stats.short_copy_ops);
    printf("long_copy_ops=%zu\n", result.unpack_stats.long_copy_ops);
    printf("copied_bytes=%zu\n", result.unpack_stats.copied_bytes);
    printf("fail_code=%d\n", result.unpack_stats.fail_code);
    printf("fail_src_pos=%zu\n", result.unpack_stats.fail_src_pos);
    printf("fail_out_pos=%zu\n", result.unpack_stats.fail_out_pos);
    printf("output_file=%s\n", out_path);
    printf("stream_preview_hex=");
    print_hex_preview(mem + result.stream_linear, 64u, 64u);
    printf("preview_hex=");
    print_hex_preview(dst, result.dst_written, 64u);

    free(exe);
    free(mem);
    free(dst);
    return 0;
}

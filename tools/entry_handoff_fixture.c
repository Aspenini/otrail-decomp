/*
 * Deterministic fixture runner for the lifted unit_0002 loader-to-unpacker handoff.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../logic/entry_bootstrap.h"
#include "../src/unit_0002_entrypoint_next_64.h"

static int read_file(const char *path, uint8_t **out_buf, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    long size_long;
    uint8_t *buf;

    if (f == NULL) {
        return 0;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    size_long = ftell(f);
    if (size_long <= 0) {
        fclose(f);
        return 0;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return 0;
    }

    buf = (uint8_t *)malloc((size_t)size_long);
    if (buf == NULL) {
        fclose(f);
        return 0;
    }
    if (fread(buf, 1, (size_t)size_long, f) != (size_t)size_long) {
        fclose(f);
        free(buf);
        return 0;
    }
    fclose(f);
    *out_buf = buf;
    *out_size = (size_t)size_long;
    return 1;
}

int main(int argc, char **argv) {
    const char *path = "Oregon_The_1990/OREGON.EXE";
    uint16_t load_seg = 0xA000u;
    uint8_t *exe = NULL;
    size_t exe_size = 0;
    uint8_t *mem = NULL;
    OtrailEntryLoaderPrepareResult prepared;
    Unit0002LoaderHandoff handoff;
    int ok;

    if (argc >= 2) {
        path = argv[1];
    }
    if (argc >= 3) {
        load_seg = (uint16_t)strtoul(argv[2], NULL, 0);
    }

    if (!read_file(path, &exe, &exe_size)) {
        fprintf(stderr, "failed to read %s\n", path);
        return 1;
    }

    mem = (uint8_t *)malloc(OTRAIL_ENTRY_BOOTSTRAP_MEM_SIZE);
    if (mem == NULL) {
        free(exe);
        return 1;
    }

    ok = otrail_entry_bootstrap_prepare_loader(
        exe,
        exe_size,
        load_seg,
        OTRAIL_ENTRY_BOOTSTRAP_READABLE,
        mem,
        OTRAIL_ENTRY_BOOTSTRAP_MEM_SIZE,
        &prepared
    );
    if (ok) {
        ok = unit_0002_entrypoint_finish_loader(
            mem,
            OTRAIL_ENTRY_BOOTSTRAP_MEM_SIZE,
            &prepared.plan,
            &handoff
        );
    }

    printf("status=%s\n", ok ? "ok" : "fail");
    if (ok) {
        printf("load_seg=0x%04X\n", prepared.load_seg);
        printf("entry_cs=0x%04X\n", prepared.entry_cs);
        printf("entry_ip=0x%04X\n", prepared.mz.entry_ip);
        printf("entry_linear=0x%05X\n", prepared.entry_linear);
        printf("stage0_copy_bytes=0x%04X\n", prepared.plan.stage0_copy_bytes);
        printf("stage0_segment_delta=0x%04X\n", prepared.plan.stage0_segment_delta);
        printf("stage1_total_paragraphs=0x%04X\n", prepared.plan.stage1_total_paragraphs);
        printf("unpacker_ds=0x%04X\n", handoff.unpacker_ds);
        printf("unpacker_es=0x%04X\n", handoff.unpacker_es);
        printf("stream_src_offset=%zu\n", handoff.stream_src_offset);
        printf("stream_dst_offset=%zu\n", handoff.stream_dst_offset);
        printf("seed_word=0x%04X\n", handoff.seed_word);
        printf("seed_bits=0x%04X\n", handoff.seed_bits);
        printf("seed_bits_left=%u\n", handoff.seed_bits_left);
        printf("first_gate_is_literal=%u\n", handoff.first_gate_is_literal);
    }

    free(mem);
    free(exe);
    return ok ? 0 : 1;
}

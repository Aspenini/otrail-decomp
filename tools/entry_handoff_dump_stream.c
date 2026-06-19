/*
 * Dump the prepared DS:0 unpacker stream reached by the lifted unit_0002 handoff.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static uint32_t linear_addr(uint16_t segment, uint16_t offset) {
    return ((uint32_t)segment << 4) + (uint32_t)offset;
}

int main(int argc, char **argv) {
    const char *path = "Oregon_The_1990/OREGON.EXE";
    const char *out_path = "build/entry_handoff_stream.bin";
    uint16_t load_seg = 0xA000u;
    uint8_t *exe = NULL;
    size_t exe_size = 0;
    uint8_t *mem = NULL;
    OtrailEntryLoaderPrepareResult prepared;
    Unit0002LoaderHandoff handoff;
    FILE *outf = NULL;
    uint32_t stream_linear;
    size_t stream_len;
    int ok;

    if (argc >= 2) {
        path = argv[1];
    }
    if (argc >= 3) {
        load_seg = (uint16_t)strtoul(argv[2], NULL, 0);
    }
    if (argc >= 4) {
        out_path = argv[3];
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
    if (!ok) {
        free(mem);
        free(exe);
        return 1;
    }

    stream_linear = linear_addr(handoff.unpacker_ds, 0u);
    stream_len = OTRAIL_ENTRY_BOOTSTRAP_MEM_SIZE - (size_t)stream_linear;
    outf = fopen(out_path, "wb");
    if (outf == NULL) {
        free(mem);
        free(exe);
        return 1;
    }
    if (fwrite(mem + stream_linear, 1, stream_len, outf) != stream_len) {
        fclose(outf);
        free(mem);
        free(exe);
        return 1;
    }
    fclose(outf);

    printf("status=ok\n");
    printf("output_file=%s\n", out_path);
    printf("stream_linear=0x%05X\n", stream_linear);
    printf("stream_len=%zu\n", stream_len);
    printf("seed_word=0x%04X\n", handoff.seed_word);
    printf("seed_bits=0x%04X\n", handoff.seed_bits);
    printf("first_gate_is_literal=%u\n", handoff.first_gate_is_literal);

    free(mem);
    free(exe);
    return 0;
}

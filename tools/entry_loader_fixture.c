/*
 * Deterministic fixture runner for the inferred entry loader model.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../logic/entry_loader_model.h"
#include "../logic/entry_loader_readable.h"

static void fill_pattern(uint8_t *buf, size_t len, uint32_t seed) {
    size_t i;
    uint32_t state = seed & 0xFFu;

    for (i = 0; i < len; i++) {
        state = (state * 109u + 73u + (uint32_t)i) & 0xFFu;
        buf[i] = (uint8_t)state;
    }
}

static void print_buffer_hex(const uint8_t *buf, size_t len) {
    size_t i;
    for (i = 0; i < len; i++) {
        printf("%02x", buf[i]);
    }
    printf("\n");
}

int main(int argc, char **argv) {
    const char *model = "inferred";
    const char *stage;
    int argi = 1;
    size_t buffer_size;
    uint32_t pattern_seed;
    size_t src_offset;
    size_t dst_offset;
    uint8_t *buffer;
    int ok = 0;

    if (argc >= 2 && (strcmp(argv[1], "inferred") == 0 || strcmp(argv[1], "readable") == 0)) {
        model = argv[1];
        argi++;
    }

    if (argc - argi < 6) {
        fprintf(
            stderr,
            "usage: %s [inferred|readable] stage0 <buffer_size> <seed> <src_offset> <dst_offset> <bytes_to_move>\n"
            "       %s [inferred|readable] stage1 <buffer_size> <seed> <src_offset> <dst_offset> <total_words> <max_words_per_pass>\n",
            argv[0],
            argv[0]
        );
        return 1;
    }

    stage = argv[argi];
    buffer_size = (size_t)strtoull(argv[argi + 1], NULL, 0);
    pattern_seed = (uint32_t)strtoul(argv[argi + 2], NULL, 0);
    src_offset = (size_t)strtoull(argv[argi + 3], NULL, 0);
    dst_offset = (size_t)strtoull(argv[argi + 4], NULL, 0);

    buffer = (uint8_t *)malloc(buffer_size);
    if (buffer == NULL) {
        fprintf(stderr, "failed to allocate %zu-byte fixture buffer\n", buffer_size);
        return 1;
    }

    fill_pattern(buffer, buffer_size, pattern_seed);

    if (strcmp(stage, "stage0") == 0) {
        size_t bytes_to_move = (size_t)strtoull(argv[argi + 5], NULL, 0);
        LoaderWindow window;

        if (src_offset + bytes_to_move > buffer_size || dst_offset + bytes_to_move > buffer_size) {
            fprintf(stderr, "stage0 offsets exceed buffer bounds\n");
            free(buffer);
            return 1;
        }

        window.src = buffer + src_offset;
        window.dst = buffer + dst_offset;
        window.bytes_to_move = bytes_to_move;
        window.chunk_limit = 0;
        if (strcmp(model, "readable") == 0) {
            ok = otrail_entry_loader_readable_stage0_reloc_copy(&window);
        } else {
            ok = otrail_entry_stage0_reloc_copy(&window);
        }
    } else if (strcmp(stage, "stage1") == 0) {
        size_t total_words;
        size_t max_words_per_pass;
        size_t byte_span;

        if (argc - argi < 7) {
            fprintf(stderr, "stage1 requires total_words and max_words_per_pass\n");
            free(buffer);
            return 1;
        }

        total_words = (size_t)strtoull(argv[argi + 5], NULL, 0);
        max_words_per_pass = (size_t)strtoull(argv[argi + 6], NULL, 0);
        byte_span = total_words * 2u;
        if (src_offset + byte_span > buffer_size || dst_offset + byte_span > buffer_size) {
            fprintf(stderr, "stage1 offsets exceed buffer bounds\n");
            free(buffer);
            return 1;
        }

        if (strcmp(model, "readable") == 0) {
            ok = otrail_entry_loader_readable_stage1_window_slide(
                buffer + src_offset,
                buffer + dst_offset,
                total_words,
                max_words_per_pass
            );
        } else {
            ok = otrail_entry_stage1_window_slide(
                buffer + src_offset,
                buffer + dst_offset,
                total_words,
                max_words_per_pass
            );
        }
    } else {
        fprintf(stderr, "unknown stage: %s\n", stage);
        free(buffer);
        return 1;
    }

    printf("status=%s\n", ok ? "ok" : "fail");
    printf("model=%s\n", model);
    printf("buffer_hex=");
    print_buffer_hex(buffer, buffer_size);

    free(buffer);
    return 0;
}

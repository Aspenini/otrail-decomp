/*
 * Deterministic fixture runner for lifted unit_0003 / unit_0004 token semantics.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../logic/entry_bootstrap.h"
#include "../src/unit_0002_entrypoint_next_64.h"
#include "../src/unit_0003_entrypoint_contig_64.h"
#include "../src/unit_0004_entrypoint_contig_64.h"

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

static int cursor_get_bit(Unit0003BitCursor *cursor, uint8_t *out_bit) {
    if (cursor->bits_left == 0u) {
        if (cursor->src_pos + 2u > cursor->src_len) {
            return 0;
        }
        cursor->bitbuf = (uint16_t)cursor->src[cursor->src_pos] |
            ((uint16_t)cursor->src[cursor->src_pos + 1u] << 8);
        cursor->src_pos += 2u;
        cursor->bits_left = 16u;
    }
    *out_bit = (uint8_t)(cursor->bitbuf & 1u);
    cursor->bitbuf >>= 1;
    cursor->bits_left--;
    return 1;
}

static int cursor_get_u8(Unit0003BitCursor *cursor, uint8_t *out_byte) {
    if (cursor->src_pos >= cursor->src_len) {
        return 0;
    }
    *out_byte = cursor->src[cursor->src_pos++];
    return 1;
}

int main(int argc, char **argv) {
    const char *path = "Oregon_The_1990/OREGON.EXE";
    uint16_t load_seg = 0xA000u;
    long target_event = 0;
    uint8_t *exe = NULL;
    size_t exe_size = 0;
    uint8_t *mem = NULL;
    OtrailEntryLoaderPrepareResult prepared;
    Unit0002LoaderHandoff handoff;
    uint32_t stream_linear;
    size_t stream_len;
    Unit0003BitCursor cursor;
    long event_index;
    int ok = 0;

    if (argc >= 2) {
        path = argv[1];
    }
    if (argc >= 3) {
        load_seg = (uint16_t)strtoul(argv[2], NULL, 0);
    }
    if (argc >= 4) {
        target_event = strtol(argv[3], NULL, 0);
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
    cursor.src = mem + stream_linear;
    cursor.src_len = stream_len;
    cursor.src_pos = handoff.stream_src_offset;
    cursor.bitbuf = handoff.seed_bits;
    cursor.bits_left = handoff.seed_bits_left;

    for (event_index = 0; event_index <= target_event; event_index++) {
        if (event_index == 0 && handoff.first_gate_is_literal) {
            uint8_t literal;

            if (!cursor_get_u8(&cursor, &literal)) {
                break;
            }
            if (event_index == target_event) {
                printf("status=ok\n");
                printf("target_event=%ld\n", target_event);
                printf("event_type=literal\n");
                printf("literal=0x%02X\n", literal);
                printf("cursor_src_pos=%zu\n", cursor.src_pos);
                printf("cursor_bitbuf=0x%04X\n", cursor.bitbuf);
                printf("cursor_bits_left=%u\n", cursor.bits_left);
                free(mem);
                free(exe);
                return 0;
            }
            continue;
        }

        uint8_t bit0;

        if (!cursor_get_bit(&cursor, &bit0)) {
            break;
        }

        if (bit0) {
            uint8_t literal;

            if (!cursor_get_u8(&cursor, &literal)) {
                break;
            }
            if (event_index == target_event) {
                printf("status=ok\n");
                printf("target_event=%ld\n", target_event);
                printf("event_type=literal\n");
                printf("literal=0x%02X\n", literal);
                printf("cursor_src_pos=%zu\n", cursor.src_pos);
                printf("cursor_bitbuf=0x%04X\n", cursor.bitbuf);
                printf("cursor_bits_left=%u\n", cursor.bits_left);
                free(mem);
                free(exe);
                return 0;
            }
            continue;
        }

        {
            uint8_t bit1;
            Unit0003TokenState token_state;

            if (!cursor_get_bit(&cursor, &bit1)) {
                break;
            }
            if (!unit_0003_entrypoint_resume_nonliteral_token(&cursor, bit1 != 0u, &token_state)) {
                break;
            }
            if (event_index == target_event) {
                printf("status=ok\n");
                printf("target_event=%ld\n", target_event);
                printf("event_type=%s\n", bit1 ? "long" : "short");
                printf("token_kind=%d\n", (int)token_state.kind);
                printf("backref_disp=%d\n", (int)token_state.backref_disp);
                printf("copy_len=%u\n", token_state.copy_len);
                printf("control_byte=%u\n", token_state.control_byte);
                printf(
                    "needs_unit_0004_resolution=%u\n",
                    token_state.needs_unit_0004_resolution
                );
                printf("cursor_src_pos=%zu\n", cursor.src_pos);
                printf("cursor_bitbuf=0x%04X\n", cursor.bitbuf);
                printf("cursor_bits_left=%u\n", cursor.bits_left);
                if (token_state.needs_unit_0004_resolution) {
                    Unit0004WindowState window = {0};
                    Unit0004ControlResult control;
                    if (!unit_0004_entrypoint_resolve_long_control(
                            token_state.control_byte,
                            &window,
                            &control
                        )) {
                        break;
                    }
                    printf("control_kind=%d\n", (int)control.kind);
                    printf("control_copy_len=%u\n", control.copy_len);
                    printf("control_reenter_top_gate=%u\n", control.reenter_top_gate);
                    printf("control_tail_target_offset=0x%X\n", control.tail_target_offset);
                    printf("control_tail_target_is_exact=%u\n", control.tail_target_is_exact);
                }
                free(mem);
                free(exe);
                return 0;
            }
        }
    }

    printf("status=fail\n");
    printf("target_event=%ld\n", target_event);
    free(mem);
    free(exe);
    return 1;
}

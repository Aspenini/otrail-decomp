/*
 * Replay inferred unpacker while emitting per-event trace CSV.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../logic/entry_unpacker_internal.h"

static void stats_init(OtrailUnpackStats *stats) {
    if (stats == NULL) {
        return;
    }
    stats->literal_ops = 0;
    stats->short_copy_ops = 0;
    stats->long_copy_ops = 0;
    stats->copied_bytes = 0;
    stats->fail_code = 0;
    stats->fail_src_pos = 0;
    stats->fail_out_pos = 0;
}

static void maybe_preseed_strict_window(uint8_t *touched, int mode) {
    size_t i;
    if (mode != OTRAIL_UNPACK_STRICT) {
        return;
    }
    for (i = 0; i < 0x2000; i++) {
        touched[(uint16_t)(0xE000u + (uint16_t)i)] = 1u;
    }
}

static int write_header(FILE *csv) {
    return fprintf(
               csv,
               "event_idx,type,src_abs_before,src_abs_after,src_delta,"
               "out_before,out_after,out_delta,hpos_before,hpos_after,"
               "bit0,bit1,token_u16,ext,literal,back_disp,copy_len,status\n"
           ) > 0;
}

int main(int argc, char **argv) {
    const char *path = "Oregon_The_1990/OREGON.EXE";
    uint32_t offset = 0x1274F;
    size_t dst_cap = 65536;
    int mode = OTRAIL_UNPACK_HEURISTIC;
    size_t max_events = 100;
    const char *trace_path = "build/unpacker_trace.csv";

    uint8_t *file_buf = NULL;
    uint8_t *dst = NULL;
    uint8_t *history = NULL;
    uint8_t *touched = NULL;
    FILE *f = NULL;
    FILE *csv = NULL;
    long file_size_long;
    size_t file_size;

    BitReader16 br;
    UnpackState st;
    OtrailUnpackStats stats;
    size_t event_idx = 0;
    int ok = 0;

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
    if (argc >= 6) {
        max_events = (size_t)strtoull(argv[5], NULL, 0);
        if (max_events == 0) {
            max_events = 100;
        }
    }
    if (argc >= 7) {
        trace_path = argv[6];
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
    history = (uint8_t *)malloc(65536);
    touched = (uint8_t *)malloc(65536);
    if (file_buf == NULL || dst == NULL || history == NULL || touched == NULL) {
        fclose(f);
        free(file_buf);
        free(dst);
        free(history);
        free(touched);
        return 1;
    }
    if (fread(file_buf, 1, file_size, f) != file_size) {
        fclose(f);
        free(file_buf);
        free(dst);
        free(history);
        free(touched);
        return 1;
    }
    fclose(f);
    f = NULL;

    csv = fopen(trace_path, "w");
    if (!csv) {
        free(file_buf);
        free(dst);
        free(history);
        free(touched);
        return 1;
    }
    if (!write_header(csv)) {
        fclose(csv);
        free(file_buf);
        free(dst);
        free(history);
        free(touched);
        return 1;
    }

    br.src = file_buf + offset;
    br.src_len = file_size - offset;
    br.pos = 0;
    br.bitbuf = 0;
    br.bits_left = 0;

    st.dst = dst;
    st.dst_cap = dst_cap;
    st.out_pos = 0;
    st.history = history;
    st.touched = touched;
    st.hpos = 0;
    st.mode = mode;
    st.stats = &stats;

    memset(history, 0, 65536);
    memset(touched, 0, 65536);
    maybe_preseed_strict_window(touched, mode);
    stats_init(&stats);

    while (event_idx < max_events) {
        size_t src_before = br.pos;
        size_t out_before = st.out_pos;
        uint16_t hpos_before = st.hpos;
        uint8_t bit0 = 0;
        uint8_t bit1 = 2;
        uint16_t token = 0;
        int ext = -1;
        int literal = -1;
        int back_disp = 0;
        size_t copy_len = 0;
        const char *etype = "unknown";
        const char *status = "ok";

        if (!br_get_bit(&br, &bit0)) {
            stats.fail_code = 1;
            stats.fail_src_pos = br.pos;
            stats.fail_out_pos = st.out_pos;
            ok = 0;
            break;
        }

        if (bit0) {
            uint8_t lit = 0;
            etype = "literal";
            if (!br_get_u8(&br, &lit)) {
                stats.fail_code = 2;
                stats.fail_src_pos = br.pos;
                stats.fail_out_pos = st.out_pos;
                ok = 0;
                break;
            }
            literal = (int)lit;
            if (!emit_literal(&st, lit)) {
                stats.fail_code = 2;
                stats.fail_src_pos = br.pos;
                stats.fail_out_pos = st.out_pos;
                ok = 0;
                break;
            }
        } else {
            if (!br_get_bit(&br, &bit1)) {
                stats.fail_code = 3;
                stats.fail_src_pos = br.pos;
                stats.fail_out_pos = st.out_pos;
                ok = 0;
                break;
            }

            if (bit1) {
                uint8_t low;
                uint8_t high;
                uint8_t ext_u8 = 0;
                size_t req_len;
                ptrdiff_t req_disp;

                etype = "long";
                if (!br_get_u16le(&br, &token)) {
                    stats.fail_code = 4;
                    stats.fail_src_pos = br.pos;
                    stats.fail_out_pos = st.out_pos;
                    ok = 0;
                    break;
                }

                low = (uint8_t)(token & 0xFFu);
                high = (uint8_t)((token >> 8) & 0xFFu);
                req_disp = (ptrdiff_t)(int16_t)(
                    (uint16_t)(((uint16_t)(0xE0u | (high >> 3)) << 8) | low)
                );
                req_len = (size_t)(high & 0x7u);
                back_disp = (int)req_disp;

                if (req_len == 0) {
                    if (!br_get_u8(&br, &ext_u8)) {
                        stats.fail_code = 5;
                        stats.fail_src_pos = br.pos;
                        stats.fail_out_pos = st.out_pos;
                        ok = 0;
                        break;
                    }
                    ext = (int)ext_u8;
                    if (ext_u8 == 0) {
                        etype = "long_end";
                        status = "stream_end";
                    } else if (ext_u8 == 1) {
                        etype = "long_nop";
                    } else {
                        req_len = (size_t)ext_u8 + 1u;
                        copy_len = req_len;
                    }
                } else {
                    req_len += 2u;
                    copy_len = req_len;
                }

                if (strcmp(etype, "long_end") == 0) {
                    /* No copy performed; stream ends after this event. */
                } else if (strcmp(etype, "long_nop") == 0) {
                    /* No-op control token. */
                } else {
                    if (!copy_from_history(&st, req_disp, req_len, 7)) {
                        if (stats.fail_code == 0) {
                            stats.fail_code = 6;
                        }
                        stats.fail_src_pos = br.pos;
                        stats.fail_out_pos = st.out_pos;
                        ok = 0;
                        break;
                    }
                    stats.long_copy_ops++;
                    stats.copied_bytes += req_len;
                }
            } else {
                uint8_t b = 0;
                uint8_t bit_a = 0;
                uint8_t bit_b = 0;
                ptrdiff_t req_disp;
                size_t req_len;

                etype = "short";
                if (!br_get_u8(&br, &b)) {
                    stats.fail_code = 8;
                    stats.fail_src_pos = br.pos;
                    stats.fail_out_pos = st.out_pos;
                    ok = 0;
                    break;
                }
                if (!br_get_bit(&br, &bit_a) || !br_get_bit(&br, &bit_b)) {
                    stats.fail_code = 9;
                    stats.fail_src_pos = br.pos;
                    stats.fail_out_pos = st.out_pos;
                    ok = 0;
                    break;
                }

                req_len = (size_t)(((bit_a << 1) | bit_b) + 2u);
                req_disp = (ptrdiff_t)(int16_t)(uint16_t)(0xFF00u | b);
                back_disp = (int)req_disp;
                copy_len = req_len;
                if (!copy_from_history(&st, req_disp, req_len, 11)) {
                    if (stats.fail_code == 0) {
                        stats.fail_code = 10;
                    }
                    stats.fail_src_pos = br.pos;
                    stats.fail_out_pos = st.out_pos;
                    ok = 0;
                    break;
                }
                stats.short_copy_ops++;
                stats.copied_bytes += req_len;
            }
        }

        if (
            fprintf(
                csv,
                "%zu,%s,0x%X,0x%X,%zu,%zu,%zu,%zu,0x%04X,0x%04X,%u,%u,0x%04X,%d,%d,%d,%zu,%s\n",
                event_idx,
                etype,
                (unsigned int)(offset + (uint32_t)src_before),
                (unsigned int)(offset + (uint32_t)br.pos),
                br.pos - src_before,
                out_before,
                st.out_pos,
                st.out_pos - out_before,
                hpos_before,
                st.hpos,
                (unsigned int)bit0,
                (unsigned int)bit1,
                token,
                ext,
                literal,
                back_disp,
                copy_len,
                status
            ) < 0
        ) {
            fclose(csv);
            free(file_buf);
            free(dst);
            free(history);
            free(touched);
            return 1;
        }

        event_idx++;
        if (strcmp(etype, "long_end") == 0) {
            ok = 1;
            break;
        }
    }

    if (event_idx == max_events && ok == 0 && stats.fail_code == 0) {
        ok = 1;
    }

    printf("status=%s\n", ok ? "ok" : "partial");
    printf("mode=%s\n", mode == OTRAIL_UNPACK_STRICT ? "strict" : "heuristic");
    printf("offset=0x%X\n", offset);
    printf("events_written=%zu\n", event_idx);
    printf("src_used=%zu\n", br.pos);
    printf("dst_written=%zu\n", st.out_pos);
    printf("fail_code=%d\n", stats.fail_code);
    printf("fail_src_pos=%zu\n", stats.fail_src_pos);
    printf("fail_out_pos=%zu\n", stats.fail_out_pos);
    printf("trace_file=%s\n", trace_path);

    fclose(csv);
    free(file_buf);
    free(dst);
    free(history);
    free(touched);
    return 0;
}

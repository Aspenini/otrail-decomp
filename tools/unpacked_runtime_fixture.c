/*
 * Fixture runner for the first readable post-unpack runtime fragments.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../logic/unpacked_runtime_fragments.h"
#include "../logic/unpacked_runtime_map.h"

static unsigned long parse_u32(const char *text) {
    return strtoul(text, 0, 0);
}

static void print_hex(const uint8_t *bytes, size_t size) {
    size_t i;

    for (i = 0; i < size; ++i) {
        printf("%02X", bytes[i]);
    }
    printf("\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage:\n");
        fprintf(stderr, "  %s seed05ff <src0> <src1>\n", argv[0]);
        fprintf(stderr, "  %s seed0dc3 <src>\n", argv[0]);
        fprintf(stderr, "  %s block05f0 <m0> <m1> <m2> <s0> <s1> <s2> <s3> <t0> <t1> <t2> <t3>\n", argv[0]);
        fprintf(stderr, "  %s block0dc0 <s0> <s1> <s2> <p0> <p1>\n", argv[0]);
        fprintf(stderr, "  %s window05a4 <m0> <m1> <m2> <s0> <s1> <s2> <s3> <t0> <t1> <t2> <t3>\n", argv[0]);
        fprintf(stderr, "  %s window06b0 <m0> <m1> <m2> <s0> <s1> <s2> <s3> <t0> <t1> <t2> <t3>\n", argv[0]);
        fprintf(stderr, "  %s window05e5 <m0> <m1> <m2> <s0> <s1> <s2> <s3> <t0> <t1> <t2> <t3>\n", argv[0]);
        fprintf(stderr, "  %s window0fd0 <m0> <m1> <m2> <s0> <s1> <s2> <s3> <t0> <t1> <t2> <t3>\n", argv[0]);
        fprintf(stderr, "  %s window0eff\n", argv[0]);
        fprintf(stderr, "  %s window0f20\n", argv[0]);
        fprintf(stderr, "  %s window0f46\n", argv[0]);
        fprintf(stderr, "  %s window05f5 <m0> <m1> <m2> <s0> <s1> <s2> <s3> <t0> <t1> <t2> <t3>\n", argv[0]);
        fprintf(stderr, "  %s window1b7a <m0> <m1> <m2> <s0> <s1> <s2> <s3> <t0> <t1> <t2> <t3>\n", argv[0]);
        fprintf(stderr, "  %s window0d1a <s0> <s1> <s2> <p0> <p1>\n", argv[0]);
        fprintf(stderr, "  %s window1c5f <s0> <s1> <s2> <p0> <p1>\n", argv[0]);
        fprintf(stderr, "  %s window1f6d <p0> <p1> <s0> <s1> <s2> <s3> <t0> <t1> <t2>\n", argv[0]);
        fprintf(stderr, "  %s window21b6 <p0> <p1> <s0> <s1> <s2> <s3> <t0> <t1> <t2>\n", argv[0]);
        fprintf(stderr, "  %s window2000 <p0> <p1> <s0> <s1> <s2> <s3> <t0> <t1> <t2>\n", argv[0]);
        fprintf(stderr, "  %s window2000full <p0> <p1> <s0> <s1> <s2> <s3> <t0> <t1> <t2>\n", argv[0]);
        fprintf(stderr, "  %s window2003 <p0> <p1> <s0> <s1> <s2> <s3> <t0> <t1> <t2>\n", argv[0]);
        fprintf(stderr, "  %s window207e <p0> <p1> <s0> <s1> <s2> <s3> <t0> <t1> <t2>\n", argv[0]);
        fprintf(stderr, "  %s window110e\n", argv[0]);
        fprintf(stderr, "  %s window2e90\n", argv[0]);
        fprintf(stderr, "  %s window2f00\n", argv[0]);
        fprintf(stderr, "  %s window2ef0\n", argv[0]);
        fprintf(stderr, "  %s window2ec0\n", argv[0]);
        fprintf(stderr, "  %s window2f40\n", argv[0]);
        fprintf(stderr, "  %s window2f00to2f80\n", argv[0]);
        fprintf(stderr, "  %s window2a3a\n", argv[0]);
        fprintf(stderr, "  %s window2a6e\n", argv[0]);
        fprintf(stderr, "  %s window2a8f\n", argv[0]);
        fprintf(stderr, "  %s window2ab5\n", argv[0]);
        fprintf(stderr, "  %s window2c7f\n", argv[0]);
        fprintf(stderr, "  %s window2cfa\n", argv[0]);
        fprintf(stderr, "  %s window2f80\n", argv[0]);
        fprintf(stderr, "  %s window2fc0\n", argv[0]);
        fprintf(stderr, "  %s window2552\n", argv[0]);
        fprintf(stderr, "  %s window2f00to3000\n", argv[0]);
        fprintf(stderr, "  %s window3000\n", argv[0]);
        fprintf(stderr, "  %s window3040\n", argv[0]);
        fprintf(stderr, "  %s window3080\n", argv[0]);
        fprintf(stderr, "  %s window1909\n", argv[0]);
        fprintf(stderr, "  %s window2612\n", argv[0]);
        fprintf(stderr, "  %s window2f00to30aa\n", argv[0]);
        fprintf(stderr, "  %s window0850\n", argv[0]);
        fprintf(stderr, "  %s window0950\n", argv[0]);
        fprintf(stderr, "  %s window11c0\n", argv[0]);
        fprintf(stderr, "  %s window2748\n", argv[0]);
        fprintf(stderr, "  %s window09e0\n", argv[0]);
        fprintf(stderr, "  %s window177c\n", argv[0]);
        fprintf(stderr, "  %s window1d7f\n", argv[0]);
        fprintf(stderr, "  %s window2500\n", argv[0]);
        fprintf(stderr, "  %s window2670\n", argv[0]);
        fprintf(stderr, "  %s window2df0\n", argv[0]);
        fprintf(stderr, "  %s window29a0\n", argv[0]);
        fprintf(stderr, "  %s window2b20\n", argv[0]);
        fprintf(stderr, "  %s window2c00\n", argv[0]);
        fprintf(stderr, "  %s window2d50\n", argv[0]);
        fprintf(stderr, "  %s window1800\n", argv[0]);
        fprintf(stderr, "  %s candidate1803prefix\n", argv[0]);
        fprintf(stderr, "  %s window2310\n", argv[0]);
        fprintf(stderr, "  %s window3130\n", argv[0]);
        fprintf(stderr, "  %s span0000to0200\n", argv[0]);
        fprintf(stderr, "  %s span0200to0400\n", argv[0]);
        fprintf(stderr, "  %s span0400to05a4\n", argv[0]);
        fprintf(stderr, "  %s span0900to0b00\n", argv[0]);
        fprintf(stderr, "  %s span1100to1200\n", argv[0]);
        fprintf(stderr, "  %s span1700to1900\n", argv[0]);
        fprintf(stderr, "  %s span1d50to1f00\n", argv[0]);
        fprintf(stderr, "  %s span2300to2500\n", argv[0]);
        fprintf(stderr, "  %s span2800to2a00\n", argv[0]);
        fprintf(stderr, "  %s span2b70to2c00\n", argv[0]);
        fprintf(stderr, "  %s span2c40to2c7f\n", argv[0]);
        fprintf(stderr, "  %s span2d90to2df0\n", argv[0]);
        fprintf(stderr, "  %s span2e40to2e90\n", argv[0]);
        fprintf(stderr, "  %s span30aato3172\n", argv[0]);
        fprintf(stderr, "  %s span07adto0850\n", argv[0]);
        fprintf(stderr, "  %s span08d0to0900\n", argv[0]);
        fprintf(stderr, "  %s span0b00to0d00\n", argv[0]);
        fprintf(stderr, "  %s span0d00to0d1a\n", argv[0]);
        fprintf(stderr, "  %s span0e05to0eff\n", argv[0]);
        fprintf(stderr, "  %s span0fabto0fd0\n", argv[0]);
        fprintf(stderr, "  %s span1098to1100\n", argv[0]);
        fprintf(stderr, "  %s span1240to1440\n", argv[0]);
        fprintf(stderr, "  %s span1440to1640\n", argv[0]);
        fprintf(stderr, "  %s span1640to1700\n", argv[0]);
        fprintf(stderr, "  %s span1900to1909\n", argv[0]);
        fprintf(stderr, "  %s span1933to1b33\n", argv[0]);
        fprintf(stderr, "  %s span1b33to1b7a\n", argv[0]);
        fprintf(stderr, "  %s span1c5bto1c5f\n", argv[0]);
        fprintf(stderr, "  %s span1d4ato1d50\n", argv[0]);
        fprintf(stderr, "  %s span1f00to1f6d\n", argv[0]);
        fprintf(stderr, "  %s span206cto207e\n", argv[0]);
        fprintf(stderr, "  %s span209cto21b6\n", argv[0]);
        fprintf(stderr, "  %s span22b5to2300\n", argv[0]);
        fprintf(stderr, "  %s span2550to2552\n", argv[0]);
        fprintf(stderr, "  %s span2592to2612\n", argv[0]);
        fprintf(stderr, "  %s span263cto2670\n", argv[0]);
        fprintf(stderr, "  %s span26b0to2748\n", argv[0]);
        fprintf(stderr, "  %s span2798to2800\n", argv[0]);
        fprintf(stderr, "  %s span2a00to2a3a\n", argv[0]);
        fprintf(stderr, "  %s span2b1ato2b20\n", argv[0]);
        fprintf(stderr, "  %s span2d4ato2d50\n", argv[0]);
        fprintf(stderr, "  %s span2eb0to2ec0\n", argv[0]);
        fprintf(stderr, "  %s map0f46\n", argv[0]);
        return 1;
    }


    if (strncmp(argv[1], "span", 4) == 0) {
        uint8_t out[OTRAIL_UNPACKED_RUNTIME_SPAN_MAX_SIZE];
        size_t out_size = 0;

        if (!otrail_unpacked_build_span(argv[1], out, sizeof(out), &out_size)) {
            return 1;
        }
        printf("status=ok\n");
        printf("kind=%s\n", argv[1]);
        printf("bytes_hex=");
        print_hex(out, out_size);
        return 0;
    }

    if (strcmp(argv[1], "map0f46") == 0) {
        size_t i;

        printf("status=ok\n");
        printf("kind=map0f46\n");
        printf("region_count=%zu\n", otrail_runtime_0f46_region_count);
        for (i = 0; i < otrail_runtime_0f46_region_count; i++) {
            const OtrailRuntimeRegionDescriptor *region = &otrail_runtime_0f46_regions[i];

            printf(
                "region_%zu=%s|%s|%s|0x%04X|0x%04X|%s|%zu|%s\n",
                i,
                region->id,
                region->family,
                region->role,
                (unsigned)region->start,
                (unsigned)region->end,
                otrail_runtime_region_kind_name(region->kind),
                otrail_runtime_region_size(region),
                region->purpose
            );
        }
        return 0;
    }

    if (strcmp(argv[1], "candidate1803prefix") == 0) {
        OtrailCandidate1803RoutinePrefix desc;
        uint8_t out[OTRAIL_CANDIDATE_1803_PREFIX_SIZE];

        if (!otrail_unpacked_build_candidate_1803_prefix(out)) {
            return 1;
        }
        if (!otrail_unpacked_describe_candidate_1803_prefix(&desc)) {
            return 1;
        }
        printf("status=ok\n");
        printf("kind=candidate1803prefix\n");
        printf("bytes_hex=");
        print_hex(out, sizeof(out));
        printf("loop_count=0x%04X\n", (unsigned)desc.loop_count);
        printf("far_call=0x%04X:0x%04X\n", (unsigned)desc.far_call_segment, (unsigned)desc.far_call_offset);
        printf("save_ax_bp_disp=%d\n", (int)desc.save_ax_bp_disp);
        printf("save_bx_bp_disp=%d\n", (int)desc.save_bx_bp_disp);
        printf("save_dx_bp_disp=%d\n", (int)desc.save_dx_bp_disp);
        return 0;
    }

    if (strcmp(argv[1], "seed05ff") == 0) {
        OtrailSeed05FFInputs in;
        uint8_t out[OTRAIL_SEED_05FF_SIZE];

        if (argc < 4) {
            return 1;
        }
        in.source_0506_pair[0] = (uint8_t)parse_u32(argv[2]);
        in.source_0506_pair[1] = (uint8_t)parse_u32(argv[3]);
        if (!otrail_unpacked_build_seed_05ff(&in, out)) {
            return 1;
        }
        printf("status=ok\n");
        printf("kind=seed05ff\n");
        printf("bytes_hex=");
        print_hex(out, sizeof(out));
        return 0;
    }

    if (strcmp(argv[1], "seed0dc3") == 0) {
        OtrailSeed0DC3Inputs in;
        uint8_t out[OTRAIL_SEED_0DC3_SIZE];

        if (argc < 3) {
            return 1;
        }
        in.source_0DA8_byte = (uint8_t)parse_u32(argv[2]);
        if (!otrail_unpacked_build_seed_0dc3(&in, out)) {
            return 1;
        }
        printf("status=ok\n");
        printf("kind=seed0dc3\n");
        printf("bytes_hex=");
        print_hex(out, sizeof(out));
        return 0;
    }

    if (strcmp(argv[1], "block05f0") == 0) {
        OtrailBlock05F0Inputs in;
        uint8_t out[OTRAIL_BLOCK_05F0_SIZE];

        if (argc < 13) {
            return 1;
        }
        in.source_05f6_triplet[0] = (uint8_t)parse_u32(argv[2]);
        in.source_05f6_triplet[1] = (uint8_t)parse_u32(argv[3]);
        in.source_05f6_triplet[2] = (uint8_t)parse_u32(argv[4]);
        in.seed_05ff[0] = (uint8_t)parse_u32(argv[5]);
        in.seed_05ff[1] = (uint8_t)parse_u32(argv[6]);
        in.seed_05ff[2] = (uint8_t)parse_u32(argv[7]);
        in.seed_05ff[3] = (uint8_t)parse_u32(argv[8]);
        in.source_060b_quad[0] = (uint8_t)parse_u32(argv[9]);
        in.source_060b_quad[1] = (uint8_t)parse_u32(argv[10]);
        in.source_060b_quad[2] = (uint8_t)parse_u32(argv[11]);
        in.source_060b_quad[3] = (uint8_t)parse_u32(argv[12]);
        if (!otrail_unpacked_build_block_05f0(&in, out)) {
            return 1;
        }
        printf("status=ok\n");
        printf("kind=block05f0\n");
        printf("bytes_hex=");
        print_hex(out, sizeof(out));
        return 0;
    }

    if (strcmp(argv[1], "block0dc0") == 0) {
        OtrailBlock0DC0Inputs in;
        uint8_t out[OTRAIL_BLOCK_0DC0_SIZE];

        if (argc < 7) {
            return 1;
        }
        in.seed_0dc3[0] = (uint8_t)parse_u32(argv[2]);
        in.seed_0dc3[1] = (uint8_t)parse_u32(argv[3]);
        in.seed_0dc3[2] = (uint8_t)parse_u32(argv[4]);
        in.source_0dce_pair[0] = (uint8_t)parse_u32(argv[5]);
        in.source_0dce_pair[1] = (uint8_t)parse_u32(argv[6]);
        if (!otrail_unpacked_build_block_0dc0(&in, out)) {
            return 1;
        }
        printf("status=ok\n");
        printf("kind=block0dc0\n");
        printf("bytes_hex=");
        print_hex(out, sizeof(out));
        return 0;
    }

    if (
        strcmp(argv[1], "window05a4") == 0 ||
        strcmp(argv[1], "window06b0") == 0 ||
        strcmp(argv[1], "window05e5") == 0 ||
        strcmp(argv[1], "window0fd0") == 0 ||
        strcmp(argv[1], "window05f5") == 0 ||
        strcmp(argv[1], "window1b7a") == 0
    ) {
        OtrailBlock05F0Inputs in;
        uint8_t out[OTRAIL_WINDOW_05A4_SIZE];
        size_t out_size = OTRAIL_WINDOW_05A4_SIZE;

        if (argc < 13) {
            return 1;
        }
        in.source_05f6_triplet[0] = (uint8_t)parse_u32(argv[2]);
        in.source_05f6_triplet[1] = (uint8_t)parse_u32(argv[3]);
        in.source_05f6_triplet[2] = (uint8_t)parse_u32(argv[4]);
        in.seed_05ff[0] = (uint8_t)parse_u32(argv[5]);
        in.seed_05ff[1] = (uint8_t)parse_u32(argv[6]);
        in.seed_05ff[2] = (uint8_t)parse_u32(argv[7]);
        in.seed_05ff[3] = (uint8_t)parse_u32(argv[8]);
        in.source_060b_quad[0] = (uint8_t)parse_u32(argv[9]);
        in.source_060b_quad[1] = (uint8_t)parse_u32(argv[10]);
        in.source_060b_quad[2] = (uint8_t)parse_u32(argv[11]);
        in.source_060b_quad[3] = (uint8_t)parse_u32(argv[12]);
        if (strcmp(argv[1], "window05a4") == 0) {
            if (!otrail_unpacked_build_window_05a4(&in, out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_05A4_SIZE;
        } else if (strcmp(argv[1], "window06b0") == 0) {
            if (!otrail_unpacked_build_window_06b0(&in, out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_06B0_SIZE;
        } else if (strcmp(argv[1], "window05e5") == 0) {
            if (!otrail_unpacked_build_window_05e5(&in, out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_05E5_SIZE;
        } else if (strcmp(argv[1], "window0fd0") == 0) {
            if (!otrail_unpacked_build_window_0fd0(&in, out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_0FD0_SIZE;
        } else if (strcmp(argv[1], "window05f5") == 0) {
            if (!otrail_unpacked_build_window_05f5(&in, out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_05F5_SIZE;
        } else {
            if (!otrail_unpacked_build_window_1b7a(&in, out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_1B7A_SIZE;
        }
        printf("status=ok\n");
        printf("kind=%s\n", argv[1]);
        printf("bytes_hex=");
        print_hex(out, out_size);
        return 0;
    }

    if (strcmp(argv[1], "window0d1a") == 0 || strcmp(argv[1], "window1c5f") == 0) {
        OtrailBlock0DC0Inputs in;
        uint8_t out[OTRAIL_WINDOW_0D1A_SIZE];

        if (argc < 7) {
            return 1;
        }
        in.seed_0dc3[0] = (uint8_t)parse_u32(argv[2]);
        in.seed_0dc3[1] = (uint8_t)parse_u32(argv[3]);
        in.seed_0dc3[2] = (uint8_t)parse_u32(argv[4]);
        in.source_0dce_pair[0] = (uint8_t)parse_u32(argv[5]);
        in.source_0dce_pair[1] = (uint8_t)parse_u32(argv[6]);
        if (strcmp(argv[1], "window0d1a") == 0) {
            if (!otrail_unpacked_build_window_0d1a(&in, out)) {
                return 1;
            }
        } else {
            if (!otrail_unpacked_build_window_1c5f(&in, out)) {
                return 1;
            }
        }
        printf("status=ok\n");
        printf("kind=%s\n", argv[1]);
        printf("bytes_hex=");
        print_hex(out, sizeof(out));
        return 0;
    }

    if (strcmp(argv[1], "window2000") == 0) {
        OtrailWindow2000PrefixInputs in;
        uint8_t out[OTRAIL_WINDOW_2000_PREFIX_SIZE];

        if (argc < 11) {
            return 1;
        }
        in.source_1230_tail[0] = (uint8_t)parse_u32(argv[2]);
        in.source_1230_tail[1] = (uint8_t)parse_u32(argv[3]);
        in.seed_05ff[0] = (uint8_t)parse_u32(argv[4]);
        in.seed_05ff[1] = (uint8_t)parse_u32(argv[5]);
        in.seed_05ff[2] = (uint8_t)parse_u32(argv[6]);
        in.seed_05ff[3] = (uint8_t)parse_u32(argv[7]);
        in.seed_0dc3[0] = (uint8_t)parse_u32(argv[8]);
        in.seed_0dc3[1] = (uint8_t)parse_u32(argv[9]);
        in.seed_0dc3[2] = (uint8_t)parse_u32(argv[10]);
        if (!otrail_unpacked_build_window_2000_prefix(&in, out)) {
            return 1;
        }
        printf("status=ok\n");
        printf("kind=window2000\n");
        printf("bytes_hex=");
        print_hex(out, sizeof(out));
        return 0;
    }

    if (
        strcmp(argv[1], "window1f6d") == 0 ||
        strcmp(argv[1], "window21b6") == 0 ||
        strcmp(argv[1], "window2000full") == 0 ||
        strcmp(argv[1], "window2003") == 0 ||
        strcmp(argv[1], "window207e") == 0
    ) {
        OtrailWindow2000PrefixInputs in;
        uint8_t out[OTRAIL_WINDOW_21B6_SIZE];
        size_t out_size = OTRAIL_WINDOW_2000_SIZE;

        if (argc < 11) {
            return 1;
        }
        in.source_1230_tail[0] = (uint8_t)parse_u32(argv[2]);
        in.source_1230_tail[1] = (uint8_t)parse_u32(argv[3]);
        in.seed_05ff[0] = (uint8_t)parse_u32(argv[4]);
        in.seed_05ff[1] = (uint8_t)parse_u32(argv[5]);
        in.seed_05ff[2] = (uint8_t)parse_u32(argv[6]);
        in.seed_05ff[3] = (uint8_t)parse_u32(argv[7]);
        in.seed_0dc3[0] = (uint8_t)parse_u32(argv[8]);
        in.seed_0dc3[1] = (uint8_t)parse_u32(argv[9]);
        in.seed_0dc3[2] = (uint8_t)parse_u32(argv[10]);
        if (strcmp(argv[1], "window1f6d") == 0) {
            if (!otrail_unpacked_build_window_1f6d(&in, out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_1F6D_SIZE;
        } else if (strcmp(argv[1], "window21b6") == 0) {
            if (!otrail_unpacked_build_window_21b6(&in, out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_21B6_SIZE;
        } else if (strcmp(argv[1], "window2000full") == 0) {
            if (!otrail_unpacked_build_window_2000(&in, out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2000_SIZE;
        } else if (strcmp(argv[1], "window2003") == 0) {
            if (!otrail_unpacked_build_window_2003(&in, out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2003_SIZE;
        } else {
            if (!otrail_unpacked_build_window_207e(&in, out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_207E_SIZE;
        }
        printf("status=ok\n");
        printf("kind=%s\n", argv[1]);
        printf("bytes_hex=");
        print_hex(out, out_size);
        return 0;
    }

    if (
        strcmp(argv[1], "window110e") == 0 ||
        strcmp(argv[1], "window2e90") == 0 ||
        strcmp(argv[1], "window2f00") == 0 ||
        strcmp(argv[1], "window2ef0") == 0 ||
        strcmp(argv[1], "window2ec0") == 0 ||
        strcmp(argv[1], "window2f40") == 0 ||
        strcmp(argv[1], "window2f00to2f80") == 0 ||
        strcmp(argv[1], "window0eff") == 0 ||
        strcmp(argv[1], "window0f20") == 0 ||
        strcmp(argv[1], "window0f46") == 0 ||
        strcmp(argv[1], "window2a3a") == 0 ||
        strcmp(argv[1], "window2a6e") == 0 ||
        strcmp(argv[1], "window2a8f") == 0 ||
        strcmp(argv[1], "window2ab5") == 0 ||
        strcmp(argv[1], "window2c7f") == 0 ||
        strcmp(argv[1], "window2cfa") == 0 ||
        strcmp(argv[1], "window2f80") == 0 ||
        strcmp(argv[1], "window2fc0") == 0 ||
        strcmp(argv[1], "window2552") == 0 ||
        strcmp(argv[1], "window2f00to3000") == 0 ||
        strcmp(argv[1], "window3000") == 0 ||
        strcmp(argv[1], "window3040") == 0 ||
        strcmp(argv[1], "window3080") == 0 ||
        strcmp(argv[1], "window1909") == 0 ||
        strcmp(argv[1], "window2612") == 0 ||
        strcmp(argv[1], "window2f00to30aa") == 0 ||
        strcmp(argv[1], "window0850") == 0 ||
        strcmp(argv[1], "window0950") == 0 ||
        strcmp(argv[1], "window11c0") == 0 ||
        strcmp(argv[1], "window2748") == 0 ||
        strcmp(argv[1], "window09e0") == 0 ||
        strcmp(argv[1], "window177c") == 0 ||
        strcmp(argv[1], "window1d7f") == 0 ||
        strcmp(argv[1], "window2500") == 0 ||
        strcmp(argv[1], "window2670") == 0 ||
        strcmp(argv[1], "window2df0") == 0 ||
        strcmp(argv[1], "window29a0") == 0 ||
        strcmp(argv[1], "window2b20") == 0 ||
        strcmp(argv[1], "window2c00") == 0 ||
        strcmp(argv[1], "window2d50") == 0 ||
        strcmp(argv[1], "window1800") == 0 ||
        strcmp(argv[1], "window2310") == 0 ||
        strcmp(argv[1], "window3130") == 0
    ) {
        uint8_t out[OTRAIL_WINDOW_2F00_TO_30AA_SIZE];
        size_t out_size = OTRAIL_WINDOW_110E_SIZE;

        if (strcmp(argv[1], "window0eff") == 0) {
            if (!otrail_unpacked_build_window_0eff(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_0EFF_SIZE;
        } else if (strcmp(argv[1], "window0f20") == 0) {
            if (!otrail_unpacked_build_window_0f20(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_0F20_SIZE;
        } else if (strcmp(argv[1], "window0f46") == 0) {
            if (!otrail_unpacked_build_window_0f46(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_0F46_SIZE;
        } else if (strcmp(argv[1], "window110e") == 0) {
            if (!otrail_unpacked_build_window_110e(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_110E_SIZE;
        } else if (strcmp(argv[1], "window2e90") == 0) {
            if (!otrail_unpacked_build_window_2e90(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2E90_SIZE;
        } else if (strcmp(argv[1], "window2f00") == 0) {
            if (!otrail_unpacked_build_window_2f00(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2F00_SIZE;
        } else if (strcmp(argv[1], "window2ef0") == 0) {
            if (!otrail_unpacked_build_window_2ef0(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2EF0_SIZE;
        } else if (strcmp(argv[1], "window2ec0") == 0) {
            if (!otrail_unpacked_build_window_2ec0(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2EC0_SIZE;
        } else if (strcmp(argv[1], "window2f40") == 0) {
            if (!otrail_unpacked_build_window_2f40(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2F40_SIZE;
        } else if (strcmp(argv[1], "window2f00to2f80") == 0) {
            if (!otrail_unpacked_build_window_2f00_to_2f80(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2F00_TO_2F80_SIZE;
        } else if (strcmp(argv[1], "window2a3a") == 0) {
            if (!otrail_unpacked_build_window_2a3a(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2A3A_SIZE;
        } else if (strcmp(argv[1], "window2a6e") == 0) {
            if (!otrail_unpacked_build_window_2a6e(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2A6E_SIZE;
        } else if (strcmp(argv[1], "window2a8f") == 0) {
            if (!otrail_unpacked_build_window_2a8f(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2A8F_SIZE;
        } else if (strcmp(argv[1], "window2ab5") == 0) {
            if (!otrail_unpacked_build_window_2ab5(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2AB5_SIZE;
        } else if (strcmp(argv[1], "window2c7f") == 0) {
            if (!otrail_unpacked_build_window_2c7f(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2C7F_SIZE;
        } else if (strcmp(argv[1], "window2cfa") == 0) {
            if (!otrail_unpacked_build_window_2cfa(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2CFA_SIZE;
        } else if (strcmp(argv[1], "window2f80") == 0) {
            if (!otrail_unpacked_build_window_2f80(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2F80_SIZE;
        } else if (strcmp(argv[1], "window2fc0") == 0) {
            if (!otrail_unpacked_build_window_2fc0(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2FC0_SIZE;
        } else if (strcmp(argv[1], "window2552") == 0) {
            if (!otrail_unpacked_build_window_2552(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2552_SIZE;
        } else if (strcmp(argv[1], "window2f00to3000") == 0) {
            if (!otrail_unpacked_build_window_2f00_to_3000(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2F00_TO_3000_SIZE;
        } else if (strcmp(argv[1], "window3000") == 0) {
            if (!otrail_unpacked_build_window_3000(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_3000_SIZE;
        } else if (strcmp(argv[1], "window3040") == 0) {
            if (!otrail_unpacked_build_window_3040(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_3040_SIZE;
        } else if (strcmp(argv[1], "window3080") == 0) {
            if (!otrail_unpacked_build_window_3080(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_3080_SIZE;
        } else if (strcmp(argv[1], "window1909") == 0) {
            if (!otrail_unpacked_build_window_1909(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_1909_SIZE;
        } else if (strcmp(argv[1], "window2612") == 0) {
            if (!otrail_unpacked_build_window_2612(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2612_SIZE;
        } else if (strcmp(argv[1], "window0850") == 0) {
            if (!otrail_unpacked_build_window_0850(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_0850_SIZE;
        } else if (strcmp(argv[1], "window0950") == 0) {
            if (!otrail_unpacked_build_window_0950(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_0950_SIZE;
        } else if (strcmp(argv[1], "window2748") == 0) {
            if (!otrail_unpacked_build_window_2748(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2748_SIZE;
        } else if (strcmp(argv[1], "window11c0") == 0) {
            if (!otrail_unpacked_build_window_11c0(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_11C0_SIZE;
        } else if (strcmp(argv[1], "window09e0") == 0) {
            if (!otrail_unpacked_build_window_09e0(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_09E0_SIZE;
        } else if (strcmp(argv[1], "window177c") == 0) {
            if (!otrail_unpacked_build_window_177c(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_177C_SIZE;
        } else if (strcmp(argv[1], "window1d7f") == 0) {
            if (!otrail_unpacked_build_window_1d7f(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_1D7F_SIZE;
        } else if (strcmp(argv[1], "window2500") == 0) {
            if (!otrail_unpacked_build_window_2500(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2500_SIZE;
        } else if (strcmp(argv[1], "window2670") == 0) {
            if (!otrail_unpacked_build_window_2670(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2670_SIZE;
        } else if (strcmp(argv[1], "window2df0") == 0) {
            if (!otrail_unpacked_build_window_2df0(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2DF0_SIZE;
        } else if (strcmp(argv[1], "window29a0") == 0) {
            if (!otrail_unpacked_build_window_29a0(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_29A0_SIZE;
        } else if (strcmp(argv[1], "window2b20") == 0) {
            if (!otrail_unpacked_build_window_2b20(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2B20_SIZE;
        } else if (strcmp(argv[1], "window2c00") == 0) {
            if (!otrail_unpacked_build_window_2c00(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2C00_SIZE;
        } else if (strcmp(argv[1], "window2d50") == 0) {
            if (!otrail_unpacked_build_window_2d50(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2D50_SIZE;
        } else if (strcmp(argv[1], "window1800") == 0) {
            if (!otrail_unpacked_build_window_1800(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_1800_SIZE;
        } else if (strcmp(argv[1], "window2310") == 0) {
            if (!otrail_unpacked_build_window_2310(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2310_SIZE;
        } else if (strcmp(argv[1], "window3130") == 0) {
            if (!otrail_unpacked_build_window_3130(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_3130_SIZE;
        } else {
            if (!otrail_unpacked_build_window_2f00_to_30aa(out)) {
                return 1;
            }
            out_size = OTRAIL_WINDOW_2F00_TO_30AA_SIZE;
        }
        printf("status=ok\n");
        printf("kind=%s\n", argv[1]);
        printf("bytes_hex=");
        print_hex(out, out_size);
        return 0;
    }

    return 1;
}

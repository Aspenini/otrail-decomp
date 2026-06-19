/*
 * Readable reconstruction helpers for the first validated post-unpack motifs.
 */

#ifndef OTRAIL_UNPACKED_RUNTIME_FRAGMENTS_H
#define OTRAIL_UNPACKED_RUNTIME_FRAGMENTS_H

#include <stddef.h>
#include <stdint.h>

#define OTRAIL_SEED_05FF_SIZE 4u
#define OTRAIL_SEED_0DC3_SIZE 3u
#define OTRAIL_BLOCK_05F0_SIZE 32u
#define OTRAIL_BLOCK_0DC0_SIZE 16u
#define OTRAIL_WINDOW_05A4_SIZE 253u
#define OTRAIL_WINDOW_06B0_SIZE 253u
#define OTRAIL_WINDOW_05E5_SIZE 200u
#define OTRAIL_WINDOW_0FD0_SIZE 200u
#define OTRAIL_WINDOW_0EFF_SIZE 172u
#define OTRAIL_WINDOW_0F20_SIZE 118u
#define OTRAIL_WINDOW_0F46_SIZE 80u
#define OTRAIL_WINDOW_05F5_SIZE 225u
#define OTRAIL_WINDOW_1B7A_SIZE 225u
#define OTRAIL_WINDOW_0D1A_SIZE 235u
#define OTRAIL_WINDOW_1C5F_SIZE 235u
#define OTRAIL_WINDOW_1F6D_SIZE 255u
#define OTRAIL_WINDOW_21B6_SIZE 255u
#define OTRAIL_WINDOW_2003_SIZE 30u
#define OTRAIL_WINDOW_207E_SIZE 30u
#define OTRAIL_WINDOW_110E_SIZE 32u
#define OTRAIL_WINDOW_2E90_SIZE 32u
#define OTRAIL_WINDOW_2F00_SIZE 64u
#define OTRAIL_WINDOW_2EF0_SIZE 80u
#define OTRAIL_WINDOW_2EC0_SIZE 128u
#define OTRAIL_WINDOW_2F40_SIZE 64u
#define OTRAIL_WINDOW_2F00_TO_2F80_SIZE 128u
#define OTRAIL_WINDOW_2A3A_SIZE 224u
#define OTRAIL_WINDOW_2A6E_SIZE 172u
#define OTRAIL_WINDOW_2A8F_SIZE 118u
#define OTRAIL_WINDOW_2AB5_SIZE 80u
#define OTRAIL_WINDOW_2CFA_SIZE 80u
#define OTRAIL_WINDOW_2C7F_SIZE 192u
#define OTRAIL_WINDOW_2F80_SIZE 64u
#define OTRAIL_WINDOW_2FC0_SIZE 64u
#define OTRAIL_WINDOW_2552_SIZE 64u
#define OTRAIL_WINDOW_2F00_TO_3000_SIZE 256u
#define OTRAIL_WINDOW_3000_SIZE 64u
#define OTRAIL_WINDOW_3040_SIZE 64u
#define OTRAIL_WINDOW_3080_SIZE 42u
#define OTRAIL_WINDOW_1909_SIZE 42u
#define OTRAIL_WINDOW_2612_SIZE 42u
#define OTRAIL_WINDOW_2F00_TO_30AA_SIZE 426u
#define OTRAIL_WINDOW_0850_SIZE 128u
#define OTRAIL_WINDOW_0950_SIZE 80u
#define OTRAIL_WINDOW_11C0_SIZE 128u
#define OTRAIL_WINDOW_2748_SIZE 80u
#define OTRAIL_WINDOW_09E0_SIZE 159u
#define OTRAIL_WINDOW_177C_SIZE 159u
#define OTRAIL_WINDOW_1D7F_SIZE 159u
#define OTRAIL_WINDOW_2500_SIZE 80u
#define OTRAIL_WINDOW_2670_SIZE 64u
#define OTRAIL_WINDOW_2DF0_SIZE 80u
#define OTRAIL_WINDOW_29A0_SIZE 64u
#define OTRAIL_WINDOW_2B20_SIZE 80u
#define OTRAIL_WINDOW_2C00_SIZE 64u
#define OTRAIL_WINDOW_2D50_SIZE 64u
#define OTRAIL_WINDOW_1800_SIZE 64u
#define OTRAIL_WINDOW_2310_SIZE 64u
#define OTRAIL_WINDOW_3130_SIZE 64u
#define OTRAIL_CANDIDATE_1803_PREFIX_SIZE 21u
#define OTRAIL_WINDOW_2000_PREFIX_SIZE 16u
#define OTRAIL_WINDOW_2000_SIZE 64u
#define OTRAIL_UNPACKED_RUNTIME_SPAN_MAX_SIZE 512u

typedef struct OtrailSeed05FFInputs {
    uint8_t source_0506_pair[2];
} OtrailSeed05FFInputs;

typedef struct OtrailSeed0DC3Inputs {
    uint8_t source_0DA8_byte;
} OtrailSeed0DC3Inputs;

typedef struct OtrailWindow2000PrefixInputs {
    uint8_t source_1230_tail[2];
    uint8_t seed_05ff[OTRAIL_SEED_05FF_SIZE];
    uint8_t seed_0dc3[OTRAIL_SEED_0DC3_SIZE];
} OtrailWindow2000PrefixInputs;

typedef struct OtrailBlock05F0Inputs {
    uint8_t source_05f6_triplet[3];
    uint8_t seed_05ff[OTRAIL_SEED_05FF_SIZE];
    uint8_t source_060b_quad[4];
} OtrailBlock05F0Inputs;

typedef struct OtrailBlock0DC0Inputs {
    uint8_t seed_0dc3[OTRAIL_SEED_0DC3_SIZE];
    uint8_t source_0dce_pair[2];
} OtrailBlock0DC0Inputs;

typedef struct OtrailCandidate1803RoutinePrefix {
    uint16_t loop_count;
    uint16_t far_call_offset;
    uint16_t far_call_segment;
    int8_t save_ax_bp_disp;
    int8_t save_bx_bp_disp;
    int8_t save_dx_bp_disp;
} OtrailCandidate1803RoutinePrefix;

int otrail_unpacked_build_seed_05ff(
    const OtrailSeed05FFInputs *in,
    uint8_t out[OTRAIL_SEED_05FF_SIZE]
);

int otrail_unpacked_build_seed_0dc3(
    const OtrailSeed0DC3Inputs *in,
    uint8_t out[OTRAIL_SEED_0DC3_SIZE]
);

int otrail_unpacked_build_block_05f0(
    const OtrailBlock05F0Inputs *in,
    uint8_t out[OTRAIL_BLOCK_05F0_SIZE]
);

int otrail_unpacked_build_block_0dc0(
    const OtrailBlock0DC0Inputs *in,
    uint8_t out[OTRAIL_BLOCK_0DC0_SIZE]
);

int otrail_unpacked_build_window_05a4(
    const OtrailBlock05F0Inputs *in,
    uint8_t out[OTRAIL_WINDOW_05A4_SIZE]
);

int otrail_unpacked_build_window_06b0(
    const OtrailBlock05F0Inputs *in,
    uint8_t out[OTRAIL_WINDOW_06B0_SIZE]
);

int otrail_unpacked_build_window_05e5(
    const OtrailBlock05F0Inputs *in,
    uint8_t out[OTRAIL_WINDOW_05E5_SIZE]
);

int otrail_unpacked_build_window_0fd0(
    const OtrailBlock05F0Inputs *in,
    uint8_t out[OTRAIL_WINDOW_0FD0_SIZE]
);

int otrail_unpacked_build_window_0eff(
    uint8_t out[OTRAIL_WINDOW_0EFF_SIZE]
);

int otrail_unpacked_build_window_0f20(
    uint8_t out[OTRAIL_WINDOW_0F20_SIZE]
);

int otrail_unpacked_build_window_0f46(
    uint8_t out[OTRAIL_WINDOW_0F46_SIZE]
);

int otrail_unpacked_build_window_05f5(
    const OtrailBlock05F0Inputs *in,
    uint8_t out[OTRAIL_WINDOW_05F5_SIZE]
);

int otrail_unpacked_build_window_1b7a(
    const OtrailBlock05F0Inputs *in,
    uint8_t out[OTRAIL_WINDOW_1B7A_SIZE]
);

int otrail_unpacked_build_window_0d1a(
    const OtrailBlock0DC0Inputs *in,
    uint8_t out[OTRAIL_WINDOW_0D1A_SIZE]
);

int otrail_unpacked_build_window_1c5f(
    const OtrailBlock0DC0Inputs *in,
    uint8_t out[OTRAIL_WINDOW_1C5F_SIZE]
);

int otrail_unpacked_build_window_1f6d(
    const OtrailWindow2000PrefixInputs *in,
    uint8_t out[OTRAIL_WINDOW_1F6D_SIZE]
);

int otrail_unpacked_build_window_21b6(
    const OtrailWindow2000PrefixInputs *in,
    uint8_t out[OTRAIL_WINDOW_21B6_SIZE]
);

int otrail_unpacked_build_window_2000_prefix(
    const OtrailWindow2000PrefixInputs *in,
    uint8_t out[OTRAIL_WINDOW_2000_PREFIX_SIZE]
);

int otrail_unpacked_build_window_2003(
    const OtrailWindow2000PrefixInputs *in,
    uint8_t out[OTRAIL_WINDOW_2003_SIZE]
);

int otrail_unpacked_build_window_207e(
    const OtrailWindow2000PrefixInputs *in,
    uint8_t out[OTRAIL_WINDOW_207E_SIZE]
);

int otrail_unpacked_build_window_110e(
    uint8_t out[OTRAIL_WINDOW_110E_SIZE]
);

int otrail_unpacked_build_window_2e90(
    uint8_t out[OTRAIL_WINDOW_2E90_SIZE]
);

int otrail_unpacked_build_window_2f00(
    uint8_t out[OTRAIL_WINDOW_2F00_SIZE]
);

int otrail_unpacked_build_window_2ef0(
    uint8_t out[OTRAIL_WINDOW_2EF0_SIZE]
);

int otrail_unpacked_build_window_2ec0(
    uint8_t out[OTRAIL_WINDOW_2EC0_SIZE]
);

int otrail_unpacked_build_window_2f40(
    uint8_t out[OTRAIL_WINDOW_2F40_SIZE]
);

int otrail_unpacked_build_window_2f00_to_2f80(
    uint8_t out[OTRAIL_WINDOW_2F00_TO_2F80_SIZE]
);

int otrail_unpacked_build_window_2a3a(
    uint8_t out[OTRAIL_WINDOW_2A3A_SIZE]
);

int otrail_unpacked_build_window_2a6e(
    uint8_t out[OTRAIL_WINDOW_2A6E_SIZE]
);

int otrail_unpacked_build_window_2a8f(
    uint8_t out[OTRAIL_WINDOW_2A8F_SIZE]
);

int otrail_unpacked_build_window_2ab5(
    uint8_t out[OTRAIL_WINDOW_2AB5_SIZE]
);

int otrail_unpacked_build_window_2cfa(
    uint8_t out[OTRAIL_WINDOW_2CFA_SIZE]
);

int otrail_unpacked_build_window_2c7f(
    uint8_t out[OTRAIL_WINDOW_2C7F_SIZE]
);

int otrail_unpacked_build_window_2f80(
    uint8_t out[OTRAIL_WINDOW_2F80_SIZE]
);

int otrail_unpacked_build_window_2fc0(
    uint8_t out[OTRAIL_WINDOW_2FC0_SIZE]
);

int otrail_unpacked_build_window_2552(
    uint8_t out[OTRAIL_WINDOW_2552_SIZE]
);

int otrail_unpacked_build_window_2f00_to_3000(
    uint8_t out[OTRAIL_WINDOW_2F00_TO_3000_SIZE]
);

int otrail_unpacked_build_window_3000(
    uint8_t out[OTRAIL_WINDOW_3000_SIZE]
);

int otrail_unpacked_build_window_3040(
    uint8_t out[OTRAIL_WINDOW_3040_SIZE]
);

int otrail_unpacked_build_window_3080(
    uint8_t out[OTRAIL_WINDOW_3080_SIZE]
);

int otrail_unpacked_build_window_1909(
    uint8_t out[OTRAIL_WINDOW_1909_SIZE]
);

int otrail_unpacked_build_window_2612(
    uint8_t out[OTRAIL_WINDOW_2612_SIZE]
);

int otrail_unpacked_build_window_2f00_to_30aa(
    uint8_t out[OTRAIL_WINDOW_2F00_TO_30AA_SIZE]
);


int otrail_unpacked_build_window_0850(
    uint8_t out[OTRAIL_WINDOW_0850_SIZE]
);

int otrail_unpacked_build_window_0950(
    uint8_t out[OTRAIL_WINDOW_0950_SIZE]
);

int otrail_unpacked_build_window_2748(
    uint8_t out[OTRAIL_WINDOW_2748_SIZE]
);



int otrail_unpacked_build_window_11c0(
    uint8_t out[OTRAIL_WINDOW_11C0_SIZE]
);

int otrail_unpacked_build_window_09e0(
    uint8_t out[OTRAIL_WINDOW_09E0_SIZE]
);

int otrail_unpacked_build_window_177c(
    uint8_t out[OTRAIL_WINDOW_177C_SIZE]
);

int otrail_unpacked_build_window_1d7f(
    uint8_t out[OTRAIL_WINDOW_1D7F_SIZE]
);

int otrail_unpacked_build_window_2500(
    uint8_t out[OTRAIL_WINDOW_2500_SIZE]
);

int otrail_unpacked_build_window_2670(
    uint8_t out[OTRAIL_WINDOW_2670_SIZE]
);

int otrail_unpacked_build_window_2df0(
    uint8_t out[OTRAIL_WINDOW_2DF0_SIZE]
);

int otrail_unpacked_build_window_29a0(
    uint8_t out[OTRAIL_WINDOW_29A0_SIZE]
);

int otrail_unpacked_build_window_2b20(
    uint8_t out[OTRAIL_WINDOW_2B20_SIZE]
);

int otrail_unpacked_build_window_2c00(
    uint8_t out[OTRAIL_WINDOW_2C00_SIZE]
);

int otrail_unpacked_build_window_2d50(
    uint8_t out[OTRAIL_WINDOW_2D50_SIZE]
);

int otrail_unpacked_build_window_1800(
    uint8_t out[OTRAIL_WINDOW_1800_SIZE]
);

int otrail_unpacked_build_candidate_1803_prefix(
    uint8_t out[OTRAIL_CANDIDATE_1803_PREFIX_SIZE]
);

int otrail_unpacked_describe_candidate_1803_prefix(
    OtrailCandidate1803RoutinePrefix *out
);

int otrail_unpacked_build_window_2310(
    uint8_t out[OTRAIL_WINDOW_2310_SIZE]
);

int otrail_unpacked_build_window_3130(
    uint8_t out[OTRAIL_WINDOW_3130_SIZE]
);

int otrail_unpacked_build_span(
    const char *span_id,
    uint8_t *out,
    size_t out_capacity,
    size_t *out_size
);

int otrail_unpacked_build_window_2000(
    const OtrailWindow2000PrefixInputs *in,
    uint8_t out[OTRAIL_WINDOW_2000_SIZE]
);

#endif

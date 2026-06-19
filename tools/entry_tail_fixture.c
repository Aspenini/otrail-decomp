/*
 * Fixture runner for the lifted unit_0004 / unit_0005 / unit_0006 tail semantics.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/unit_0004_entrypoint_contig_64.h"
#include "../src/unit_0005_entrypoint_contig_64.h"
#include "../src/unit_0006_entrypoint_contig_64.h"

static unsigned long parse_u32(const char *text) {
    return strtoul(text, 0, 0);
}

static void print_hex(const uint8_t *bytes, size_t size, const char *label) {
    size_t i;

    printf("%s=", label);
    for (i = 0; i < size; ++i) {
        printf("%02X", bytes[i]);
    }
    printf("\n");
}

static void print_patch_result(const Unit0005TailDispatch *dispatch) {
    printf("kind=patch\n");
    printf("next_dx_rel=0x%04X\n", dispatch->next_state.dx_rel);
    printf("next_di=0x%04X\n", dispatch->next_state.di);
    printf("patch_off=0x%04X\n", dispatch->patch_off);
    printf("patch_increment=0x%04X\n", dispatch->patch_increment);
    printf("consumed_low_byte_only=%u\n", (unsigned)dispatch->consumed_low_byte_only);
}

int main(int argc, char **argv) {
    Unit0005TailState state;
    Unit0005TailDispatch dispatch;

    if (argc < 2) {
        fprintf(stderr, "usage:\n");
        fprintf(stderr, "  %s bootstrap <popped_bx> <first_byte> <control_word> [jump_off jump_seg stack_off stack_seg]\n", argv[0]);
        fprintf(stderr, "  %s byte <bx_seed> <dx_rel> <di> <value>\n", argv[0]);
        fprintf(stderr, "  %s control <bx_seed> <dx_rel> <di> <control_word> [jump_off jump_seg stack_off stack_seg]\n", argv[0]);
        fprintf(stderr, "  %s slice\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "slice") == 0) {
        Unit0006MixedSlice slice;
        if (!unit_0006_entrypoint_describe_mixed_slice(&slice)) {
            fprintf(stderr, "slice describe failed\n");
            return 1;
        }
        printf("status=ok\n");
        printf("kind=slice\n");
        printf("begins_mid_instruction=%u\n", (unsigned)slice.begins_mid_instruction);
        printf("leading_xor_tail_byte=0x%02X\n", (unsigned)slice.leading_xor_tail_byte);
        printf("handoff_code_size=%u\n", (unsigned)slice.handoff_code_size);
        printf("postjump_data_size=%u\n", (unsigned)slice.postjump_data_size);
        print_hex(
            unit_0006_entrypoint_handoff_code_bytes,
            sizeof(unit_0006_entrypoint_handoff_code_bytes),
            "handoff_code_hex"
        );
        print_hex(
            unit_0006_entrypoint_postjump_data_prefix_bytes,
            sizeof(unit_0006_entrypoint_postjump_data_prefix_bytes),
            "postjump_data_hex"
        );
        return 0;
    }

    if (strcmp(argv[1], "bootstrap") == 0) {
        Unit0004TailBootstrap bootstrap;
        if (argc < 5) {
            fprintf(stderr, "bootstrap mode requires popped_bx first_byte control_word\n");
            return 1;
        }
        if (!unit_0004_entrypoint_bootstrap_relocation_tail(
                (uint16_t)parse_u32(argv[2]),
                (uint8_t)parse_u32(argv[3]),
                (uint16_t)parse_u32(argv[4]),
                &bootstrap
            )) {
            fprintf(stderr, "bootstrap dispatch failed\n");
            return 1;
        }
        printf("status=ok\n");
        printf("initial_bx_seed=0x%04X\n", bootstrap.initial_state.bx_seed);
        printf("initial_dx_rel=0x%04X\n", bootstrap.initial_state.dx_rel);
        printf("initial_di=0x%04X\n", bootstrap.initial_state.di);
        printf("next_stream_offset=0x%04X\n", bootstrap.next_stream_offset);
        printf("first_stream_byte=0x%02X\n", bootstrap.first_stream_byte);
        printf("first_control_word=0x%04X\n", bootstrap.first_control_word);
        printf("consumed_control_word=%u\n", (unsigned)bootstrap.consumed_control_word);

        dispatch = bootstrap.dispatch;
        if (dispatch.kind == UNIT0005_TAIL_PATCH) {
            print_patch_result(&dispatch);
            return 0;
        }
        if (dispatch.kind == UNIT0005_TAIL_SEGMENT_BUMP) {
            printf("kind=segment_bump\n");
            printf("next_dx_rel=0x%04X\n", dispatch.next_state.dx_rel);
            printf("next_di=0x%04X\n", dispatch.next_state.di);
            return 0;
        }
        if (dispatch.kind == UNIT0005_TAIL_TERMINAL) {
            Unit0006TerminalHeader header;
            Unit0006TerminalHandoff handoff;
            if (argc < 9) {
                fprintf(stderr, "bootstrap terminal requires header words\n");
                return 1;
            }
            header.jump_off = (uint16_t)parse_u32(argv[5]);
            header.jump_seg_word = (uint16_t)parse_u32(argv[6]);
            header.stack_off = (uint16_t)parse_u32(argv[7]);
            header.stack_seg_rel = (uint16_t)parse_u32(argv[8]);
            if (!unit_0006_entrypoint_finalize_terminal_handoff(
                    bootstrap.initial_state.bx_seed,
                    &header,
                    &handoff
                )) {
                fprintf(stderr, "bootstrap terminal finalize failed\n");
                return 1;
            }
            printf("kind=terminal\n");
            printf("final_ds=0x%04X\n", handoff.final_ds);
            printf("final_es=0x%04X\n", handoff.final_es);
            printf("final_ss=0x%04X\n", handoff.final_ss);
            printf("final_sp=0x%04X\n", handoff.final_sp);
            printf("jump_off=0x%04X\n", handoff.jump_off);
            printf("jump_seg=0x%04X\n", handoff.jump_seg);
            printf("cli_seen=%u\n", (unsigned)handoff.cli_seen);
            printf("sti_seen=%u\n", (unsigned)handoff.sti_seen);
            printf("bx_cleared=%u\n", (unsigned)handoff.bx_cleared);
            return 0;
        }
        fprintf(stderr, "unknown bootstrap dispatch kind\n");
        return 1;
    }

    if (argc < 6) {
        fprintf(stderr, "byte/control mode requires bx_seed dx_rel di and value/control\n");
        return 1;
    }

    state.bx_seed = (uint16_t)parse_u32(argv[2]);
    state.dx_rel = (uint16_t)parse_u32(argv[3]);
    state.di = (uint16_t)parse_u32(argv[4]);

    if (strcmp(argv[1], "byte") == 0) {
        if (!unit_0005_entrypoint_tail_apply_byte(&state, (uint8_t)parse_u32(argv[5]), &dispatch)) {
            fprintf(stderr, "byte dispatch failed\n");
            return 1;
        }
        printf("status=ok\n");
        print_patch_result(&dispatch);
        return 0;
    }

    if (strcmp(argv[1], "control") == 0) {
        uint16_t control_word = (uint16_t)parse_u32(argv[5]);
        if (!unit_0005_entrypoint_tail_apply_control(&state, control_word, &dispatch)) {
            fprintf(stderr, "control dispatch failed\n");
            return 1;
        }
        printf("status=ok\n");
        if (dispatch.kind == UNIT0005_TAIL_PATCH) {
            print_patch_result(&dispatch);
            return 0;
        }
        if (dispatch.kind == UNIT0005_TAIL_SEGMENT_BUMP) {
            printf("kind=segment_bump\n");
            printf("next_dx_rel=0x%04X\n", dispatch.next_state.dx_rel);
            printf("next_di=0x%04X\n", dispatch.next_state.di);
            return 0;
        }
        if (dispatch.kind == UNIT0005_TAIL_TERMINAL) {
            Unit0006TerminalHeader header;
            Unit0006TerminalHandoff handoff;
            if (argc < 10) {
                fprintf(stderr, "terminal control requires header words\n");
                return 1;
            }
            header.jump_off = (uint16_t)parse_u32(argv[6]);
            header.jump_seg_word = (uint16_t)parse_u32(argv[7]);
            header.stack_off = (uint16_t)parse_u32(argv[8]);
            header.stack_seg_rel = (uint16_t)parse_u32(argv[9]);
            if (!unit_0006_entrypoint_finalize_terminal_handoff(state.bx_seed, &header, &handoff)) {
                fprintf(stderr, "terminal finalize failed\n");
                return 1;
            }
            printf("kind=terminal\n");
            printf("final_ds=0x%04X\n", handoff.final_ds);
            printf("final_es=0x%04X\n", handoff.final_es);
            printf("final_ss=0x%04X\n", handoff.final_ss);
            printf("final_sp=0x%04X\n", handoff.final_sp);
            printf("jump_off=0x%04X\n", handoff.jump_off);
            printf("jump_seg=0x%04X\n", handoff.jump_seg);
            printf("cli_seen=%u\n", (unsigned)handoff.cli_seen);
            printf("sti_seen=%u\n", (unsigned)handoff.sti_seen);
            printf("bx_cleared=%u\n", (unsigned)handoff.bx_cleared);
            return 0;
        }
    }

    fprintf(stderr, "unknown mode: %s\n", argv[1]);
    return 1;
}

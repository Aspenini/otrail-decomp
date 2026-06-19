/*
 * Search candidate CS/BX placements for the exact lifted 0x127EC relocation tail.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/unit_0004_entrypoint_contig_64.h"
#include "../src/unit_0005_entrypoint_contig_64.h"
#include "../src/unit_0006_entrypoint_contig_64.h"

#define TAIL_MEM_SIZE 0x10000u
#define TAIL_STREAM_OFFSET 0x0158u

static uint16_t read_u16le(const uint8_t *buf, uint16_t off) {
    return (uint16_t)buf[off] | ((uint16_t)buf[(uint16_t)(off + 1u)] << 8);
}

static void write_u16le(uint8_t *buf, uint16_t off, uint16_t value) {
    buf[off] = (uint8_t)(value & 0xFFu);
    buf[(uint16_t)(off + 1u)] = (uint8_t)((value >> 8) & 0xFFu);
}

static int is_control_flow_byte(uint8_t value) {
    return value == 0xE8u || value == 0xE9u || value == 0xEAu || value == 0x9Au ||
        value == 0xC3u || value == 0xCBu || value == 0xCFu;
}

static size_t leading_zero_prefix(const uint8_t *buf, size_t len) {
    size_t i;
    for (i = 0; i < len; i++) {
        if (buf[i] != 0u) {
            return i;
        }
    }
    return len;
}

static void emit_candidate(
    uint16_t cs_base,
    uint16_t bx_seed,
    size_t steps,
    size_t segment_bumps,
    size_t header_hits,
    const Unit0006TerminalHandoff *handoff,
    size_t payload_size,
    const uint8_t *payload,
    size_t sample_len
) {
    uint32_t jump_linear_20 = ((((uint32_t)handoff->jump_seg) << 4) + (uint32_t)handoff->jump_off) & 0xFFFFFu;
    uint16_t jump_linear_64k = (uint16_t)(jump_linear_20 & 0xFFFFu);
    size_t available;
    size_t i;
    size_t target_nonzero = 0;
    size_t target_ctrl = 0;
    size_t target_ascii = 0;
    size_t target_zero_prefix;
    int target_first_ctrl = -1;

    if ((size_t)jump_linear_64k >= payload_size) {
        return;
    }

    available = payload_size - (size_t)jump_linear_64k;
    if (available > sample_len) {
        available = sample_len;
    }
    target_zero_prefix = leading_zero_prefix(payload + jump_linear_64k, available);
    for (i = 0; i < available; i++) {
        uint8_t byte = payload[(size_t)jump_linear_64k + i];
        if (byte != 0u) {
            target_nonzero++;
        }
        if (is_control_flow_byte(byte)) {
            target_ctrl++;
            if (target_first_ctrl < 0) {
                target_first_ctrl = (int)i;
            }
        }
        if (byte >= 32u && byte < 127u) {
            target_ascii++;
        }
    }

    printf(
        "cs_base=0x%04X\tbx_seed=0x%04X\tsteps=%lu\tsegment_bumps=%lu\theader_hits=%lu\t"
        "jump_off=0x%04X\tjump_seg_word=0x%04X\tjump_seg_final=0x%04X\tjump_linear_20=0x%05lX\tjump_linear_64k=0x%04X\t"
        "stack_off=0x%04X\tstack_seg_rel=0x%04X\tfinal_ds=0x%04X\tfinal_es=0x%04X\tfinal_ss=0x%04X\tfinal_sp=0x%04X\t"
        "target_nonzero=%lu\ttarget_ctrl=%lu\ttarget_zero_prefix=%lu\ttarget_ascii=%lu\t"
        "target_first_ctrl=%d\n",
        cs_base,
        bx_seed,
        (unsigned long)steps,
        (unsigned long)segment_bumps,
        (unsigned long)header_hits,
        handoff->jump_off,
        (uint16_t)(handoff->jump_seg - bx_seed),
        handoff->jump_seg,
        (unsigned long)jump_linear_20,
        jump_linear_64k,
        handoff->final_sp,
        (uint16_t)(handoff->final_ss - bx_seed),
        handoff->final_ds,
        handoff->final_es,
        handoff->final_ss,
        handoff->final_sp,
        (unsigned long)target_nonzero,
        (unsigned long)target_ctrl,
        (unsigned long)target_zero_prefix,
        (unsigned long)target_ascii,
        target_first_ctrl
    );
}

static void search_one_basis(
    const uint8_t *payload,
    size_t payload_size,
    uint16_t cs_base,
    uint16_t bx_seed,
    size_t max_steps,
    size_t sample_len
) {
    uint8_t mem[TAIL_MEM_SIZE];
    Unit0004TailBootstrap bootstrap;
    Unit0005TailState state;
    uint16_t si;
    size_t steps = 0;
    size_t segment_bumps = 0;
    size_t header_hits = 0;

    memset(mem, 0, sizeof(mem));
    if (payload_size > sizeof(mem)) {
        payload_size = sizeof(mem);
    }
    memcpy(mem, payload, payload_size);

    si = (uint16_t)(cs_base + TAIL_STREAM_OFFSET);
    if (!unit_0004_entrypoint_bootstrap_relocation_tail(
            (uint16_t)(bx_seed - 0x0010u),
            mem[si],
            read_u16le(mem, (uint16_t)(si + 1u)),
            &bootstrap
        )) {
        return;
    }
    state = bootstrap.initial_state;
    si = (uint16_t)(cs_base + bootstrap.next_stream_offset);

    if (bootstrap.dispatch.kind == UNIT0005_TAIL_SEGMENT_BUMP) {
        state = bootstrap.dispatch.next_state;
        segment_bumps++;
        steps++;
    } else if (bootstrap.dispatch.kind == UNIT0005_TAIL_TERMINAL) {
        Unit0006TerminalHeader header;
        Unit0006TerminalHandoff handoff;
        header.jump_off = read_u16le(mem, cs_base);
        header.jump_seg_word = read_u16le(mem, (uint16_t)(cs_base + 2u));
        header.stack_off = read_u16le(mem, (uint16_t)(cs_base + 4u));
        header.stack_seg_rel = read_u16le(mem, (uint16_t)(cs_base + 6u));
        if (!unit_0006_entrypoint_finalize_terminal_handoff(
                bootstrap.initial_state.bx_seed,
                &header,
                &handoff
            )) {
            return;
        }
        steps++;
        emit_candidate(
            cs_base,
            bx_seed,
            steps,
            segment_bumps,
            header_hits,
            &handoff,
            payload_size,
            payload,
            sample_len
        );
        return;
    } else {
        if (bootstrap.dispatch.patch_off < cs_base + 8u &&
            bootstrap.dispatch.patch_off >= cs_base) {
            header_hits++;
        } else if ((uint16_t)(bootstrap.dispatch.patch_off + 1u) < cs_base + 8u &&
            (uint16_t)(bootstrap.dispatch.patch_off + 1u) >= cs_base) {
            header_hits++;
        }
        write_u16le(
            mem,
            bootstrap.dispatch.patch_off,
            (uint16_t)(read_u16le(mem, bootstrap.dispatch.patch_off) + bootstrap.dispatch.patch_increment)
        );
        state = bootstrap.dispatch.next_state;
        steps++;
    }

    while (steps < max_steps) {
        uint8_t value = mem[si];
        Unit0005TailDispatch dispatch;
        si = (uint16_t)(si + 1u);

        if (value == 0u) {
            uint16_t control_word = read_u16le(mem, si);
            si = (uint16_t)(si + 2u);
            if (!unit_0005_entrypoint_tail_apply_control(&state, control_word, &dispatch)) {
                return;
            }
        } else {
            if (!unit_0005_entrypoint_tail_apply_byte(&state, value, &dispatch)) {
                return;
            }
        }

        if (dispatch.kind == UNIT0005_TAIL_SEGMENT_BUMP) {
            state = dispatch.next_state;
            segment_bumps++;
            steps++;
            continue;
        }

        if (dispatch.kind == UNIT0005_TAIL_TERMINAL) {
            Unit0006TerminalHeader header;
            Unit0006TerminalHandoff handoff;
            header.jump_off = read_u16le(mem, cs_base);
            header.jump_seg_word = read_u16le(mem, (uint16_t)(cs_base + 2u));
            header.stack_off = read_u16le(mem, (uint16_t)(cs_base + 4u));
            header.stack_seg_rel = read_u16le(mem, (uint16_t)(cs_base + 6u));
            if (!unit_0006_entrypoint_finalize_terminal_handoff(bx_seed, &header, &handoff)) {
                return;
            }
            emit_candidate(
                cs_base,
                bx_seed,
                steps,
                segment_bumps,
                header_hits,
                &handoff,
                payload_size,
                payload,
                sample_len
            );
            return;
        }

        if (dispatch.patch_off < cs_base + 8u &&
            dispatch.patch_off >= cs_base) {
            header_hits++;
        } else if ((uint16_t)(dispatch.patch_off + 1u) < cs_base + 8u &&
            (uint16_t)(dispatch.patch_off + 1u) >= cs_base) {
            header_hits++;
        }
        write_u16le(
            mem,
            dispatch.patch_off,
            (uint16_t)(read_u16le(mem, dispatch.patch_off) + dispatch.patch_increment)
        );
        state = dispatch.next_state;
        steps++;
    }
}

int main(int argc, char **argv) {
    const char *payload_path = "build/entry_bootstrap_replay_readable_heuristic.bin";
    uint32_t max_cs_base = 0x4000u;
    uint32_t cs_step = 0x10u;
    uint32_t bx_start = 0x0010u;
    uint32_t bx_end = 0x0010u;
    uint32_t bx_step = 0x10u;
    uint32_t max_steps = 20000u;
    uint32_t sample_len = 128u;
    FILE *f;
    long payload_size_long;
    size_t payload_size;
    uint8_t *payload;
    uint32_t cs_base;
    uint32_t bx_seed;

    if (argc >= 2) payload_path = argv[1];
    if (argc >= 3) max_cs_base = (uint32_t)strtoul(argv[2], 0, 0);
    if (argc >= 4) cs_step = (uint32_t)strtoul(argv[3], 0, 0);
    if (argc >= 5) bx_start = (uint32_t)strtoul(argv[4], 0, 0);
    if (argc >= 6) bx_end = (uint32_t)strtoul(argv[5], 0, 0);
    if (argc >= 7) bx_step = (uint32_t)strtoul(argv[6], 0, 0);
    if (argc >= 8) max_steps = (uint32_t)strtoul(argv[7], 0, 0);
    if (argc >= 9) sample_len = (uint32_t)strtoul(argv[8], 0, 0);
    if (cs_step == 0u || bx_step == 0u) {
        fprintf(stderr, "steps must be non-zero\n");
        return 1;
    }

    f = fopen(payload_path, "rb");
    if (!f) {
        fprintf(stderr, "failed to open %s\n", payload_path);
        return 1;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 1;
    }
    payload_size_long = ftell(f);
    if (payload_size_long <= 0) {
        fclose(f);
        return 1;
    }
    payload_size = (size_t)payload_size_long;
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return 1;
    }
    payload = (uint8_t *)malloc(payload_size);
    if (!payload) {
        fclose(f);
        return 1;
    }
    if (fread(payload, 1, payload_size, f) != payload_size) {
        fclose(f);
        free(payload);
        return 1;
    }
    fclose(f);

    printf(
        "cs_base\tbx_seed\tsteps\tsegment_bumps\theader_hits\tjump_off\tjump_seg_word\tjump_seg_final\tjump_linear_20\tjump_linear_64k\t"
        "stack_off\tstack_seg_rel\tfinal_ds\tfinal_es\tfinal_ss\tfinal_sp\ttarget_nonzero\ttarget_ctrl\ttarget_zero_prefix\ttarget_ascii\t"
        "target_first_ctrl\n"
    );
    for (bx_seed = bx_start; bx_seed <= bx_end; bx_seed += bx_step) {
        for (cs_base = 0u; cs_base < max_cs_base && cs_base + TAIL_STREAM_OFFSET < payload_size; cs_base += cs_step) {
            search_one_basis(
                payload,
                payload_size,
                (uint16_t)cs_base,
                (uint16_t)bx_seed,
                (size_t)max_steps,
                (size_t)sample_len
            );
        }
        if (bx_end - bx_seed < bx_step) {
            break;
        }
    }

    free(payload);
    return 0;
}

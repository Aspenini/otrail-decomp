/*
 * Public declarations for the lifted unit_0003 token decoder slice.
 */

#ifndef UNIT_0003_ENTRYPOINT_CONTIG_64_H
#define UNIT_0003_ENTRYPOINT_CONTIG_64_H

#include <stddef.h>
#include <stdint.h>

typedef struct Unit0003BitCursor {
    const uint8_t *src;
    size_t src_len;
    size_t src_pos;
    uint16_t bitbuf;
    uint8_t bits_left;
} Unit0003BitCursor;

typedef enum Unit0003TokenKind {
    UNIT0003_TOKEN_SHORT_BACKREF = 0,
    UNIT0003_TOKEN_LONG_BACKREF = 1,
    UNIT0003_TOKEN_LONG_CONTROL = 2
} Unit0003TokenKind;

typedef struct Unit0003TokenState {
    Unit0003TokenKind kind;
    int16_t backref_disp;
    uint16_t copy_len;
    uint8_t control_byte;
    uint8_t needs_unit_0004_resolution;
} Unit0003TokenState;

int unit_0003_entrypoint_resume_nonliteral_token(
    Unit0003BitCursor *cursor,
    int take_long_path,
    Unit0003TokenState *out
);

#endif

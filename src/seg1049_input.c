/*
 * seg1049_input.c - keyboard input (segment 0x1049).
 *
 *   read_field   @ 0x1049:0x0d95  (far) - read a text field, with Ctrl-S handling
 *   read_input   @ 0x1049:0x0c12  (near) - the core char-reader (charset-filtered)
 *
 * This is the input path used by every prompt in the game, so it is on the
 * critical path for a port. The contract (below) is what a PAL backend must
 * reproduce.
 *
 * Address-annotated reconstruction; not yet compile-verified.
 *
 * --- input contract (for the port) ---------------------------------------
 * read_field(dst, x, y, maxlen, charset):
 *   - reads a line of up to `maxlen` characters at screen (x, y), echoing each;
 *   - only characters present in `charset` (a Pascal/counted string like
 *     "A-Za-z '.-" or "1-9") are accepted; others are ignored;
 *   - the result lands in `dst` as a COUNTED string: dst[0] = length,
 *     dst[1..] = the characters (this is why call sites read g_input_buf
 *     length-first, e.g. `if (g_input_buf == 0)` tests for an empty entry);
 *   - Enter finishes the field; Esc sets the global quit flag (g_quit_flag);
 *   - Ctrl-S (key code 0x13) toggles sound (g_sound_on) at any time and the
 *     field keeps reading.
 *   The last key code read is kept in g_last_key (0x1796).
 */

#include <stdint.h>

extern uint8_t  g_quit_flag;   /* 0x1520 */
extern uint8_t  g_sound_on;    /* 0x1410: sound toggle (Ctrl-S)  */
extern uint16_t g_1412;        /* 0x1412: input state, reset on entry */
extern uint8_t  g_last_key;    /* 0x1796: last key code read     */
extern uint16_t g_1416;        /* 0x1416: input sub-state        */

extern void strncpy_n(char far *dst, const char far *src, int n);  /* 0x20a4:0x064e */
extern void read_input_1049_0c12(const char far *charset, char far *dst, int x, int y); /* 0x1049:0x0c12 */

#define KEY_CTRL_S 0x13   /* Ctrl-S: toggle sound */

/* ---------------------------------------------------------- 0x1049:0x0d95
 * Read a text field. Loops the core reader, handling Ctrl-S (sound toggle) by
 * re-reading; returns when Enter or Esc ends the field.
 */
void read_field(char far *dst, int x, int y, int maxlen, const char far *charset)
{
    char prompt[256];
    int  ctrl_s;

    strncpy_n(prompt, charset, 0xFF);                  /* 0x0DB3 */
    g_1412 = 0;                                         /* 0x0DBA */

    do {
        read_input_1049_0c12(charset, dst, maxlen, x);  /* 0x0DD1: core read */

        /* Ctrl-S pressed during the field? */
        ctrl_s = (g_last_key == KEY_CTRL_S && g_1416 == 4);  /* 0x0DD4 */
        if (ctrl_s)
            g_sound_on = !g_sound_on;                   /* 0x0DF3..0x0E00: toggle sound */
    } while (ctrl_s && !g_quit_flag);                   /* 0x0E03: re-read after a toggle */
}                                                       /* 0x0E14 retf */

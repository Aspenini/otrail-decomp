/*
 * seg_c6b_learn.c - "Learn about the Trail" info screens (menu option 2).
 *
 *   learn_about_trail @ 0x0c6b:0x051b  (far)
 *
 * Recovered from build/OREGON_unpacked.exe. An eight-page slideshow: each page
 * draws text, shows a press-any-key prompt, then clears the text area; every
 * page is gated on g_quit_flag so Esc-Esc backs out to the main menu.
 *
 * Text x is the left margin (g_text_margin_x); y is measured down from the top
 * of the text area (g_text_top_y). All page strings live in this segment's
 * CONST pool and are quoted in comments. Address-annotated reconstruction of
 * Borland Turbo C output; not yet compile-verified.
 */

#include <stdint.h>

extern uint8_t  g_quit_flag;      /* 0x1520 */
extern uint16_t g_text_margin_x;  /* 0x06a0: left margin x for body text       */
extern uint16_t g_text_top_y;     /* 0x1582: top y of the text area            */

/* Text helpers (segment 0x1049). */
extern void draw_text(const char far *s, int x, int y);        /* 0x1049:0x1855 multi-line */
extern void draw_text_line(const char far *s, int x, int y);   /* 0x1049:0x1b3c single line */
extern void press_any_key(void);                               /* 0x1049:0x15a0 */
extern void clear_text_area(void);                             /* 0x1049:0x1e43 */

/* CONST pool string addresses in this segment (cs:NN); content shown inline. */
#define S(off) ((const char far *)(off))

void learn_about_trail(void)                                   /* 0x051B */
{
    int x = g_text_margin_x;
    int y0 = g_text_top_y;

    /* "Try taking a journey by covered wagon across 2000 miles ..." */
    draw_text(S(0x000), x, y0 + 0x12); press_any_key(); clear_text_area();   /* 0x0539 */
    if (g_quit_flag) return;

    /* "How will you cross the rivers? ..." */
    draw_text(S(0x0da), x, y0 + 0x1c); press_any_key(); clear_text_area();   /* 0x0562 */
    if (g_quit_flag) return;

    /* "What about supplies? ... hunt ... buffalo ... bear ..." */
    draw_text(S(0x18a), x, y0 + 0x1c); press_any_key(); clear_text_area();   /* 0x058B */
    if (g_quit_flag) return;

    /* "At the Dalles, you can try navigating the Columbia River ..." */
    draw_text(S(0x217), x, y0 + 0x1c); press_any_key(); clear_text_area();   /* 0x05B4 */
    if (g_quit_flag) return;

    /* "If for some reason you don't survive ... The Oregon Top Ten." */
    draw_text(S(0x2b0), x, y0 + 0x12); press_any_key(); clear_text_area();   /* 0x05DD */
    if (g_quit_flag) return;

    /* Control-S page: heading + body. */
    draw_text_line(S(0x3a2), x, y0 + 0x1e);   /* "Control-S key"            0x0606 */
    draw_text(S(0x3b0), x, y0 + 0x30);        /* "You may turn the sound..."0x061B */
    press_any_key(); clear_text_area();
    if (g_quit_flag) return;

    /* Esc page: heading + body. */
    draw_text_line(S(0x3fb), x, y0 + 0x1e);   /* "Esc key"                  0x0644 */
    draw_text(S(0x403), x, y0 + 0x30);        /* "You may want to quit ..." 0x0659 */
    press_any_key(); clear_text_area();
    if (g_quit_flag) return;

    /* Credits page (positions baked into draw_text_line by offset). */
    draw_text_line(S(0x490), x, y0);          /* "The software team responsible"   */
    draw_text_line(S(0x4ae), x, y0);          /* "for creation of this product..." */
    draw_text_line(S(0x4d5), x, y0);          /* "Ed Gratz"            */
    draw_text_line(S(0x4de), x, y0);          /* "Charolyn Kapplinger" */
    draw_text_line(S(0x4f2), x, y0);          /* "Mark Paquette"       */
    draw_text_line(S(0x500), x, y0);          /* "Larry Phenow"        */
    draw_text_line(S(0x50d), x, y0);          /* "Julie Redland"       */
    press_any_key(); clear_text_area();                                       /* 0x06F0 */
}                                                                             /* 0x06FD retf */

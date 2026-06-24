/*
 * seg_c00_topten.c - "See the Oregon Top Ten" (main menu option 3).
 *
 *   see_top_ten @ 0x0c00:0x0433  (far)
 *
 * Draws the framed high-score screen, renders the current Top Ten list, then
 * offers to explain how points are earned.
 *
 * Address-annotated reconstruction of Borland Turbo C output; not yet
 * compile-verified against the binary.
 */

#include <stdint.h>

extern uint8_t  g_input_buf;     /* 0x141a */
extern uint8_t  g_topten_cur[];  /* 0x1674: current Top Ten list data */

extern void draw_text(const char far *s, int x, int y);      /* 0x1049:0x1855 */
extern void draw_text_line(const char far *s, int x, int y); /* 0x1049:0x1b3c */
extern void show_top_ten(const void far *list);              /* 0x1049:0x2a47 */
extern void sub_1049_1c5f(void);                             /* 0x1049:0x1c5f */
extern void read_input_var(void far *dst);                   /* 0x1049:0x1bb3 */
extern void show_input_prompt_2042_0215(int a, int b);       /* 0x2042:0x0215 */
extern void gfx_screen_init_1ceb_0b71(void);                 /* 0x1ceb:0x0b71 */
extern void gfx_1ceb_17db(int a);                            /* 0x1ceb:0x17db */
extern void gfx_box_1ceb_16a1(int x1, int y1, int x2, int y2); /* 0x1ceb:0x16a1 */

#define S(off) ((const char far *)(off))

/* ---------------------------------------------------------- 0x0c00:0x0433 */
void see_top_ten(void)
{
    gfx_screen_init_1ceb_0b71();                        /* 0x0441 */
    gfx_1ceb_17db(0);                                   /* 0x044A */
    gfx_box_1ceb_16a1(0, 0, 0, 0);                      /* 0x045C: outer frame */
    gfx_box_1ceb_16a1(0, 0, 0, 0);                      /* 0x0470: inner frame */

    draw_text_line(S(0xd7) /* "The Oregon Top Ten" */, 0xa0, 7);  /* 0x0482 */
    show_top_ten(g_topten_cur);                         /* 0x0495: render list from 0x1674 */
    sub_1049_1c5f();                                    /* 0x04AE */

    /* "Would you like to see how points are earned? " */
    draw_text(S(0xeb), 0, 0);                           /* 0x04C0 */
    show_input_prompt_2042_0215(0, 0);                  /* 0x04CB */
    read_input_var(&g_input_buf);                       /* 0x04D5: read Y/N */
    /* (if yes, the scoring-explanation screen is shown) */
}

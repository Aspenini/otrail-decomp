/*
 * seg_7ce_ending.c - the endgame scoring screen (segment 0x07ce).
 *
 *   show_ending @ 0x07ce:0x039a  (far)
 *
 * Reached from travel_loop after arriving in Oregon (g_game_over). Congratulates
 * the player, tallies the final score, and -- if it's high enough -- adds them
 * to the Oregon Top Ten (HISCORES.REC).
 *
 * Oregon Trail scoring: points are awarded for surviving party members, leftover
 * supplies, and cash, then multiplied by an occupation/difficulty factor (the
 * "7 - departure_month" style factor set in start_new_game; the farmer earns the
 * most, the banker the least). The exact arithmetic is 32-bit long math and is
 * summarised here.
 *
 * Address-annotated structural reconstruction; not yet compile-verified.
 */

#include <stdint.h>

extern uint8_t  g_quit_flag;     /* 0x1520 */
extern uint8_t  g_input_buf;     /* 0x141a */
extern long     g_score_factor;  /* 0x15ec: occupation/difficulty multiplier */
extern uint16_t g_15d8, g_15da, g_15dc;  /* 0x15d8: running score accumulator (long) */
extern uint8_t  g_topten_cur[];  /* 0x1674: current Top Ten list */

extern void draw_text(const char far *s, int x, int y);   /* 0x1049:0x1855 */
extern void press_any_key(void);                          /* 0x1049:0x15a0 */
extern void finalize_action_1049_1ee3(void);              /* 0x1049:0x1ee3 */
extern int  party_size_1049_2e57(void);                   /* 0x1049:0x2e57: living members */
extern void far_sprintf(/* ... */);                       /* 0x20a4:0x0634 */
extern void far_print(const char far *s);                 /* 0x20a4:0x06c1 */
extern void read_field(int a, int b, const char far *cs, void far *dst); /* 0x1049:0x0d95 */
extern void read_input(void far *dst);                    /* 0x1049:0x1bb3 */
extern int  streq(const char far *a, const char far *b);  /* 0x20a4:0x0724 */
extern void gfx_screen_init_1ceb_0b71(void);              /* 0x1ceb:0x0b71 */
extern int  topten_qualifies(long score);                 /* compares to the Top Ten cutoff */
extern void topten_insert(const char far *name, long score); /* add to HISCORES.REC */

#define S(off) ((const char far *)(off))

/* ---------------------------------------------------------- 0x07ce:0x039a */
void show_ending(void)
{
    long score;
    char name[16];

    /* "Congratulations! You have made it to Oregon! Let's see how many points
     * you have received." */
    draw_text(S(0x14d), 0, 0);                          /* 0x03C6 */
    press_any_key();                                    /* 0x03D0 */
    finalize_action_1049_1ee3();                        /* 0x03D5 */
    if (g_quit_flag) return;                            /* 0x03DA */

    /* --- score breakdown --- */
    gfx_screen_init_1ceb_0b71();                        /* 0x03E4 */
    far_sprintf(/* "Points for arriving in Oregon" cs:0x1a9 */);  /* 0x03E9.. */
    /* For each surviving party member, for leftover supplies, and for cash,
     * award points and multiply by g_score_factor. The running total lives in
     * the long at g_15d8. (party_size @ 0x047C, long math @ 0x0495..) */
    (void)party_size_1049_2e57();
    score = (long)g_15d8 | ((long)g_15da << 16);        /* accumulated points */

    /* "<X> points are <category>" lines (cs:0x290 " points are ") summarise
     * each scoring category as it is added. */

    /* --- qualify for the Oregon Top Ten? --- */
    if (topten_qualifies(score)) {
        /* "Congratulations! Type your name as you would like to see it on the
         * Oregon Top Ten list." */
        draw_text(S(0x2b3), 0, 0);
        read_field(0, 0, S(0x30b) /* "A-Za-z '.-" */, name);
        /* "Would you like to make any changes? " (cs:0x317) - edit loop */
        read_input(&g_input_buf);
        if (streq(&g_input_buf, S(0xee9) /* "N" */))
            topten_insert(name, score);                 /* add to HISCORES.REC */
    } else {
        /* "You have accumulated <N> points.  This is not enough to qualify for
         * the Oregon Top Ten." */
        far_sprintf(/* cs:0x342, score */);
        draw_text(/* assembled message */ 0, 0, 0);
        press_any_key();
    }
}                                                       /* ~0x0d96 retf */

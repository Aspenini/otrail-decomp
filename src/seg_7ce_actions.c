/*
 * seg_7ce_actions.c - player travel-menu actions (segment 0x07ce).
 *
 * Handlers dispatched from travel_turn (see seg032_trail.c):
 *   change_pace      @ 0x07ce:0x24bf  (this file)
 *   change_rations   @ 0x07ce:0x27cf  (this file)
 *   stop_to_rest     @ 0x07ce:0x2950
 *   attempt_trade    @ 0x07ce:0x11c3
 *   talk_to_people   @ 0x07ce:0x17eb
 *   buy_at_fort      @ 0x07ce:0x1d3c
 *
 * Address-annotated reconstruction of Borland Turbo C output; not yet
 * compile-verified against the binary.
 */

#include <stdint.h>

extern uint8_t  g_quit_flag;   /* 0x1520 */
extern uint8_t  g_input_buf;   /* 0x141a */
extern uint8_t  g_died;        /* 0x1586: a death ended the game                 */
extern uint8_t  g_pace;        /* 0x15e8: 0=steady, 1=strenuous, 2=grueling     */
extern uint8_t  g_rations;     /* 0x15e9: 0=filling, 1=meager, 2=bare bones      */
extern uint16_t g_684;         /* 0x0684: prompt far-ptr target                  */

/* Text / IO helpers. */
extern void draw_text(const char far *s, int x, int y);   /* 0x1049:0x1855 */
extern void draw_text_line(const char far *s, int x, int y); /* 0x1049:0x1b3c */
extern void press_any_key(void);                          /* 0x1049:0x15a0 */
extern void draw_panel(int x, int y);                     /* 0x1049:0x1df2 */
extern void far_sprintf(/* ... */);                       /* 0x20a4:0x0634 */
extern void far_print(const char far *s);                 /* 0x20a4:0x06c1 */
extern void read_field(int a, int b, const char far *cs, void far *dst); /* 0x1049:0x0d95 */
extern int  parse_choice(const char far *s);              /* 0x1049:0x180b */
extern void fmt_pace(void);                               /* 0x1049:0x2dfd */
extern void fmt_rations(void);                            /* 0x1049:0x2e2a */
extern void gfx_screen_init_1ceb_0b71(void);              /* 0x1ceb:0x0b71 */
extern void gfx_box_1ceb_16a1(int x1, int y1, int x2, int y2); /* 0x1ceb:0x16a1 */
extern void gfx_1ceb_17db(int a);                         /* 0x1ceb:0x17db */
extern void draw_into_box_1049_1938(/* ... */);           /* 0x1049:0x1938 */
extern void show_input_prompt_2042_0215(int a, int b);    /* 0x2042:0x0215 */
extern void advance_day_2042_029e(void);                  /* 0x2042:0x029e: advance the calendar one day */
extern void trade_setup_1049_30c8(void);                   /* 0x1049:0x30c8 */
extern void image_show_14c6_0321(const char far *n, void far *h); /* 0x14c6:0x0321 */
extern void image_blit_14c6_03ea(int x, void far *h);     /* 0x14c6:0x03ea */
extern void image_free_14c6_043c(int lo, int hi);         /* 0x14c6:0x043c */
extern void redraw_supplies_1049_37d5(void);              /* 0x1049:0x37d5 */
extern int  rand_1049_008c(int n);                        /* 0x1049:0x008c */
extern uint8_t g_must_trade;   /* 0x177c: a missing part blocks travel           */
extern void clear_panel_1049_1e64(void);                  /* 0x1049:0x1e64 */
extern void daily_update_1049_3be4(void);                 /* 0x1049:0x3be4: recompute party/supply state */
extern void finalize_1049_1ee3(void);                     /* 0x1049:0x1ee3 */
extern void fill_rect(int x1, int y1, int x2, int y2);    /* 0x1049:0x155e */
extern void draw_status_1049_2c1e(void);                  /* 0x1049:0x2c1e */

#define S(off) ((const char far *)(off))

/* ---------------------------------------------------------- 0x07ce:0x24bf
 * Change the travel pace. Option 4 explains what each pace means and returns to
 * the screen; Enter (empty) or Esc leaves. The screen redisplays with the new
 * value after each change.
 */
void change_pace(void)
{
    int sel;

    for (;;) {                                              /* loop top 0x24CE */
        gfx_screen_init_1ceb_0b71();
        draw_panel(/* g_69c, g_69e */ 0, 0);                /* 0x24DB */
        draw_text_line(S(0x21de) /* "Change pace" */, 0, 0);/* 0x24ED */
        far_sprintf(/* "(currently " */);                   /* 0x24FD */
        fmt_pace();                                         /* 0x2508 */
        far_print(S(0x21f7) /* ")" */);                     /* 0x2517 */
        /* "The pace at which you travel can change. Your choices are:
         *  1. a steady pace / 2. a strenuous pace / 3. a grueling pace / 4. " */
        draw_text(S(0x21fa), 0, 0);                         /* 0x2536 */
        draw_text(S(0x227b) /* "find out what these different paces mean" */, 0, 0); /* 0x2548 */
        draw_text((const char far *)&g_684, 0, 0);          /* 0x255A: prompt */

        read_field(0, 1, S(0x22a5) /* "1-4" */, &g_input_buf);  /* 0x257A */
        gfx_screen_init_1ceb_0b71();                        /* 0x257F */
        if (g_quit_flag) return;                            /* 0x2584 */
        if (g_input_buf == 0) return;                       /* 0x258E: Enter = done */

        sel = parse_choice(&g_input_buf);                   /* 0x259D */
        switch (sel) {
        case 1: g_pace = 0; break;                          /* 0x25AD: steady    */
        case 2: g_pace = 1; break;                          /* 0x25BA: strenuous */
        case 3: g_pace = 2; break;                          /* 0x25C7: grueling  */
        case 4:                                             /* explain the paces */
            gfx_box_1ceb_16a1(0, 0, 0, 0);
            /* "steady - You travel about 8 hours a day, taking frequent rests..." */
            draw_text(S(0x22a9), 0, 0);                     /* 0x260C */
            /* "strenuous - You travel about 12 hours a day..." */
            draw_text(S(0x230f), 0, 0);                     /* 0x2648 */
            /* "grueling - ..." */
            draw_text(S(0x23cd), 0, 0);                     /* 0x2684 */
            press_any_key();                                /* 0x268E */
            gfx_screen_init_1ceb_0b71();                    /* 0x2693 */
            if (g_quit_flag) return;                        /* 0x2698 */
            break;
        default: break;
        }
    }                                                       /* jmp 0x24CE (0x26A7) */
}                                                           /* 0x26AD retf */

/* ---------------------------------------------------------- 0x07ce:0x27cf
 * Change daily food rations. Enter (empty) or Esc leaves; the screen redisplays
 * with the new value after each change.
 */
void change_rations(void)
{
    int sel;

    for (;;) {                                              /* loop top 0x27DE */
        gfx_screen_init_1ceb_0b71();
        draw_panel(0, 0);                                   /* 0x27EB */
        draw_text_line(S(0x26ae) /* "Change food rations" */, 0, 0); /* 0x27FD */
        far_sprintf(/* "(currently " */);                   /* 0x280D */
        fmt_rations();                                      /* 0x2818 */
        far_print(S(0x26cf) /* ")" */);                     /* 0x2827 */
        /* "The amount of food the people in your party eat each day can change.
         * These amounts are:
         *  1. filling - meals are large and generous.
         *  2. meager  - meals are small, but adequate.
         *  3. bare bones - meals are very small; everyone stays hungry." */
        draw_text(S(0x26d2), 0, 0);                         /* 0x2846 */
        draw_text(S(0x2732), 0, 0);                         /* 0x285D */
        draw_text(S(0x275b), 0, 0);                         /* 0x286F */
        draw_text(S(0x2761), 0, 0);                         /* 0x2880 */
        draw_text(S(0x278a), 0, 0);                         /* 0x2892 */
        draw_text(S(0x2790), 0, 0);                         /* 0x28A3 */
        draw_text((const char far *)&g_684, 0, 0);          /* 0x28B5: prompt */

        read_field(0, 1, S(0x27cb) /* "1-3" */, &g_input_buf);  /* 0x28D5 */
        gfx_screen_init_1ceb_0b71();                        /* 0x28DA */
        if (g_quit_flag || g_input_buf == 0) return;        /* 0x28DF..0x28ED */

        sel = parse_choice(&g_input_buf);                   /* 0x28F4 */
        switch (sel) {
        case 1: g_rations = 0; break;                       /* 0x2904: filling    */
        case 2: g_rations = 1; break;                       /* 0x2910: meager     */
        case 3: g_rations = 2; break;                       /* 0x291C: bare bones */
        default: break;
        }
    }
}                                                           /* 0x2924 retf */

/* ---------------------------------------------------------- 0x07ce:0x2950
 * Rest for a chosen number of days (0-9). Each day advances the calendar and
 * recomputes party/supply state, redrawing the status; resting lets the sick
 * recover but still consumes food. Stops early if a death occurs.
 */
void stop_to_rest(void)
{
    int days;   /* parsed day count */
    int i;      /* [bp-0x2] day counter */

    clear_panel_1049_1e64();                                /* 0x296F */
    draw_text(S(0x2925) /* "How many days would you like to rest? " */, 0, 0); /* 0x2981 */
    show_input_prompt_2042_0215(0, 0);                      /* 0x298C */
    read_field(0, 0, S(0x294c) /* "0-9" */, &g_input_buf);  /* 0x29A1 */
    days = parse_choice(&g_input_buf);                      /* 0x29AB */

    for (i = 0; i < days; i++) {                            /* 0x29B8..0x2A0B */
        if (g_died)                                         /* 0x29C0 */
            break;
        advance_day_2042_029e();                            /* 0x29CB */
        daily_update_1049_3be4();                           /* 0x29D3 */
        fill_rect(0, 0, 0, 0);                              /* 0x29E7: clear status */
        draw_status_1049_2c1e();                            /* 0x29F2 */
        /* (status line redrawn 0x29FF) */
    }

    finalize_1049_1ee3();                                   /* 0x2A0D */
}                                                           /* 0x2A15 retf */

/* ---------------------------------------------------------- 0x07ce:0x11c3
 * Attempt to trade with another emigrant. Shows the supplies screen, then
 * randomly either finds no trade ("No one wants to trade with you today.") or
 * generates an offer ("You meet another emigrant who wants <X>. ... will trade
 * you <Y>. Are you willing to trade?"). Accepting swaps the goods and may
 * resolve a g_must_trade block (e.g. trading for the part you needed). Either
 * way, the attempt costs a day.
 *
 * The random item/quantity selection and the swap arithmetic are summarised
 * rather than traced statement-by-statement.
 */
void attempt_trade(void)
{
    int want_qty;   /* [bp-0x3c] item they want / quantity */
    int give_qty;   /* [bp-0x3e] item they offer / quantity */

    trade_setup_1049_30c8();                                 /* 0x11D7 */
    gfx_screen_init_1ceb_0b71();                            /* 0x11DC */
    image_show_14c6_0321(S(0x110f) /* "supplies.pcc" */, 0);/* 0x11EC */
    image_blit_14c6_03ea(0xdd0, 0);                         /* 0x1206: supply icons */
    image_blit_14c6_03ea(0xdc8, 0);                         /* 0x1220 */
    image_blit_14c6_03ea(0xdb0, 0);                         /* 0x123A */
    image_blit_14c6_03ea(0xdc0, 0);                         /* 0x1254 */
    image_free_14c6_043c(0, 0);                             /* 0x1261 */
    redraw_supplies_1049_37d5();                            /* 0x126A */
    daily_update_1049_3be4();                               /* 0x1272 */
    if (g_died || g_quit_flag) return;                      /* 0x1277..0x1285 */

    /* Pick a random item the emigrant wants and one they offer. */
    want_qty = rand_1049_008c(/* item count */ 0);          /* 0x128C */
    give_qty = rand_1049_008c(/* item count */ 0);          /* 0x1298 */

    /* ... if no valid match, "No one wants to trade with you today." (cs:0x111c)
     * else present the offer (cs:0x113b/0x1187/0x119a) and read Y/N ... */

    /* On accept: apply the swap (long arithmetic on the supply records). If the
     * traded-for item clears a blocking shortage, drop g_must_trade. */
    if (g_must_trade) {                                     /* 0x1788 */
        /* e.g. traded for an "ox" (cs:0x11c0) -> can travel again */
        if (/* the needed item was obtained */ 0)
            g_must_trade = 0;
    }

    redraw_supplies_1049_37d5();                            /* 0x17BD */
    advance_day_2042_029e();                                /* 0x17C6: trading costs a day */
}                                                           /* 0x17CB retf */

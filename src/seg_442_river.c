/*
 * seg_442_river.c - river-crossing module (segment 0x0442).
 *
 *   cross_river @ 0x0442:0x18bb  (far)
 *   ford_river        0x02be    caulk_and_float   0x0695
 *   take_ferry        0x09ef    hire_indian       0x0e2c
 *   wait_conditions   0x1090    get_more_info     0x126f
 *   river_done        0x1321    (crossing animation + arrival text)
 *
 * Reached from continue_travel (0x0032:0x361a) when the next location is a
 * river. Shows the river's width/depth and weather, offers the crossing
 * choices, and dispatches to the chosen method; loops until the party crosses.
 *
 * The visible option numbers shift with what's available at this river:
 *   1. attempt to ford the river
 *   2. caulk the wagon and float it across
 *   3. take a ferry across        (only if a ferry operates here)
 *   3. hire an Indian to help      (only if offered here)   [1990 wording]
 *   N. wait to see if conditions improve
 *   N. get more information
 *
 * Address-annotated structural reconstruction of Borland Turbo C output; the
 * dynamic option numbering is summarised. Not yet compile-verified.
 */

#include <stdint.h>

extern uint8_t  g_quit_flag;   /* 0x1520 */

/* Text / IO helpers. */
extern void draw_text(const char far *s, int x, int y);   /* 0x1049:0x1855 */
extern void clear_text_area(void);                        /* 0x1049:0x1e43 */
extern void far_print(const char far *s);                 /* 0x20a4:0x06c1 */
extern void far_sprintf(/* ... */);                       /* 0x20a4:0x0634 */
extern void read_field(int a, int b, const char far *cs, void far *dst); /* 0x1049:0x0d95 */
extern int  parse_choice(const char far *s);              /* 0x1049:0x180b */
extern void fmt_weather(void);                            /* 0x1049:0x2d8b */
extern void gfx_screen_init_1ceb_0b71(void);              /* 0x1ceb:0x0b71 */

/* Crossing-method handlers (segment 0x0442, near). */
static void ford_river_442_02be(void);         /* 0x0442:0x02be (below) */
static void caulk_and_float_442_0695(void);    /* 0x0442:0x0695 (below) */
static void take_ferry_442_09ef(void);         /* 0x0442:0x09ef (below) */
static void hire_indian_442_0e2c(void);        /* 0x0442:0x0e2c: pay a Shoshoni guide in clothing */
static void wait_conditions_442_1090(void);    /* 0x0442:0x1090 */
static void get_more_info_442_126f(void);      /* 0x0442:0x126f */
static void river_done_442_1321(void);         /* 0x0442:0x1321 */

extern uint8_t  g_input_buf;   /* 0x141a */
extern uint8_t  g_clothing;    /* 0x15cc: sets of clothing in inventory (store item) */

#define S(off) ((const char far *)(off))

/* ---------------------------------------------------------- 0x0442:0x18bb */
void cross_river(void)
{
    int  choice;
    int  ferry_avail;    /* [bp-0x22a] */
    int  indian_avail;   /* [bp-0x22b] */
    int  crossed;        /* [bp-0x22c] */

    gfx_screen_init_1ceb_0b71();                            /* 0x18CA */
    /* draw the river panel + landmark name (0x18D7..0x190A) */

    /* "You must cross the river ... currently <N> feet across, and <M> feet
     * deep in the middle." */
    far_sprintf(/* msg, cs:0x174d, width */);               /* 0x196C */
    far_print(S(0x17a3) /* " feet across, and " */);        /* 0x1980 */
    far_print(S(0x17b6) /* " feet deep in the middle." */); /* 0x1994 */
    draw_text(/* msg */ 0, 0, 0);
    /* press a key (0x19CF); abort on quit */
    if (g_quit_flag) return;                                /* 0x19D4 */

    while (!crossed) {                                      /* loop top 0x19F2 */
        clear_text_area();                                  /* 0x19FC */

        /* status: "Weather: <w> / River width: <n> feet / River depth: <d> feet" */
        far_sprintf(/* cs:0x17d0 "Weather: " */);           /* 0x1A0F */
        fmt_weather();                                      /* 0x1A1F */
        far_print(S(0x17da) /* "/River width: " */);        /* 0x1A29 */
        far_print(S(0x17e9) /* " feet/" */);                /* 0x1A3D */
        far_print(S(0x17f0) /* "River depth: " */);         /* 0x1A47 */
        far_print(S(0x17e9) /* " feet/" */);                /* 0x1A5B */

        draw_text(S(0x17fe) /* "You may:\\" */, 0, 0);       /* 0x1A75 */
        draw_text(S(0x1808) /* "1. attempt to ford the river\\"
                              "2. caulk the wagon and float it across" */, 0, 0); /* 0x1A8A */
        if (ferry_avail)                                    /* 0x1A94 */
            draw_text(S(0x184c) /* "3. take a ferry across" */, 0, 0);   /* 0x1AA3 */
        if (indian_avail)                                   /* 0x1AAD */
            draw_text(S(0x1863) /* "3. hire an Indian to help" */, 0, 0);/* 0x1ABC */
        /* next free number: "wait to see if conditions improve" */
        far_print(S(0x187d));                               /* 0x1AF8 */
        /* and: "get more information" */
        far_print(S(0x18a1));                               /* 0x1B29 */

        read_field(1, 1, S(0x18b8) /* "1-N" */, &g_input_buf);  /* 0x1B85 */
        clear_text_area();                                  /* 0x1B8A */
        if (g_quit_flag) return;                            /* 0x1B8F */

        choice = parse_choice(&g_input_buf);                /* 0x1B9E */
        switch (choice) {
        case 1: ford_river_442_02be();      break;          /* 0x1BB2 */
        case 2: caulk_and_float_442_0695(); break;          /* 0x1BC0 */
        case 3:                                             /* 0x1BC8 */
            if (ferry_avail)        take_ferry_442_09ef();
            else if (indian_avail)  hire_indian_442_0e2c();
            else                    wait_conditions_442_1090();
            break;
        case 4:                                             /* 0x1BF7 */
            if (ferry_avail || indian_avail) wait_conditions_442_1090();
            else                             get_more_info_442_126f();
            break;
        case 5: get_more_info_442_126f();   break;          /* 0x1C19 */
        default: break;
        }
        if (g_quit_flag) return;                            /* 0x1C1C */
    }                                                       /* jmp 0x19F2 */

    river_done_442_1321();                                  /* 0x1C29 */
}

/* ---------------------------------------------------------- 0x0442:0x02be
 * Attempt to ford the river (wade across). Rolls a random outcome whose
 * severity scales with the river's depth/conditions, and applies the result.
 * The exact roll and loss arithmetic (long math + rand) are summarised; the
 * outcome ladder and its messages are the recovered behaviour.
 *
 * Outcome ladder (cs:strings in this segment):
 *   0  "You made the crossing successfully."
 *   1  muddy: "You become stuck in the mud.  Lose 1 day." (advances a day)
 *            or "It was a muddy crossing, but you did not get stuck."
 *   2  rough: "It was a rough crossing, but you did not overturn."
 *            or "The wagon tipped over but you did not lose anything."
 *   3+ "The wagon tipped over ...  You lose:" / "Your supplies got wet.
 *      Lose 1 day."
 *   deep "The river is too deep to ford.  You lose:" -> lose supplies, oxen,
 *      and/or party members (can be fatal).
 */
extern int  rand_outcome(int depth);          /* random crossing result by depth */
extern void daily_update(void);               /* 0x1049:0x3be4: advance a day */
extern void lose_supplies(int which);         /* apply a supply/party loss */

static void ford_river_442_02be(void)
{
    int outcome = rand_outcome(/* river depth */ 0);   /* 0x02xx: roll */

    switch (outcome) {
    case 0:
        far_print(S(0x178) /* "You made the crossing successfully." */);   /* 0x02DF */
        break;
    case 1:
        if (/* sub-roll: got stuck */ 0) {
            far_print(S(0x19c) /* "You become stuck in the mud.  Lose 1 day." */); /* 0x036D */
            daily_update();                              /* 0x0387 */
        } else {
            far_print(S(0x1c6) /* "It was a muddy crossing, but you did not get stuck." */); /* 0x038E */
        }
        break;
    case 2:
        if (/* sub-roll: tipped */ 0) {
            far_print(S(0x22d) /* "The wagon tipped over but you did not lose anything." */);
        } else {
            far_print(S(0x1fa) /* "It was a rough crossing, but you did not overturn." */); /* 0x03B3 */
        }
        break;
    default:
        /* Deep / disastrous crossing: lose supplies (and possibly oxen or
         * party members), wet supplies, lost days. */
        far_print(S(0x294) /* "The river is too deep to ford.  You lose:" */);
        far_print(S(0x270) /* "Your supplies got wet.  Lose 1 day." */);
        lose_supplies(/* category from the roll */ 0);
        daily_update();
        break;
    }
}

extern void press_any_key(void);              /* 0x1049:0x15a0 */
extern void draw_text_line(const char far *s, int x, int y); /* 0x1049:0x1b3c */
extern int  streq(const char far *a, const char far *b);     /* 0x20a4:0x0724 */
extern long g_cash;                           /* 0x15d2: player money */

/* ---------------------------------------------------------- 0x0442:0x0695
 * Caulk the wagon and float it across. Needs enough depth; floating risks the
 * wagon tipping over and losing supplies. Takes a day.
 */
static void caulk_and_float_442_0695(void)
{
    if (/* river too shallow to float */ 0) {            /* 0x06B8 */
        clear_text_area();
        draw_text_line(S(0x60c) /* "The river is too shallow" */, 0, 0);  /* 0x06D4 */
        draw_text_line(S(0x625) /* "to float across." */, 0, 0);          /* 0x06E6 */
        press_any_key();
        return;                                          /* 0x06F0 */
    }
    daily_update();                                      /* 0x06F6: crossing costs a day */
    /* outcome: "You had no trouble floating the wagon across." (cs:0x636) or
     * "The wagon tipped over while floating.  You lose:" -> lost supplies. */
    far_print(S(0x636));                                 /* 0x070D */
}

/* ---------------------------------------------------------- 0x0442:0x09ef
 * Take a ferry. Costs $5.00 and a wait of several days; needs deep enough water.
 * Usually safe, but the ferry can break loose from its moorings and cost you.
 */
static void take_ferry_442_09ef(void)
{
    if (/* river too shallow for the ferry */ 0) {       /* 0x0A12 */
        draw_text(S(0x868) /* "The ferry is not operating today because the
                              river is too shallow." */, 0, 0);           /* 0x0A33 */
        press_any_key();
        return;                                          /* 0x0A47 */
    }
    /* "The ferry operator says that he will charge you $5.00 and that you will
     * have to wait <N> days.  Are you willing to do this?" */
    read_field(0, 0, /* "Y"/"N" */ 0, &g_input_buf);     /* 0x0A6B */
    if (!streq(&g_input_buf, S(/* "Y" */ 0)))
        return;                                          /* declined */

    if (g_cash < 500 /* $5.00 */) {
        far_print(S(0x929) /* "You do not have enough money to pay for the ferry." */);
        return;
    }
    g_cash -= 500;
    /* wait the required days, then cross */
    /* outcome: "The ferry got your party and wagon safely across." (cs:0x95e)
     * or "The ferry broke loose from moorings.  You lose:" (cs:0x990)
     * or "Some trouble in crossing but nothing was lost." */
    far_print(S(0x95e));
}

/* The near crossing handlers actually take (int mode, struct river far *) — the
 * caller pushes a `mode` byte and a far pointer to cross_river's on-stack river
 * record. The no-arg prototypes above are the simplified view used by the
 * cross_river reconstruction; hire_indian below performs the real crossing by
 * re-dispatching into ford / float with mode 5. */
extern void daily_update(void);               /* 0x1049:0x3be4: advance a day */

/* Small Borland C-runtime / library helpers used by the handlers below. */
extern int   rand_n(int n);                   /* 0x1049:0x008c: random 0..n-1 */
extern void  draw_para(const char far *s, int x, int w, int y); /* 0x1049:0x1938: word-wrapped paragraph */
extern void  draw_text_line(const char far *s, int x, int y);   /* 0x1049:0x1b3c */
extern void  beep(int hz);                    /* 0x2042:0x029e: tone/delay */
extern int   streq(const char far *a, const char far *b);       /* 0x20a4:0x0724 */

/* ---------------------------------------------------------- 0x0442:0x0e2c
 * Hire an Indian (a Shoshoni guide) to take the wagon across in exchange for
 * sets of clothing. Negotiates a price, checks you can pay, and on acceptance
 * deducts the clothing and crosses the river by fording or floating.
 *
 * The depth test that selects ford-vs-float (the long compare at 0x20a4:0xb10
 * with the river record fields) and the sprintf/draw argument plumbing are
 * summarised; the clothing economy and the messages are the recovered behaviour.
 */
static void hire_indian_442_0e2c(void)
{
    int needed = rand_n(2) + 2;          /* 0x0E3B..0x0E47: 2 or 3 sets of clothing */

    /* "A Shoshoni guide says that he will take your wagon across the river in
     * exchange for <N> sets of clothing.\" */
    far_sprintf(/* cs:0xd36 + needed, cs:0xd8b */);          /* 0x0E76 */
    far_print(&g_input_buf);                                 /* 0x0E80 */
    far_print(S(0xd8b) /* " sets of clothing.\\" */);        /* 0x0E8A */
    draw_para(/* msg */ 0, 0, 0, 0);                         /* 0x0E8F */
    draw_text(/* msg */ 0, 0, 0x4a);                         /* 0x0EBF */

    if (g_clothing < needed) {                               /* 0x0EC4..0x0ECC */
        /* "You don't have <N> sets of clothing." */
        far_sprintf(/* cs:0xd9f + needed */);               /* 0x0EEB */
        far_print(&g_input_buf);                             /* 0x0EF5 */
        far_print(S(0xdaf) /* " sets of clothing." */);      /* 0x0EFF */
        draw_para(/* msg */ 0, 0, 0, 0);                     /* 0x0F04 */
        draw_text(/* msg */ 0, 0, 0);                        /* 0x0F09 */
        press_any_key();                                     /* 0x0F0E */
        return;                                              /* jmp 0x1067 */
    }

    /* "Will you accept this\offer? " — read a single key, default 'N'. */
    draw_text(S(0xdc2), 0, 0);                               /* 0x0F23 */
    read_field(0x0d, 0x11, 0, &g_input_buf);                 /* 0x0F2E..0x0F38 */
    if (g_quit_flag) return;                                 /* 0x0F3D */
    if (streq(&g_input_buf, S(0xddf) /* "N" */))             /* 0x0F51 */
        return;                                              /* declined */

    g_clothing -= needed;                                    /* 0x0F5B..0x0F63 */

    /* "The Shoshoni guide will help you " + "ford the river." (cs:0xe03) or
     * "float your wagon across." (cs:0xe13), chosen by river depth. */
    far_print(S(0xde1));                                     /* build msg @ 0x0F66 */
    if (/* river shallow enough to ford */ 1)                /* 0x0F8C..0x0F99 */
        far_print(S(0xe03) /* "ford the river." */);         /* 0x0FB4 */
    else
        far_print(S(0xe13) /* "float your wagon across." */);/* 0x0FE6 */
    draw_para(/* msg */ 0, 0, 0, 0);                         /* 0x101D */
    draw_text(/* msg */ 0, 0, 0);                            /* 0x1022 */
    press_any_key();                                         /* 0x1027 */
    if (g_quit_flag) return;                                 /* 0x102C */

    /* Perform the crossing the guide promised (mode 5 = guided, lower risk). */
    if (/* river shallow enough to ford */ 1)                /* 0x1044..0x1051 */
        ford_river_442_02be();      /* call 0x02be(5, river)  0x1059 */
    else
        caulk_and_float_442_0695(); /* call 0x0695(5, river)  0x1064 */
}

/* ---------------------------------------------------------- 0x0442:0x1090
 * Wait a day to see if the river conditions improve. Camps a day, advances the
 * calendar, re-rolls the weather/river state, and redraws the river panel; the
 * crossing menu then loops again with the new conditions.
 */
static void wait_conditions_442_1090(void)
{
    draw_text_line(S(0x106d) /* "You camp near the river for a day." */,
                   0x64, 0xa0);                              /* 0x10AC */
    press_any_key();                                         /* 0x10B1 */
    daily_update();                                          /* 0x10B9: advance a day */

    /* re-roll weather/river conditions and rebuild the status line */
    /* (0x1049:0x155e then 0x1049:0x2c1e into a scratch buffer)   0x10CD..0x10D8 */
    draw_text_line(0 /* cleared line */, 0x09, 0xa0);        /* 0x10E5 */

    river_display_442_0000();   /* recompute + show width/depth   0x10ED */
}

/* ---------------------------------------------------------- 0x0442:0x126f
 * Explain the three crossing methods, one screen at a time (press a key between
 * them; quitting bails out early).
 */
static void get_more_info_442_126f(void)
{
    clear_text_area();                                       /* 0x127E */
    /* "To ford a river means to pull your wagon across a shallow part of the
     * river, with the oxen still attached." */
    draw_para(S(0x10f6), 0x50, 0xe4, 0);                     /* 0x129A */
    draw_text(0, 0, 0);                                      /* 0x129F */
    press_any_key();                                         /* 0x12A4 */
    if (g_quit_flag) return;                                 /* 0x12A9 */

    clear_text_area();                                       /* 0x12B2 */
    /* "To caulk the wagon means to seal it so that no water can get in.  The
     * wagon can then be floated across like a boat." */
    draw_para(S(0x1161), 0x50, 0xe4, 0);                     /* 0x12CE */
    draw_text(0, 0, 0);                                      /* 0x12D3 */
    press_any_key();                                         /* 0x12D8 */
    if (g_quit_flag) return;                                 /* 0x12DD */

    clear_text_area();                                       /* 0x12E6 */
    /* "To use a ferry means to put your wagon on top of a flat boat that belongs
     * to someone else.  The owner of the ferry will take your wagon across the
     * river." */
    draw_para(S(0x11d5), 0x4b, 0xe4, 0);                     /* 0x1302 */
    draw_text(0, 0, 0);                                      /* 0x1307 */
    press_any_key();                                         /* 0x130C */
}

/* ---------------------------------------------------------- 0x0442:0x1321
 * Play the river-crossing animation, then show the arrival/result text. Loads
 * the raft sprite (float.pcc), scrolls the wagon and party across the river
 * using the 0x1ceb drawing primitives (filled rects for the banks/water and
 * sprite blits for the wagon) with per-frame delays and a tone, branching on
 * the crossing method/outcome stored in the river record (field -0x22d: 0=ford,
 * 1=float, 2=ferry). The frame geometry/arithmetic is summarised; this is the
 * recovered structure and asset use.
 */
extern void  gfx_fill_rect(int x0, int y0, int x1, int y1, int color); /* 0x1ceb:0x16e9 */
extern void  gfx_blit(int x, int y, int w, int h);          /* 0x1ceb:0x16a1 */
extern void  gfx_set_color(int a, int b);                   /* 0x1ceb:0x0cda */
extern void  gfx_save_strip(int a);                         /* 0x1ceb:0x17db */
extern void  gfx_restore_strip(int x, int y, int a);        /* 0x1ceb:0x19fd */
extern void  gfx_box(int x0, int y0, int x1, int y1, int t); /* 0x1ceb:0x1c5f */
extern void  image_show(void far *dst, const char far *name); /* 0x14c6:0x0321 */
extern void  image_blit(void far *buf, int x, int y, int w, int h); /* 0x14c6:0x03ea */
extern void  image_free(void far *buf);                     /* 0x14c6:0x043c */
extern void  river_display_442_0000(void);                  /* 0x0442:0x0000 */

static void river_done_442_1321(void)
{
    int pal_a, pal_b;

    if (g_179a) { pal_a = 0x85; pal_b = 0xc7; }              /* 0x1330..0x1341 */
    else        { pal_a = 0x01; pal_b = 0x00; }              /* 0x1343..0x134A */

    image_show(/* sprite buf */ 0, S(0x1317) /* "float.pcc" */); /* 0x1357 */
    gfx_screen_init_1ceb_0b71();                             /* 0x135C */
    gfx_box(0x19, 0x96, 0x50, 0xf0, 3);                      /* 0x1385: frame the scene */

    /* Animate the wagon/raft crossing left-to-right: ~0x33 frames stepping the
     * sprite x, each with a save/blit/restore and a short tone (0x2042:0x29e).
     * The party-sprite frame (field -0x22d) selects ford/float/ferry artwork. */
    /* ... animation loop 0x13ED..0x1660 (summarised) ... */

    /* draw the arrival/result message (built into the river record buffer) and
     * sound the arrival tone. */
    draw_para(/* msg */ 0, 0xc8, 0, 0);                      /* 0x167E */
    draw_text(/* msg */ 0, 0, 0);                            /* 0x16F7 */
    press_any_key();                                         /* 0x1720 */
    image_free(/* sprite buf */ 0);                          /* 0x1730: release sprite buf */
    if (!g_quit_flag) beep(0x384);                           /* 0x173E */
}

/*
 * seg_442_river.c - river-crossing module (segment 0x0442).
 *
 *   cross_river @ 0x0442:0x18bb  (far)
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
extern void hire_indian_442_0e2c(void);        /* 0x0442:0x0e2c: pay a Shoshoni guide in clothing */
extern void wait_conditions_442_1090(void);    /* 0x0442:0x1090 */
extern void get_more_info_442_126f(void);      /* 0x0442:0x126f */
extern void river_done_442_1321(void);         /* 0x0442:0x1321 */

extern uint8_t  g_input_buf;   /* 0x141a */

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

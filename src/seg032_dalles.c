/*
 * seg032_dalles.c - The Dalles toll decision and the no-oxen travel blocker
 *                   (segment 0x0032, the travel engine).
 *
 *   columbia_dalles @ 0x0032:0x2ff5  (near) - float the Columbia vs the Barlow toll
 *   check_oxen      @ 0x0032:0x356a  (near) - block travel when out of oxen
 *
 * Note: 0x356a was labelled "generic landmark arrival handler" in the function
 * map; lifting it shows it is actually the no-oxen check - it compares the ox
 * count (g_oxen, 0x15c4) against zero and, if depleted, sets the must-trade
 * flag and shows "You are unable to continue your journey.  You have no oxen to
 * pull your wagon." The symbol table is corrected accordingly.
 *
 * Address-annotated structural reconstruction of Borland Turbo C output; the
 * decision flow and the cash/oxen reads are verified against the binary, the
 * exact Barlow toll formula (long-math from 0x15c4 and constants 0x80/0x83/
 * 0x2000) is left noted rather than guessed. Not yet compile-verified.
 */

#include <stdint.h>

/* ------------------------------------------------------------------ globals */
extern uint8_t  g_quit_flag;        /* 0x1520: Esc/quit requested              */
extern uint8_t  g_must_trade;       /* 0x177c: a missing item blocks travel    */
extern char     g_needed_item[0xa]; /* 0x177e: name of the item to trade for   */
extern long     g_oxen;             /* 0x15c4: ox count (checked by the blocker)*/
extern long     g_cash;             /* 0x15d2: money                           */
extern uint8_t  g_input_buf;        /* 0x141a: raw keyboard input (counted)    */
extern uint16_t g_cursor_row;       /* 0x157a: text layout origin              */
extern uint16_t g_cursor_col;       /* 0x157c                                  */
extern uint16_t g_panel_y;          /* 0x069e: panel Y for draw_panel          */
extern uint16_t g_margin_x;         /* 0x06a0: left text margin                */

/*
 * Caller scene context (passed by the trail loop, [bp+0x04] -> far ptr). Only
 * the fields this routine touches are modelled; offsets are from the binary.
 */
struct trail_ctx {
    char pad0[0x1c - 0x0b];
    uint8_t float_river;   /* [-0x0b]: 1 = float the Columbia, 0 = Barlow road */
    /* ... -0x08/-0x06/-0x04/-0x02 hold the scene buffer + position ... */
};

/* --------------------------------------------------------------- prototypes */
extern void __stkcheck_20a4_0244(int frame);

extern void gfx_screen_init_1ceb_0b71(void);
extern void draw_panel_1049_1df2(int x, int y);
extern void clear_text_area_1049_1e43(void);
extern void clear_panel_1049_1e64(int x1, int y1, int x2, int y2);
extern void text_1049_1855(void);
extern void echo_field_1049_d95(const char far *prompt, char far *dst);
extern void read_input_var_1049_1bb3(char far *dst);
extern void draw_msg_box_1049_1938(int x, int y, char far *out, int w, const char far *fmt);
extern void press_any_key_1049_15a0(void);
extern void finalize_action_1049_1ee3(void);
extern void format_money_1049_2ccc(char far *out, long money, int flag);

extern void show_input_prompt_2042_0215(int a, int b);

extern void  textlayout_20a4_634(int x, int y, char far *out, const char far *fmt);
extern void  far_print_20a4_6c1(const char far *s);
extern void  strncpy_n_20a4_64e(char far *dst, const char far *src, int n);
extern int   streq_20a4_724(const char far *a, const char far *b);   /* 0 = equal */
extern long  lcmp_20a4_b10(long a, long b);
extern long  lsub_20a4_af4(long a, long b);
extern long  barlow_toll(long oxen);   /* the long-math toll calc, formula TBD */

#define CS(off)  ((const char far *)(off))
#define DS(off)  ((const char far *)(off))

/* ---------------------------------------------------------- 0x0032:0x2ff5
 * The Dalles: "1. float down the Columbia River / 2. take the Barlow Toll
 * Road". Floating sets ctx->float_river (the loop hands off to the rafting
 * animation at 0x31df). The toll road computes a fee, confirms it, and deducts
 * it from g_cash - refusing if the party can't afford it. Esc bails out.
 */
void columbia_dalles(void far *ctxp)
{
    struct trail_ctx far *ctx = *(struct trail_ctx far * far *)((char far *)ctxp + 4);
    char layout[0x100];     /* [bp-0x108] */
    char moneybuf[0x100];   /* [bp-0x208] */
    int  paid = 0;          /* [bp-0x07] */
    long toll;

    do {
        gfx_screen_init_1ceb_0b71();
        draw_panel_1049_1df2(0x14, g_panel_y);
        clear_text_area_1049_1e43();
        textlayout_20a4_634(g_margin_x - 0xa, 0x37, layout, CS(0x2f1f));
            /* "The trail divides here.  You may:
                  1. float down the Columbia River
                  2. take the Barlow Toll Road        1-2" */
        far_print_20a4_6c1(DS(0x684));                  /* "What is your choice? " */
        text_1049_1855();

        show_input_prompt_2042_0215(0x12, 0x19);
        echo_field_1049_d95(CS(0x2f88), (char far *)&g_input_buf);
        if (g_quit_flag)
            return;

        if (streq_20a4_724((const char far *)&g_input_buf, CS(0x2f8c)) == 0) {
            ctx->float_river = 1;                       /* choice 1: float */
            break;
        }
        ctx->float_river = 0;

        /* choice 2: the Barlow toll road - price it, confirm, charge it */
        toll = barlow_toll(g_oxen);                     /* 0x309f..0x30d9 long-math */
        clear_text_area_1049_1e43();
        textlayout_20a4_634(g_margin_x - 0xa, 0x4e, moneybuf, CS(0x2f8e));  /* "You must pay " */
        format_money_1049_2ccc(layout, toll, 0);
        far_print_20a4_6c1(layout);                     /* "$N" */
        far_print_20a4_6c1(CS(0x2f9c));                 /* " to travel the Barlow road.  Are you willing?" */
        text_1049_1855();

        show_input_prompt_2042_0215(0x0d, 0x0a);
        read_input_var_1049_1bb3((char far *)&g_input_buf);
        if (g_quit_flag)
            return;

        if (streq_20a4_724((const char far *)&g_input_buf, CS(0x2fd6)) == 0) {  /* "Y" */
            if (lcmp_20a4_b10(g_cash, toll) < 0) {      /* can't afford it */
                draw_msg_box_1049_1938(g_cursor_row, g_cursor_col, layout, 0,
                                       CS(0x2fd8));      /* "You do not have enough cash." */
                press_any_key_1049_15a0();
                if (g_quit_flag)
                    return;
            } else {
                g_cash = lsub_20a4_af4(g_cash, toll);   /* pay the toll */
                paid = 1;
            }
        }
    } while (!paid && !ctx->float_river);
}

/* ---------------------------------------------------------- 0x0032:0x356a
 * Block travel if the party has no oxen to pull the wagon. Sets the must-trade
 * flag with "an ox" as the needed item and shows the explanatory message.
 */
void check_oxen(void)
{
    char buf[0x100];   /* [bp-0x100] */

    if (g_oxen > 0)                                      /* lcmp [0x15c4] > 0 */
        return;

    g_must_trade = 1;                                   /* 0x177c */
    strncpy_n_20a4_64e(g_needed_item, CS(0x3515), 0xa); /* "an ox" */
    clear_panel_1049_1e64(0x65, 0x11a, 0x3f, 0x24);
    draw_msg_box_1049_1938(g_cursor_row, g_cursor_col, buf, 0xe0, CS(0x351b));
        /* "You are unable to continue your journey.  You have no oxen to pull
            your wagon." */
    text_1049_1855();
    press_any_key_1049_15a0();
    finalize_action_1049_1ee3();
}

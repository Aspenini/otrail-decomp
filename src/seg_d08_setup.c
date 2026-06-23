/*
 * seg_d08_setup.c - new-game setup screens (segment 0x0d08).
 *
 * Called in order by start_new_game (see seg_d08_game_start.c):
 *   choose_profession      @ 0x0d08:0x01f2  (this file)
 *   name_party             @ 0x0d08:0x04d0  (TODO)
 *   choose_departure_month @ 0x0d08:0x0a4f  (TODO)
 *   buy_supplies           @ 0x0d08:0x1e78  (TODO)
 *
 * Address-annotated reconstruction of Borland Turbo C output; not yet
 * compile-verified against the binary.
 */

#include <stdint.h>

extern uint8_t  g_quit_flag;        /* 0x1520 */
extern uint8_t  g_input_buf;        /* 0x141a */
extern uint8_t  g_active_save[];    /* 0x1588: byte 0 = chosen profession        */
extern uint16_t g_choice;           /* 0x1570: scratch for parsed numeric input  */
extern uint16_t g_text_margin_x;    /* 0x06a0 */
extern uint16_t g_text_top_y;       /* 0x1582 */
extern uint16_t g_157a, g_157c;     /* draw_text end cursor                       */
extern uint16_t g_157e, g_1580;     /* menu line position                         */
extern uint16_t g_69c, g_69e;       /* occupation-panel position                  */
extern uint16_t g_684;              /* prompt far-ptr target                      */
extern uint16_t g_15d2, g_15d4, g_15d6;  /* profession-dependent value (long)     */
extern uint8_t  g_departure_month;  /* 0x15c1: 3=March .. 7=July                  */
extern uint16_t g_year;             /* 0x15c2: starting year (1848 = 0x738)       */
extern uint8_t  g_15c0;             /* 0x15c0: set to 1 once departure is chosen  */

/* Text / IO helpers (segment 0x1049). */
extern void draw_text(const char far *s, int x, int y);   /* 0x1049:0x1855 */
extern void draw_text_line(const char far *s, int x, int y); /* 0x1049:0x1b3c */
extern void press_any_key(void);                          /* 0x1049:0x15a0 */
extern void clear_text_area(void);                        /* 0x1049:0x1e43 */
extern void fill_rect(int x1, int y1, int x2, int y2);    /* 0x1049:0x155e */
extern void draw_panel_1049_1df2(int x, int y);           /* 0x1049:0x1df2 */
extern void read_field_1049_0d95(int a, int b, const char far *charset, void far *dst); /* 0x1049:0x0d95 */
extern int  atoi_20a4_104b(const char far *s, int far *end);  /* 0x20a4:0x104b */
extern int  parse_choice_1049_180b(const char far *s);    /* 0x1049:0x180b */
extern void strncpy_n(char far *d, const char far *s, int n); /* 0x20a4:0x064e */
extern void far_sprintf(/* dst, fmt, ... */);             /* 0x20a4:0x0634 */
extern void far_print(const char far *s);                 /* 0x20a4:0x06c1 */
extern int  rand_1049_008c(int n);                        /* 0x1049:0x008c: random 0..n-1 */
extern void image_load_14c6_021a(const char far *name, int a, int b); /* 0x14c6:0x021a */
extern int  read_member_name_d08_032e(int slot);          /* 0x0d08:0x032e (near) */
extern void gfx_screen_init_1ceb_0b71(void);              /* 0x1ceb:0x0b71 */

/* Party names: 5 records of PARTY_NAME_LEN bytes starting at g_active_save+1
 * (leader is record 0). DEFAULT_NAMES is a pool of 6-byte names at cs:0x5fa. */
#define PARTY_NAME_LEN 0xB
extern char g_party_names[5][PARTY_NAME_LEN];   /* 0x1589 */

#define S(off) ((const char far *)(off))

/* ---------------------------------------------------------- 0x0d08:0x01f2
 * Pick an occupation. 1=banker, 2=carpenter, 3=farmer; 4 explains the money /
 * score trade-off and re-prompts. Loops until a valid 1-3 choice (or quit).
 */
void choose_profession(int arg)
{
    int sel;   /* [bp-0x2] */

    draw_panel_1049_1df2(g_69c, g_69e);             /* 0x0208: occupation panel */

    do {
        /* occupation list + "Find out the differences between these choices" */
        draw_text(S(0x000), g_text_margin_x, g_text_top_y + 0x12);  /* 0x021D */
        draw_text(S(0x099), g_157e, g_1580);                        /* 0x022F */
        draw_text((const char far *)&g_684, g_text_margin_x, g_157c); /* 0x0241 */

        read_field_1049_0d95(1, 1, S(0x0c9) /* "1-4" */, &g_input_buf);  /* 0x0256 */
        clear_text_area();                          /* 0x025B */
        if (g_quit_flag)                            /* 0x0260 */
            return;

        sel = atoi_20a4_104b(&g_input_buf, 0);      /* 0x0274 */
        g_choice = sel;

        if (sel < 4) {                              /* 0x027C: a real occupation */
            g_active_save[0] = (uint8_t)sel;        /* 0x0286: store profession  */
            switch (sel) {
            case 1:  g_15d2 = 0x8b; g_15d4 = 0; g_15d6 = 0x4800; break;  /* banker    */
            case 2:  g_15d2 = 0x8a; g_15d4 = 0; g_15d6 = 0x4800; break;  /* carpenter */
            case 3:  g_15d2 = 0x89; g_15d4 = 0; g_15d6 = 0x4800; break;  /* farmer    */
            default: break;
            }
            gfx_screen_init_1ceb_0b71();            /* 0x02D5 */
        } else {
            /* Option 4: explain the trade-off, then loop back to re-prompt. */
            draw_text(S(0x0cd), g_text_margin_x, 0x22); /* "...banker...more money..." 0x02E9 */
            draw_text(S(0x155), g_157a, g_157c);        /* "...farmer earns most points" 0x02FB */
            press_any_key();                            /* 0x0300 */
            clear_text_area();                          /* 0x0305 */
            if (g_quit_flag)                            /* 0x030A */
                return;
        }
    } while (g_choice >= 4);                        /* 0x0313: until valid 1-3 */
}                                                   /* 0x0320 ret 2 */

/* ---------------------------------------------------------- 0x0d08:0x04d0
 * Name the party: the wagon leader (required, non-blank) plus up to four other
 * members. Blank member slots are filled with a random default name. Names are
 * stored as 11-byte records at g_party_names (leader = record 0).
 */
void name_party(int arg)
{
    char preview[/*0x21a*/ 0x200];   /* [bp-0x21a] formatted-line scratch */
    int  blank;                      /* [bp-0x119] "name is blank" flag   */
    int  len, i, m;

    image_load_14c6_021a(S(0x3d5) /* "family.pcc" */, 0, 0);  /* 0x04EA */

    /* --- wagon leader: required, loop until a non-blank name is entered --- */
    do {
        blank = 1;                                              /* 0x0506 */
        draw_text(S(0x3e0) /* "What is the first name of the wagon leader? " */,
                  g_text_margin_x - 2, 0x77);                   /* 0x051B */
        fill_rect(g_157e, g_1580, g_157e + 0x50, g_1580 + 8);   /* 0x0536: clear field */
        read_field_1049_0d95(1, 0xA, S(0x40d) /* "A-Za-z '.-" */, &g_input_buf); /* 0x054B */
        len = g_input_buf;                                      /* 0x0550: counted length */
        for (i = 1; i <= len; i++)                              /* 0x055C..0x0580 */
            if ((&g_input_buf)[i] != ' ')                       /* any real char -> not blank */
                blank = 0;
    } while (blank);                                            /* 0x0587 */
    if (g_quit_flag) return;                                    /* 0x0589 */

    strncpy_n(g_party_names[0], &g_input_buf, 0xA);             /* 0x05A1: store leader */

    /* --- header + the numbered "1.\2.\3.\4.\5." list --- */
    fill_rect(0, 0x77, 0x13f, 0xc7);                            /* 0x05B5 */
    draw_text(S(0x418) /* "What are the first names of the four other members..." */, 0x77, 0); /* 0x05C6 */
    draw_text(S(0x45c) /* "1. \2. \3. \4. \5. " */, 0x90, 1);   /* 0x05D8 */
    far_sprintf(/* preview, "%s", g_party_names[0] */);         /* 0x05F0: line 1 = leader */
    far_print(S(0x470) /* "\" */);                              /* 0x05FA */
    draw_text_line(S(0x472) /* "(Enter names or press Enter)" */, 0xa0, 0xbe); /* 0x0611 */

    /* --- members 1..4: read each; a blank entry stops early --- */
    m = 0;
    do {
        m++;                                                    /* 0x061B */
        read_member_name_d08_032e(m);                           /* 0x0623 */
        if (g_quit_flag) return;                                /* 0x0626 */
        draw_text(S(0x470), g_157e /*0x1580 region*/, 0x19);    /* 0x063D */
        g_1580 -= 9;                                            /* 0x064F: next line up */
    } while (m < 4 && g_input_buf != 0);                        /* 0x0658..0x0666 */

    /* Any slots still empty get a random default name from the cs:0x5fa pool. */
    for (; m <= 4; m++) {                                       /* 0x066B.. */
        do { i = rand_1049_008c(0xA); } while (/*name used*/ 0);/* 0x0682: pick unused default */
        strncpy_n(g_party_names[m], /* DEFAULT_NAMES[i] @ cs:0x5fa */ 0, 0xA);   /* 0x06BF */
        /* (preview line formatted/redrawn for slot m) */       /* 0x06C4.. */
    }
}                                                               /* ret 2 */

/* ---------------------------------------------------------- 0x0d08:0x0a4f
 * Choose the departure month. Options 1-5 map to March..July (stored as
 * g_departure_month = choice + 2); option 6 attends an advice meeting and
 * re-prompts. Once a month is set, the calendar starts at 1848.
 */
void choose_departure_month(int arg)
{
    char buf[256];   /* [bp-0x100] */

    do {
        draw_panel_1049_1df2(g_69c, 0xb5);                      /* 0x0A66 */
        /* "It is 1848. Your jumping off place ... which month to leave ..." */
        draw_text(S(0x828), g_text_margin_x, 0x22);             /* 0x0A78 */
        far_sprintf(/* buf, cs:0x8a8 "1. March\..6. Ask for advice" */); /* 0x0A90 */
        far_print((const char far *)&g_684);                    /* 0x0A9A */
        draw_text(buf, g_157a, g_157c);                         /* 0x0A9F: the month list */

        read_field_1049_0d95(1, 1, S(0x8f8) /* "1-6" */, &g_input_buf);  /* 0x0AB4 */
        if (g_quit_flag) return;                                /* 0x0AB9 */
        g_choice = parse_choice_1049_180b(&g_input_buf);        /* 0x0AC7 */
        gfx_screen_init_1ceb_0b71();                            /* 0x0ACF */

        if (g_choice < 6) {                                     /* 0x0AD4: a month */
            g_departure_month = (uint8_t)(g_choice + 2);        /* 0x0ADB: 3=Mar..7=Jul */
        } else {                                                /* option 6: advice */
            draw_panel_1049_1df2(g_69c, g_69e);                 /* 0x0AEE */
            /* "You attend a public meeting ... California-Oregon fever ..." */
            draw_text(S(0x8fc), g_text_margin_x, 0x25);         /* 0x0B00 */
            /* "If you leave too early, there won't be any grass ..." */
            draw_text(S(0x95d), g_157a, g_157c);                /* 0x0B12 */
            press_any_key();                                    /* 0x0B17 */
            if (g_quit_flag) return;                            /* 0x0B1C */
            gfx_screen_init_1ceb_0b71();                        /* 0x0B25 */
        }
    } while (g_choice >= 6);                                    /* 0x0B2A: until a month */

    g_year = 0x738;     /* 1848 */                              /* 0x0B34 */
    g_15c0 = 1;                                                 /* 0x0B3A */
}                                                               /* 0x0B42 ret 2 */

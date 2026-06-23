/*
 * seg_d08_game_start.c - "Travel the Trail" entry (menu option 1).
 *
 * Recovered from build/OREGON_unpacked.exe, code segment 0x0d08.
 *   travel_the_trail          @ 0x0d08:0x2217  (far, called from main)
 *   prompt_continue_saved_game@ 0x0d08:0x20ee  (near)
 *
 * Address-annotated reconstruction of Borland Turbo C output; unresolved
 * helpers keep machine names. Not yet compile-verified against the binary.
 *
 * Saved games are stored as a list of up to SAVE_SLOTS records of SAVE_REC_SIZE
 * bytes each; the active game record lives in DGROUP at 0x1588.
 */

#include <stdint.h>

#define SAVE_REC_SIZE 0x8F   /* 143 bytes per saved-game record   */
#define SAVE_SLOTS    40     /* 0x165c local buffer / 143 ~= 40   */

/* ------------------------------------------------------------------ globals */
extern uint8_t  g_quit_flag;       /* 0x1520 */
extern uint8_t  g_1552;            /* 0x1552: set to 1 at game start            */
extern uint8_t  g_1586, g_1587;    /* 0x1586/0x1587: travel-abort sub-flags     */
extern uint8_t  g_input_buf;       /* 0x141a: raw keyboard input                */
extern uint8_t  g_active_save[SAVE_REC_SIZE];   /* 0x1588: active game record   */
extern uint8_t  g_0bc0[SAVE_REC_SIZE];          /* 0x0bc0: scratch save record  */
extern uint16_t g_state_1620[3];   /* 0x1620: set to {0x3779,0x4189,0x6560}     */
extern uint16_t g_state_1626[3];   /* 0x1626: zeroed                            */
extern uint16_t g_state_1644[3];   /* 0x1644: {0x297c,0x8f5c,0x75c2}            */
extern uint16_t g_state_1650[3];   /* 0x1650: {0x717b,0x0a3d,0x23d7}            */
extern uint16_t g_state_1662[6];   /* 0x1662..0x166c: repeated {0x..,0x0a3d,0x23d7} */

/* --------------------------------------------------------------- prototypes */
extern void __stkcheck_20a4_0244(int frame);   /* 0x20a4:0x0244 */
extern void sub_20a4_0bc1(void);               /* 0x20a4:0x0bc1: called at travel start */
extern void memcpy_20a4_025d(void far *dst, const void far *src, int n); /* 0x20a4:0x025d */
extern int  streq_20a4_0724(const char far *a, const char far *b);       /* 0x20a4:0x0724: returns ZF */

extern void sub_1049_1e43(void);
extern void draw_1049_1855(/* ... */);
extern void load_save_list_1049_1f84(void far *buf);   /* 0x1049:0x1f84 */
extern void read_input_1049_1bb3(void far *dst);       /* 0x1049:0x1bb3 */
extern int  parse_num_1049_2110(const char far *s, int far *out); /* 0x1049:0x2110 */
extern void save_list_1049_202a(void far *buf);        /* 0x1049:0x202a */

static void prompt_continue_saved_game(int arg);       /* 0x0d08:0x20ee */
static void start_new_game(int arg);                   /* 0x0d08:0x1f7c */

/* New-game setup steps (segment 0x0d08, near). Identified by their prompts. */
extern void choose_profession(int arg);        /* 0x0d08:0x01f2  "1-4" occupations   */
extern void name_party(int arg);               /* 0x0d08:0x04d0  wagon leader + 4    */
extern void choose_departure_month(int arg);   /* 0x0d08:0x0a4f  March..July / advice */
extern void buy_supplies(int arg);             /* 0x0d08:0x1e78  store, supplies.pcc  */
extern void gfx_screen_init_1ceb_0b71(void);   /* 0x1ceb:0x0b71 */

/* Borland long (32-bit) arithmetic runtime helpers (segment 0x20a4). */
extern long lmul_20a4_0b00(long a, long b);    /* 0x20a4:0x0b00 */
extern long ldiv_20a4_0b14(long a, long b);    /* 0x20a4:0x0b14 */
extern long lhelp_20a4_0aee(long a);           /* 0x20a4:0x0aee */
extern long lhelp_20a4_0b50(void);             /* 0x20a4:0x0b50 */

/* Additional game-state globals (DGROUP). */
extern uint8_t  g_profession;      /* 0x15c1: chosen occupation (0..3, <4 = preset) */
extern uint8_t  g_15e8, g_15e9, g_1610, g_160a;
extern uint16_t g_15d8, g_15da, g_15dc, g_15ea, g_160b, g_160d, g_1611, g_1613, g_1615;
extern long     g_start_fund;      /* 0x15f2 */
extern long     g_score_factor;    /* 0x15ec: derived from (7 - profession)         */
extern uint8_t  g_15f8[0x11];      /* 17-byte state array                            */
extern uint8_t  g_15de[4];         /* 4 party-member flags                           */
extern uint8_t  g_15e3[4];

/* CONST pool strings in this segment:
 *   cs:0x20c1 "Would you like to continue\a saved game? "
 *   cs:0x20eb "Y"                                                            */

/* ---------------------------------------------------------- 0x0d08:0x2217 */
void travel_the_trail(void)
{
    sub_20a4_0bc1();                 /* 0x2225 */
    g_1586 = 0;                      /* 0x222A */
    g_1587 = 0;                      /* 0x222F */
    sub_1049_1e43();                 /* 0x2234 */
    g_1552 = 1;                      /* 0x2239 */

    prompt_continue_saved_game(/* bp */ 0);  /* 0x223F */
    if (g_quit_flag)                 /* 0x2242 */
        return;

    /* Initialise a fresh game's state vectors (0x224B..). */
    g_state_1620[0] = 0x3779;  g_state_1620[1] = 0x4189;  g_state_1620[2] = 0x6560;
    g_state_1626[0] = 0;       g_state_1626[1] = 0;       g_state_1626[2] = 0;
    g_state_1644[0] = 0x297c;  g_state_1644[1] = 0x8f5c;  g_state_1644[2] = 0x75c2;
    g_state_1650[0] = 0x717b;  g_state_1650[1] = 0x0a3d;  g_state_1650[2] = 0x23d7;
    g_state_1662[0] = 0x717a;  g_state_1662[1] = 0x0a3d;  g_state_1662[2] = 0x23d7;
    g_state_1662[3] = 0x717b;  g_state_1662[4] = 0x0a3d;  g_state_1662[5] = 0x23d7;
}                                    /* 0x22BA retf */

/* ---------------------------------------------------------- 0x0d08:0x20ee
 * "Would you like to continue a saved game?" If yes, pick a slot, copy it into
 * the active game record and delete it from the list; if no/empty, start fresh.
 */
static void prompt_continue_saved_game(int arg)
{
    uint8_t list[SAVE_SLOTS][SAVE_REC_SIZE];   /* [bp-0x165c], 0x165c bytes */
    int sel;                                   /* [bp-0x2]  selected slot   */
    int i;                                     /* [bp-0x4]  shift index     */

    load_save_list_1049_1f84(list);            /* 0x2103 */
    if (list[0][0] == 0) {                      /* 0x2108: no saved games   */
        start_new_game(arg);          /* 0x220B */
        return;
    }

    draw_1049_1855(/* 0x5f, 0x10, cs:0x20c1 prompt */);  /* 0x211F */
    read_input_1049_1bb3(&g_input_buf);                  /* 0x2129 */
    if (g_quit_flag)                                     /* 0x212E */
        return;

    if (!streq_20a4_0724(&g_input_buf, /* cs:0x20eb "Y" */ 0)) {  /* 0x2142: not "Y" */
        start_new_game(arg);                    /* 0x2203 */
        return;
    }

    /* Selected to continue: which slot? (list[0] header byte gates a parse.) */
    if (list[0][SAVE_REC_SIZE - 2] != 0)        /* 0x214C: [bp-0x15cd] */
        parse_num_1049_2110(/* cs:0x20ed */ 0, &sel);    /* 0x215D */
    else
        sel = 0;                                /* 0x2164 */
    if (g_quit_flag)                            /* 0x2169 */
        return;

    /* Copy the chosen record into the active game state, reload the list. */
    memcpy_20a4_025d(g_active_save, list[sel], SAVE_REC_SIZE);  /* 0x218C */
    load_save_list_1049_1f84(list);                            /* 0x2197 */

    /* Delete slot 'sel' by shifting the tail of the list down one record. */
    if (sel <= 0x26) {                          /* 0x219C/0x21A5: cap at slot 38 */
        for (i = sel; i != 0x26; i++)           /* 0x21AF..0x21E0 */
            memcpy_20a4_025d(list[i + 1], list[i], SAVE_REC_SIZE);
    }
    memcpy_20a4_025d(&g_0bc0, list[/* bp-0x93 -> last */ SAVE_SLOTS - 1], SAVE_REC_SIZE); /* 0x21F1 */
    save_list_1049_202a(list);                  /* 0x21FC */
}                                               /* 0x2214 ret 2 */

/* ---------------------------------------------------------- 0x0d08:0x1f7c
 * New-game setup: run the four setup screens (occupation, party names,
 * departure month, store), then initialise all game-state variables.
 */
static void start_new_game(int arg)
{
    int i;

    gfx_screen_init_1ceb_0b71();                /* 0x1F8A */

    choose_profession(arg);                     /* 0x1F90 */
    if (g_quit_flag) return;
    name_party(arg);                            /* 0x1F9E */
    if (g_quit_flag) return;
    choose_departure_month(arg);                /* 0x1FAC */
    if (g_quit_flag) return;
    buy_supplies(arg);                          /* 0x1FBA */
    if (g_quit_flag) return;

    g_15e8 = 0; g_15e9 = 0;                      /* 0x1FC7 */
    g_15d8 = 0; g_15da = 0; g_15dc = 0;          /* 0x1FD1 */
    g_15ea = 0;                                  /* 0x1FE5 */
    g_1610 = 1;                                  /* 0x1FE8 */

    /* Preset occupations (< 4) seed a starting fund; custom (>= 4) starts 0. */
    if (g_profession < 4)                        /* 0x1FED */
        g_start_fund = lmul_20a4_0b00(lhelp_20a4_0b50(), 0x84);   /* 0x1FF4..0x2009 */
    else
        g_start_fund = 0;                        /* 0x2013 */

    /* Final-score factor scales with occupation difficulty: (7 - profession). */
    g_score_factor = lhelp_20a4_0aee(            /* 0x2042..0x204E */
        ldiv_20a4_0b14(lhelp_20a4_0b50(), 7 - g_profession));    /* 0x202D..0x203A */

    for (i = 0; i < 0x11; i++) g_15f8[i] = 0;    /* 0x2052..0x2068 */
    for (i = 0; i < 4;    i++) g_15de[i] = 0;    /* 0x206A..0x2080 */
    for (i = 0; i < 4;    i++) g_15e3[i] = 0;    /* 0x2082..0x2098 */

    g_160a = 0; g_160b = 0; g_160d = 0;          /* 0x209A */
    g_1611 = 0; g_1613 = 0; g_1615 = 0;          /* 0x20A9 */
}                                               /* 0x20BE ret 2 */

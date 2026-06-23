/*
 * seg000_main.c - The Oregon Trail (MS-DOS 1990) program entry / main menu.
 *
 * Recovered from build/OREGON_unpacked.exe, code segment 0x0000, entry IP
 * 0x010A (= the MZ entry point CS:IP = 0x0000:0x010A). Function spans
 * 0x010A..0x0318.  This is a faithful, address-annotated reconstruction of the
 * Borland Turbo C output; helper functions and globals not yet identified keep
 * their `sub_SSSS_OOOO` / `g_OFFSET` machine names. Compile-verification against
 * the binary is future work.
 *
 * The menu choice variable g_menu_choice (DGROUP:0x1554) drives the title
 * screen the player first sees:
 *     1. Travel the Trail            -> sub_d08_2217   (play the game)
 *     2. Learn about the Trail       -> sub_c6b_51b
 *     3. See the Oregon Top Ten      -> sub_c00_433    (high scores)
 *     4. Turn the sound on/off       -> handled inline (toggles g_sound_on)
 *     5. Choose Management Options   -> sub_f34_fad
 *     6. End                         -> exit(0)
 */

#include <stdint.h>

/* ------------------------------------------------------------------ globals */
/* All in DGROUP unless noted. Names are inferred from use; offsets are exact. */
extern uint8_t  g_quit_flag;      /* 0x1520: set by sub-screens to abort to top */
extern uint16_t g_1412;           /* 0x1412 */
extern uint8_t  g_sound_on;       /* 0x1410: 0/1, toggled by menu option 4      */
extern uint16_t g_menu_choice;    /* 0x1554: parsed 1..6 selection              */
extern uint8_t  g_input_buf;      /* 0x141a: raw keyboard input (counted string)*/
extern char     g_sound_label[];  /* 0x1558: "on."/"off." scratch for option 4  */
extern uint16_t g_157a, g_157c;   /* 0x157a/0x157c: cursor row/col              */
extern uint16_t g_157e, g_1580;   /* 0x157e/0x1580: menu line position          */
extern uint16_t g_6a0;            /* 0x06a0: screen width / attribute           */
extern uint16_t g_684;            /* 0x0684: far ptr target for status print     */
extern uint8_t  g_died;           /* 0x1586: party death / game-ending flag      */
extern uint8_t  g_game_over;      /* 0x1587: trail finished (reached Oregon)     */

/* --------------------------------------------------------------- prototypes */
/* C runtime / library group (segment 0x20a4). */
extern void   cstartup_20a4_0000(void);            /* 0x20a4:0x0000 */
extern void   _exit_20a4_00d8(int code);           /* 0x20a4:0x00d8: terminate  */
extern void   far_sprintf_20a4_0634(/* dst, fmt, ... */);  /* 0x20a4:0x0634 */
extern void   far_print_20a4_06c1(const char far *s);      /* 0x20a4:0x06c1 */
extern void   strncpy_20a4_064e(char far *dst, const char far *src, int n); /* 0x20a4:0x064e */

/* Program modules. */
extern void   init_2042_0000(void);                /* 0x2042:0x0000 */
extern void   sub_2042_0215(int a, int b);         /* 0x2042:0x0215 */
extern void   init_1ceb_1357(void);                /* 0x1ceb:0x1357 */
extern void   init_182e_0000(void);                /* 0x182e:0x0000 */
extern void   init_150c_0000(void);                /* 0x150c:0x0000 */

extern void   sub_1049_45ce(void);
extern void   sub_1049_418c(void);
extern void   sub_1049_2be1(void);
extern void   sub_1049_0e7c(int a);
extern void   draw_1049_1855(/* varargs in asm */);/* 0x1049:0x1855: render text */
extern void   sub_1049_1521(/* row, col, far str */);
extern void   sub_1049_0c12(/* read input */);     /* 0x1049:0x0c12 */
extern int    parse_1049_180b(uint8_t far *s);     /* 0x1049:0x180b: parse choice */
extern void   sub_1049_1e43(void);
extern void   sub_1049_15a0(void);
extern void   cleanup_1049_42fb(void);

extern void   travel_the_trail(void);  /* menu 1: 0x0d08:0x2217 - set up game */
extern void   travel_loop(void);       /* menu 1: 0x0032:0x3f93 - play the trail */
extern void   sub_cdb_00df(void);
extern void   learn_about_trail(void); /* menu 2: 0x0c6b:0x051b */
extern void   topten_c00_0433(void);   /* menu 3 */
extern void   manage_f34_0fad(void);   /* menu 5 */

/* Menu-line string constants live in this code segment's CONST pool (cs:NN).
 * 0x68 "off"  0x6c "on"  0x6f ")  5. Choose Management Options\  6. End\\"
 * 0x99 "1-6"  0x9d "on." 0xa1 "off."  0xa6 "The sound is now turned "        */

void main(void)                                              /* 0x010A */
{
    char screen_buf[256];   /* sub sp,0x100 */

    cstartup_20a4_0000();   /* 0x010A */
    init_2042_0000();       /* 0x010F */
    init_1ceb_1357();       /* 0x0114 */
    init_182e_0000();       /* 0x0119 */
    init_150c_0000();       /* 0x011E */

    sub_1049_45ce();        /* 0x012A */
    sub_1049_418c();        /* 0x012F */
    g_quit_flag = 0;        /* 0x0134 */
    g_1412 = 0;             /* 0x0139 */
    sub_1049_2be1();        /* 0x013E */
    sub_1049_0e7c(1);       /* 0x0147 */

    for (;;) {                                               /* loop top 0x0134 */
        /* draw the title/menu screen */
        draw_1049_1855(/* g_6a0, 0x3f, &cs:0x0 */);          /* 0x0159 */
        /* menu line 4 shows current sound state ("off"/"on") */
        sub_1049_1521(/* g_157e, g_1580, */
                      g_sound_on ? /* cs:0x68 "off" */ 0 : /* cs:0x6c "on" */ 0);   /* 0x015E-0x0186 */

        far_sprintf_20a4_0634(/* &screen_buf, cs:0x6f menu tail, g_157a, g_157c */); /* 0x0199 */
        far_print_20a4_06c1((const char far *)&g_684);       /* 0x01A8 */
        draw_1049_1855();                                    /* 0x01AD */
        sub_2042_0215(0x13, 0x1a);                           /* 0x01B8 */

        sub_1049_0c12(/* 1, 1, cs:0x99 "1-6", &g_input_buf */); /* 0x01CD: prompt+read */
        if (g_input_buf > 0)                                 /* 0x01D2 */
            g_menu_choice = parse_1049_180b(&g_input_buf);   /* 0x01DE */
        else
            g_menu_choice = 0xA;                             /* 0x01E8 */

        if (g_menu_choice == 1 || g_menu_choice == 2 || g_menu_choice == 4)
            sub_1049_1e43();                                 /* 0x0203 */

        if (g_quit_flag)                                     /* 0x0208 */
            goto check_end;

        switch (g_menu_choice) {                             /* 0x0212 */
        case 1:                                              /* Travel the Trail */
            travel_the_trail();                              /* 0x021A: set up the game */
            if (!g_quit_flag) {                              /* 0x0224: only if not aborted */
                travel_loop();                               /* 0x0226: play the trail */
                /* Quit mid-trail (Esc) without dying or finishing -> back to menu. */
                if (g_quit_flag && g_died == 0 && g_game_over == 0) { /* 0x0230-0x023E */
                    g_quit_flag = 0;                         /* 0x0240 */
                    sub_cdb_00df();                          /* 0x0245 */
                }
            }
            break;
        case 2:                                              /* Learn about the Trail */
            learn_about_trail();                             /* 0x0252 */
            break;
        case 3:                                              /* See the Oregon Top Ten */
            topten_c00_0433();                               /* 0x025F */
            break;
        case 4:                                              /* Turn the sound on/off */
            g_sound_on = !g_sound_on;                        /* 0x026F-0x027C */
            if (g_sound_on)
                strncpy_20a4_064e(g_sound_label, /* cs:0x9d "on." */ 0, 0xC);  /* 0x0294 */
            else
                strncpy_20a4_064e(g_sound_label, /* cs:0xa1 "off." */ 0, 0xC); /* 0x02A9 */
            /* "The sound is now turned <state>" */
            far_sprintf_20a4_0634(/* &screen_buf, cs:0xa6, g_6a0, 0x50 */);    /* 0x02C1 */
            far_print_20a4_06c1((const char far *)g_sound_label);             /* 0x02CB */
            draw_1049_1855();                                /* 0x02D0 */
            draw_1049_1855(/* g_157a, g_157c+9, cs:0xbf */); /* 0x02E5 */
            sub_1049_15a0();                                 /* 0x02EA */
            sub_1049_1e43();                                 /* 0x02EF */
            break;
        case 5:                                              /* Choose Management Options */
            manage_f34_0fad();                               /* 0x02FB */
            break;
        default:
            break;
        }

    check_end:                                               /* 0x0300 */
        if (g_menu_choice == 6)                              /* 6 = End */
            break;
    }

    cleanup_1049_42fb();                                     /* 0x030A */
    _exit_20a4_00d8(0);                                      /* 0x0312-0x0314 */
}

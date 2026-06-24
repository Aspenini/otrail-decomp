/*
 * seg_f34_management.c - "Choose Management Options" (main menu option 5).
 *
 *   choose_management @ 0x0f34:0x0fad  (far)
 *
 * The settings/admin screen: view the high-score lists, erase saved data, and
 * configure the joystick. Loops until "Return to the main menu" (8) or Esc.
 *
 * Address-annotated reconstruction of Borland Turbo C output; not yet
 * compile-verified against the binary.
 */

#include <stdint.h>

extern uint8_t  g_quit_flag;     /* 0x1520 */
extern uint8_t  g_input_buf;     /* 0x141a */
extern uint8_t  g_joystick_on;   /* 0x1411: joystick enabled flag (on/off label) */
extern uint16_t g_684;           /* 0x0684 */
extern uint8_t  g_topten_cur[];  /* 0x1674: current Top Ten list data            */

extern void draw_text(const char far *s, int x, int y);      /* 0x1049:0x1855 */
extern void draw_text_line(const char far *s, int x, int y); /* 0x1049:0x1b3c */
extern void press_any_key(void);                             /* 0x1049:0x15a0 */
extern void show_top_ten(const void far *list);              /* 0x1049:0x2a47 - render a Top Ten table */
extern void far_sprintf(/* ... */);                          /* 0x20a4:0x0634 */
extern void far_print(const char far *s);                    /* 0x20a4:0x06c1 */
extern void read_field(int a, int b, const char far *cs, void far *dst); /* 0x1049:0x0d95 */
extern int  parse_choice(const char far *s);                 /* 0x1049:0x180b */
extern void show_input_prompt_2042_0215(int a, int b);       /* 0x2042:0x0215 */
extern void gfx_screen_init_1ceb_0b71(void);                 /* 0x1ceb:0x0b71 */
extern void gfx_1ceb_17db(int a);                            /* 0x1ceb:0x17db */
extern void gfx_box_1ceb_16a1(int x1, int y1, int x2, int y2); /* 0x1ceb:0x16a1 */

/* Management sub-actions (segment 0x0f34, near). */
extern void erase_current_topten_f34_009b(void);   /* 0x0f34:0x009b */
extern void erase_tombstones_f34_0264(void);       /* 0x0f34:0x0264 */
extern void erase_saves_f34_04b3(void);            /* 0x0f34:0x04b3 */
extern void toggle_joystick_f34_0d54(void);        /* 0x0f34:0x0d54 */
extern void calibrate_joystick_f34_0980(void);     /* 0x0f34:0x0980 */

#define S(off) ((const char far *)(off))

/* CONST strings (cs:NN): 0xe50 "The Oregon Trail"  0xe61 "Version 2.0"
 * 0xe6d "Management Options"  0xe80 the menu body  0xf40 "on" / 0xf43 "off"
 * 0xf47 "  7. Calibrate joystick\  8. Return to the main menu\\"  0xf7e "1-8" */

/* ---------------------------------------------------------- 0x0f34:0x0fad */
void choose_management(void)
{
    int sel;

    for (;;) {                                          /* loop top 0x0FBC */
        gfx_screen_init_1ceb_0b71();
        draw_text_line(S(0xe50) /* "The Oregon Trail" */, 0, 0);  /* 0x0FCD */
        draw_text_line(S(0xe61) /* "Version 2.0" */, 0, 0);       /* 0x0FDF */
        gfx_1ceb_17db(0);                               /* 0x0FE8 */
        gfx_box_1ceb_16a1(0, 0, 0, 0);                  /* 0x0FFC */
        draw_text_line(S(0xe6d) /* "Management Options" */, 0, 0); /* 0x100E */
        /* "You may: 1. See the current Top Ten list / 2. See the original ... /
         *  3. Erase the current Top Ten list / 4. Erase the tombstone messages /
         *  5. Erase saved games / 6. Turn joystick " */
        draw_text(S(0xe80), 0, 0);                      /* 0x1023 */
        draw_text(g_joystick_on ? S(0xf40) /* "on" */
                                : S(0xf43) /* "off" */, 0, 0);     /* 0x1028..0x1050 */
        /* "  7. Calibrate joystick / 8. Return to the main menu" */
        far_sprintf(/* cs:0xf47 */);                    /* 0x106B */
        far_print((const char far *)&g_684);            /* 0x1075 */
        draw_text(/* assembled menu */ 0, 0, 0);        /* 0x107A */

        show_input_prompt_2042_0215(0, 0);              /* 0x1085 */
        read_field(0, 0, S(0xf7e) /* "1-8" */, &g_input_buf);     /* 0x109A */
        gfx_screen_init_1ceb_0b71();                    /* 0x109F */
        if (g_quit_flag) return;                        /* 0x10A4 */

        sel = parse_choice(&g_input_buf);               /* 0x10B3 */
        switch (sel) {
        case 1:                                         /* 0x10C1 */
            draw_text_line(S(0xf82) /* "Current Top Ten list" */, 0, 0);
            show_top_ten(g_topten_cur);                 /* 0x10D1: from 0x1674 */
            press_any_key();
            break;
        case 2:                                         /* 0x10E0 */
            draw_text_line(S(0xf97) /* "Original Top Ten list" */, 0, 0);
            show_top_ten(S(0x750) /* built-in default list */);   /* 0x10F0 */
            press_any_key();
            break;
        case 3: erase_current_topten_f34_009b(); break; /* 0x1102 */
        case 4: erase_tombstones_f34_0264();     break; /* 0x110D */
        case 5: erase_saves_f34_04b3();          break; /* 0x1118 */
        case 6: toggle_joystick_f34_0d54();      break; /* 0x1123 */
        case 7: calibrate_joystick_f34_0980();   break; /* 0x112E */
        case 8: return;                                 /* 0x1138: back to main menu */
        default: break;
        }
    }
}                                                       /* 0x114C retf */

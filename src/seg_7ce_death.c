/*
 * seg_7ce_death.c - the death / tombstone sequence (segment 0x07ce).
 *
 *   death_sequence  @ 0x07ce:0x0f37  (far)
 *   show_death_msg  @ 0x07ce:0x0db9  (near) - draws the cause-of-death message
 *
 * Called from travel_loop when g_died is set. Draws the tombstone for the dead
 * member, lets the player write an epitaph (stored in TOMB.REC), and -- if the
 * whole party has died -- shows "All of the people in your party have died."
 *
 * Address-annotated reconstruction of Borland Turbo C output; not yet
 * compile-verified against the binary.
 */

#include <stdint.h>

extern uint8_t  g_quit_flag;    /* 0x1520 */
extern uint8_t  g_input_buf;    /* 0x141a */
extern uint16_t g_location;     /* 0x15ea */
extern uint8_t  g_next_location;/* 0x1610 */
extern uint8_t  g_160f;         /* 0x160f: index of the member who died */
extern char     g_party_names[5][11];  /* 0x1589 */

extern void draw_text(const char far *s, int x, int y);      /* 0x1049:0x1855 */
extern void draw_text_line(const char far *s, int x, int y); /* 0x1049:0x1b3c */
extern void fill_rect(int x1, int y1, int x2, int y2);       /* 0x1049:0x155e */
extern void draw_tombstone_1049_274e(void);                  /* 0x1049:0x274e */
extern void draw_landmark_1049_2589(void);                   /* 0x1049:0x2589 */
extern void strncpy_n(char far *d, const char far *s, int n);/* 0x20a4:0x064e */
extern void read_input(void far *dst);                       /* 0x1049:0x1bb3: read Y/N  */
extern void read_field(int a, int b, const char far *cs, void far *dst); /* 0x1049:0x0d95 */
extern void show_input_prompt_2042_0215(int a, int b);       /* 0x2042:0x0215 */
extern int  streq(const char far *a, const char far *b);     /* 0x20a4:0x0724 */
extern void gfx_screen_init_1ceb_0b71(void);                 /* 0x1ceb:0x0b71 */
extern void show_death_msg_7ce_0db9(void);                   /* 0x07ce:0x0db9 */

extern uint8_t  g_all_dead;     /* set when the whole party is gone */

#define S(off) ((const char far *)(off))

/* ---------------------------------------------------------- 0x07ce:0x0f37 */
void death_sequence(void)
{
    char epitaph[/*…*/ 0x40];

    /* The member who died (g_160f indexes the party-name records). */
    strncpy_n(g_party_names[g_160f], /* dead member name */ 0, 0);  /* 0x0F57 */

    gfx_screen_init_1ceb_0b71();                        /* 0x0F6E */
    draw_tombstone_1049_274e();                         /* 0x0F78 */
    show_death_msg_7ce_0db9();                          /* 0x0F7E: cause of death */
    if (g_quit_flag) return;                            /* 0x0F81 */

    /* "Would you like to write an epitaph? " */
    fill_rect(0, 0, 0, 0);                              /* 0x0F9A */
    draw_text(S(0xe80), 0, 0);                          /* 0x0FAB */
    read_input(&g_input_buf);                           /* 0x0FC0: Y/N */
    if (g_quit_flag) return;                            /* 0x0FC5 */

    if (streq(&g_input_buf, S(0xea5) /* "Y" */)) {       /* 0x0FD9 */
        for (;;) {                                      /* edit loop, 0x0FE3 */
            /* "What would you like on the tombstone?" */
            fill_rect(0, 0, 0, 0);                      /* 0x0FF2 */
            draw_text(S(0xea7), 0, 0);                  /* 0x1003 */
            read_field(0, 0, S(0xece) /* "0-9A-Za-z ,!?$@#%^&*()\"'.-" */, epitaph); /* 0x1023 */
            if (g_quit_flag) return;                    /* 0x1028 */

            /* default to "N" if left blank, else store the epitaph */
            if (epitaph[0] == 0)                        /* 0x1045 */
                strncpy_n(epitaph, S(0xee9) /* "N" */, 0);

            draw_tombstone_1049_274e();                 /* 0x1066: redraw with epitaph */
            fill_rect(0, 0, 0, 0);                      /* 0x107A */
            draw_text(S(0xeeb) /* "Would you like to make changes? " */, 0, 0); /* 0x108B */
            read_input(&g_input_buf);                   /* 0x10A0 */
            if (g_quit_flag) return;                    /* 0x10A5 */
            if (streq(&g_input_buf, S(0xee9) /* "N" */)) /* 0x10B8 */
                break;                                  /* done editing */
        }
        /* epitaph saved to TOMB.REC */
    }

    /* If the whole party is gone, this is the end of the journey. */
    if (g_all_dead) {                                   /* 0x10C2 path */
        draw_landmark_1049_2589();                      /* 0x10CA */
        fill_rect(0, 0, 0, 0);                          /* 0x10DE */
        draw_text_line(S(0xf0c) /* "All of the people" */, 0, 0);          /* 0x10F0 */
        draw_text_line(S(0xf1e) /* "in your party have died." */, 0, 0);   /* 0x1102 */
        show_death_msg_7ce_0db9();                      /* 0x1108 */
    }
}                                                       /* 0x110E retf */

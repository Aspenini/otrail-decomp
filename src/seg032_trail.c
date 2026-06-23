/*
 * seg032_trail.c - the main trail loop (segment 0x0032, the travel engine).
 *
 *   travel_loop @ 0x0032:0x3f93  (far)
 *
 * Called from main() (menu option 1) right after travel_the_trail sets up the
 * game. Iterates over the trail's locations (g_location 0..0x11); each turn
 * draws the travel status, runs one travel turn (the "1. Continue on the
 * trail / 2. Check supplies / ..." menu lives in travel_turn @ 0x3cb3), and
 * handles landmark arrivals, death, and arrival at Oregon (location 0x11).
 *
 * Address-annotated structural reconstruction of Borland Turbo C output; the
 * per-turn phase helpers are named by inference and lifted separately. Not yet
 * compile-verified against the binary.
 */

#include <stdint.h>

/* ------------------------------------------------------------------ globals */
extern uint8_t  g_quit_flag;    /* 0x1520 */
extern uint8_t  g_died;         /* 0x1586: party member / game-ending death     */
extern uint8_t  g_game_over;    /* 0x1587: trail finished (reached Oregon)       */
extern uint8_t  g_first_turn;   /* 0x1552: set at game start, cleared after turn1*/
extern uint16_t g_location;     /* 0x15ea: current trail location index (0..0x11)*/
extern uint8_t  g_next_location;/* 0x1610: next location index                   */
extern uint16_t g_160d;         /* 0x160d: distance/progress toward next landmark*/
extern uint8_t  g_must_trade, g_172b; /* per-trip flags reset at loop entry            */
extern uint16_t g_1742;         /* landmark/location data pointer                */

/* --------------------------------------------------------------- prototypes */
extern void __stkcheck_20a4_0244(int frame);
extern void image_show_14c6_0321(const char far *name, void far *handle); /* 0x14c6:0x0321 */
extern void image_free_14c6_043c(int lo, int hi);                         /* 0x14c6:0x043c */
extern void gfx_screen_init_1ceb_0b71(void);                              /* 0x1ceb:0x0b71 */
extern void draw_landmark_1049_2589(uint16_t far *data, int flag);        /* 0x1049:0x2589 */

/* Trail-engine phases (segment 0x0032, near). */
extern void trail_phase_setup_32_04d4(void);   /* 0x0032:0x04d4 */
extern void draw_travel_status_32_03a6(void);  /* 0x0032:0x03a6 */
extern void arrive_landmark_32_0655(void);     /* 0x0032:0x0655 */
extern void travel_turn(void);                 /* 0x0032:0x3cb3 - the per-turn menu/movement (below) */
extern void show_ending_32_011d(void);         /* 0x0032:0x011d */

/* End-of-game handlers (segment 0x07ce). */
extern void death_sequence_7ce_0f37(void);     /* 0x07ce:0x0f37 */
extern void sub_7ce_039a(void);                /* 0x07ce:0x039a */

#define LOC_OREGON 0x11   /* arriving at location 17 = reached Oregon, game won */

/* ---------------------------------------------------------- 0x0032:0x3f93 */
void travel_loop(void)
{
    int img_travelox;   /* [bp-0x4] */
    int img_scenery;    /* [bp-0x8] */

    image_show_14c6_0321("travelox.pcc", &img_travelox);   /* 0x3FAB */
    image_show_14c6_0321("scenery.pcc",  &img_scenery);    /* 0x3FBA */

    g_must_trade = 0;                                            /* 0x3FBF */
    g_172b = 0;                                            /* 0x3FC8 */
    /* local pace/ration defaults set here ([bp-0xa]=3, etc.) */

    for (;;) {                                             /* loop top 0x3FD5 */
        if (g_died || g_game_over)                         /* 0x3FD5/0x3FDF */
            break;

        /* Redraw the current landmark on the first turn or at trail forks. */
        if (g_first_turn == 0 || g_location == 8)          /* 0x3FE9 */
            draw_landmark_1049_2589(&g_1742, 0);           /* 0x3FFF */

        trail_phase_setup_32_04d4();                       /* 0x4005 */
        draw_travel_status_32_03a6();                      /* 0x4009: date/weather/supplies */
        if (g_quit_flag) goto done;                        /* 0x400C */

        if (g_first_turn && g_160d > 0 && g_next_location != g_location) /* 0x402C */
            arrive_landmark_32_0655();                     /* 0x4046 */
        if (g_quit_flag) goto done;                        /* 0x4049 */

        g_first_turn = 0;                                  /* 0x4069 */
        travel_turn();                                     /* 0x406F: the core travel turn */
        if (g_quit_flag) goto done;                        /* 0x4072 */

        if (g_died)                                        /* 0x4091 */
            death_sequence_7ce_0f37();                     /* 0x4098 */

        g_location = g_next_location;                      /* 0x409D: advance */
        /* Reaching Oregon (location 0x11) without dying ends the game. */
        g_game_over = (g_location == LOC_OREGON && !g_died) ? 1 : 0;  /* 0x40A5..0x40B9 */
    }                                                      /* jmp 0x3FD5 (0x40BC) */

done:                                                      /* 0x40BF */
    image_free_14c6_043c(0, 0);   /* scenery  */           /* 0x40C5 */
    image_free_14c6_043c(0, 0);   /* travelox */           /* 0x40D0 */

    if (g_game_over) {                                     /* 0x40D5 */
        show_ending_32_011d();                             /* 0x40DD */
        if (g_quit_flag) return;                           /* 0x40E0 */
        sub_7ce_039a();                                    /* 0x40E9 */
    }
    gfx_screen_init_1ceb_0b71();                           /* 0x40EE */
}                                                          /* 0x40F6 retf */

/* --- per-turn engine -------------------------------------------------------
 * Status formatters (segment 0x1049) and action handlers (segments 0x0032,
 * 0x07ce player actions, 0x0606 hunting). Named from the travel menu dispatch.
 */
extern void fmt_weather_1049_2d8b(void);   /* 0x1049:0x2d8b */
extern void fmt_health_1049_2db7(void);    /* 0x1049:0x2db7 */
extern void fmt_pace_1049_2dfd(void);      /* 0x1049:0x2dfd */
extern void fmt_rations_1049_2e2a(void);   /* 0x1049:0x2e2a */
extern void draw_text2(const char far *s, int x, int y);  /* 0x1049:0x1855 */
extern void far_print2(const char far *s);                /* 0x20a4:0x06c1 */
extern void read_field2(int a, int b, const char far *cs, void far *dst); /* 0x1049:0x0d95 */
extern int  parse_choice2(const char far *s);             /* 0x1049:0x180b */

extern void must_trade_msg_32_3b68(void); /* 0x0032:0x3b68 */
extern void continue_travel_32_361a(void);       /* 0x0032:0x361a */
extern void check_supplies_32_3a82(void);  /* 0x0032:0x3a82 */
extern void look_at_map_32_2d16(void);     /* 0x0032:0x2d16 */
extern void change_pace_7ce_24bf(void);    /* 0x07ce:0x24bf */
extern void change_rations_7ce_27cf(void); /* 0x07ce:0x27cf */
extern void stop_to_rest_7ce_2950(void);   /* 0x07ce:0x2950 */
extern void attempt_trade_7ce_11c3(void);  /* 0x07ce:0x11c3 */
extern void talk_to_people_7ce_17eb(void); /* 0x07ce:0x17eb */
extern void hunt_606_1968(void);           /* 0x0606:0x1968 - hunting minigame */
extern void apply_hunt_1049_3be4(void);    /* 0x1049:0x3be4 */
extern void buy_at_fort_7ce_1d3c(void);    /* 0x07ce:0x1d3c */

extern uint8_t  g_at_fort;     /* 0x1729: at a fort/settlement (talk + buy avail)*/
extern uint8_t  g_1617;        /* 0x1617 */
extern uint8_t  g_1728;        /* 0x1728: end-of-turn / advance flag             */
/* g_must_trade (0x177c) declared above: set when a missing part blocks travel  */

/* ---------------------------------------------------------- 0x0032:0x3cb3
 * One travel turn: show the status panel (Weather/Health/Pace/Rations) and the
 * action menu, then dispatch the player's choice. Repeats until the turn ends
 * (g_1728), the player quits, or someone dies. At a fort, option 8 is "Talk to
 * people" and 9 is "Buy supplies"; on the open trail, option 8 is "Hunt".
 */
void travel_turn(void)
{
    int choice;

    if (g_1617 == 0) {                                  /* 0x3CC2 */
        continue_travel_32_361a();                            /* 0x3CD1 */
        if (g_quit_flag) return;
    }

    for (;;) {                                          /* loop top 0x3CC2 */
        /* --- status panel --- */
        gfx_screen_init_1ceb_0b71();                    /* 0x3CE9 */
        /* date / fort header drawn here (0x3CFA..0x3D5x) */
        far_print2("Weather: ");  fmt_weather_1049_2d8b();   /* 0x3D70 */
        far_print2("\Health: ");  fmt_health_1049_2db7();    /* 0x3D8A */
        far_print2("\Pace: ");    fmt_pace_1049_2dfd();      /* 0x3DA4 */
        far_print2("\Rations: "); fmt_rations_1049_2e2a();   /* 0x3DC3 */

        draw_text2("You may:\\", 0x4e, 2);              /* 0x3DF1 */
        draw_text2("1. Continue on trail\2. Check supplies\3. Look at map\\"
                   "4. Change pace\5. Change food rations\6. Stop to rest\\"
                   "7. Attempt to trade", 0x20, 0);     /* 0x3E08 */

        if (g_at_fort) {                                /* 0x3E20 */
            draw_text2("8. Talk to people", 0, 0);      /* 0x3E34 */
            /* a fort may also offer buying (near check at 0x3E3C) */
            draw_text2("9. Buy supplies", 0, 0);        /* 0x3E50: "1-9" */
        } else {
            draw_text2("8. Hunt for food", 0, 0);       /* 0x3E72 */
        }

        read_field2(1, 1, /* "1-8"/"1-9" */ 0, &g_input_buf);  /* 0x3EA9 */
        if (g_quit_flag) return;                        /* 0x3EAE */
        choice = parse_choice2(&g_input_buf);           /* 0x3EBD */

        switch (choice) {                               /* 0x3EC6.. */
        case 1:                                         /* Continue on the trail */
            if (g_must_trade) must_trade_msg_32_3b68();  /* 0x177c set: blocked, must trade */
            else              continue_travel_32_361a(); /* advance to the next location */
            break;
        case 2: check_supplies_32_3a82();   break;      /* 0x3EEC */
        case 3: look_at_map_32_2d16();      break;      /* 0x3EF9 */
        case 4: change_pace_7ce_24bf();     break;      /* 0x3F03 */
        case 5: change_rations_7ce_27cf();  break;      /* 0x3F0F */
        case 6: stop_to_rest_7ce_2950();    break;      /* 0x3F1B */
        case 7: attempt_trade_7ce_11c3();   break;      /* 0x3F27 */
        case 8:                                         /* 0x3F33 */
            if (g_at_fort) {
                talk_to_people_7ce_17eb();
            } else {
                hunt_606_1968();                        /* 0x3F41: hunting minigame */
                apply_hunt_1049_3be4();
            }
            break;
        case 9: buy_at_fort_7ce_1d3c();     break;      /* 0x3F55 */
        default: break;
        }

        if (g_quit_flag) return;                        /* 0x3F5A */
        if (g_died) return;                             /* 0x3F63 */
        if (g_1728) return;                             /* 0x3F6A: turn advanced */
        /* else redisplay the menu (jmp 0x3CC2) */
    }
}                                                       /* 0x3F77 ret 2 */

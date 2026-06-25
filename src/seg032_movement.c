/*
 * seg032_movement.c - the "Continue on the trail" movement engine (seg 0x0032).
 *
 *   continue_travel @ 0x0032:0x361a  (near)
 *
 * Dispatched from travel_turn when the player chooses "1. Continue on the
 * trail". It is a small state machine driven by g_at_landmark (0x1729):
 *
 *   - if the wagon is AT a landmark, leaving it triggers the landmark's
 *     handling (a trail fork, a river crossing, or the Dalles/Columbia choice);
 *   - otherwise it draws the travel view and the "size up the situation" prompt.
 *
 * In both cases it then leaves the landmark (clears g_at_landmark), checks for
 * oxen (check_oxen @ 0x356a), advances one simulated day (daily_update @
 * 0x1049:0x3be4 - calendar, illness, food), and computes the day's distance.
 *
 * The per-day mileage is computed HERE (0x38d4..0x3958), not in a separate
 * helper: a chain of 32-bit fixed-point operations on the snow depth (g_snow,
 * 0x15f2) and the afflicted-member count (g_172a, 0x172a), using constants like
 * 0xcccccccd (1/10) - deeper snow and more sick travellers cut the day's
 * distance. The result advances g_miles_past (0x160d) and reduces g_miles_to.
 *
 * Address-annotated structural reconstruction; not compile-verified.
 */

#include <stdint.h>

extern uint8_t  g_quit_flag;     /* 0x1520 */
extern uint8_t  g_must_trade;    /* 0x177c */
extern uint8_t  g_at_landmark;   /* 0x1729: at a landmark/settlement (fork/river/fort) */
extern uint16_t g_location;      /* 0x15ea: current location index */
extern uint8_t  g_179a;          /* 0x179a: display-mode flag */
extern uint8_t  g_160f, g_160d;  /* arrival bookkeeping (miles_to / miles_past) */
extern uint16_t g_160b;          /* legs-travelled counter */
extern uint8_t  g_died;          /* 0x1586: a death ended the game */

extern void far_sprintf(/* ... */);                       /* 0x20a4:0x0634 */
extern void far_print(const char far *s);                 /* 0x20a4:0x06c1 */
extern int  str_find_field(const char far *s, const char far *key); /* 0x20a4:0x06ed */
extern void press_any_key(void);                          /* 0x1049:0x15a0 */
extern void image_load_14c6_021a(const char far *name, int a, int b); /* 0x14c6:0x021a */
extern void gfx_screen_init_1ceb_0b71(void);              /* 0x1ceb:0x0b71 */

/* Landmark / movement helpers (segment 0x0032 unless noted, near). */
extern void trail_fork_32_2dbe(void);          /* 0x0032:0x2dbe: "the trail divides here" */
extern void cross_river_442_18bb(void);        /* 0x0442:0x18bb */
extern void columbia_dalles_32_2ff5(void);     /* 0x0032:0x2ff5: location 0x10 special */
extern void check_oxen_32_356a(void);          /* 0x0032:0x356a: block travel if out of oxen */
extern void daily_update_1049_3be4(void);      /* 0x1049:0x3be4: advance a day (calendar/illness/food) */
extern void size_up_prompt_32_080d(void);      /* 0x0032:0x080d: "Press ENTER to size up the situation" */
extern void draw_travel_scene_32_0bd4(void);   /* 0x0032:0x0bd4: draw the landscape/travel view */
extern void sub_7ce_412e(void);                /* 0x07ce:0x412e */

#define S(off) ((const char far *)(off))
#define LOC_DALLES 0x10   /* the Dalles: Columbia River vs. Barlow Road */

/* ---------------------------------------------------------- 0x0032:0x361a */
void continue_travel(void)
{
    if (!g_at_landmark) {                                /* 0x3629: open trail */
        draw_travel_scene_32_0bd4();                     /* 0x3800: landscape view */
        size_up_prompt_32_080d();                        /* 0x380F: "size up the situation" */
        return;
    }

    /* --- at a landmark: handle leaving it --- */
    trail_fork_32_2dbe();                                /* 0x3634: forks in the trail */
    if (g_quit_flag) return;                             /* 0x3637 */

    /* If this location is a river, run the crossing. */
    if (str_find_field(/* location data for g_location */ 0, S(0x35f1) /* "River" */)) { /* 0x3656 */
        cross_river_442_18bb();                          /* 0x3669 */
        g_160f = g_160f;                                 /* 0x3676: arrival bookkeeping */
        g_160b = g_160b;
        g_160d = 1;                                      /* 0x3686 */
        if (g_quit_flag) return;                         /* 0x3693 */
    }

    if (g_location == LOC_DALLES) {                      /* 0x369D */
        columbia_dalles_32_2ff5();                       /* 0x36A5 */
        if (g_quit_flag) return;
        /* the Dalles draws its own scene (P16.PCC) and ends the leg */
        gfx_screen_init_1ceb_0b71();                     /* 0x36C6 */
        image_load_14c6_021a(S(0x35f7) /* "P16.PCC" */, 0, 0);  /* 0x36DD/0x36F0 */
        sub_7ce_412e();                                  /* 0x36FA */
        return;
    }

    /* Arrival display: "From <current location> it is <N> miles to <next>." */
    draw_travel_scene_32_0bd4();                         /* 0x371A */
    far_sprintf(/* buf, cs:0x35ff "From " */);           /* 0x3743 */
    far_print(/* current location name (g_location) */ 0); /* 0x3758 */
    far_print(S(0x3605) /* " it is " */);                /* 0x3762 */
    far_print(/* <miles> */ 0);                          /* 0x376D */
    far_print(S(0x360d) /* " miles to " */);             /* 0x3777 */
    far_print(/* <next landmark name> */ 0);             /* 0x378E */
    far_print(S(0x3618) /* "." */);                      /* 0x3798 */
    press_any_key();                                     /* 0x37E7 */

    /* --- leave the landmark and advance the day --- (0x3882..0x3958) */
    g_at_landmark = 0;                                   /* 0x3882 */
    check_oxen_32_356a();                                /* 0x3888: out of oxen blocks travel */
    if (g_must_trade || g_died || g_quit_flag) return;   /* 0x388B..0x38A5 */

    daily_update_1049_3be4();                            /* 0x38B9: calendar / illness / food */
    if (g_quit_flag) return;                             /* 0x38BE */

    /* the day's distance: 32-bit fixed-point math on the travel-rate long
     * (0x15f2) and afflicted-member count (g_172a), with 1/10 constants;
     * advances g_miles_past and reduces g_miles_to. */
    /* g_miles_past += miles_today; g_miles_to -= miles_today;   0x38D4..0x3958 */
}                                                        /* 0x3A6F exit / retf */

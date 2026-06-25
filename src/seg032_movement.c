/*
 * seg032_movement.c - the "Continue on the trail" movement engine (seg 0x0032).
 *
 *   continue_travel @ 0x0032:0x361a  (near)
 *
 * Dispatched from travel_turn when the player chooses "1. Continue on the
 * trail". It is a small state machine driven by g_at_landmark (0x1729):
 *
 *   - if the wagon is AT a landmark, leaving it triggers the landmark's
 *     handling (a trail fork, a river crossing, the Dalles/Columbia choice, or
 *     a generic arrival), then shows "From <here> it is <N> miles to <next>.";
 *   - otherwise it draws the travel view (the landscape scene) and the
 *     "Press ENTER to size up the situation" prompt.
 *
 * Note: the per-day mileage/food advance (miles by pace, food by rations) is
 * driven by the daily-update path (see daily_update @ 0x1049:0x3be4) and a
 * helper not yet pinned down; this function is the arrival + travel-view
 * dispatcher. Address-annotated structural reconstruction; not compile-verified.
 */

#include <stdint.h>

extern uint8_t  g_quit_flag;     /* 0x1520 */
extern uint8_t  g_must_trade;    /* 0x177c */
extern uint8_t  g_at_landmark;   /* 0x1729: at a landmark/settlement (fork/river/fort) */
extern uint16_t g_location;      /* 0x15ea: current location index */
extern uint8_t  g_179a;          /* 0x179a: display-mode flag */
extern uint8_t  g_160f, g_160d;  /* arrival bookkeeping */
extern uint16_t g_160b;

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
extern void arrival_32_356a(void);             /* 0x0032:0x356a: generic landmark arrival */
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

    arrival_32_356a();                                   /* 0x3703: generic arrival */
    if (g_quit_flag || g_must_trade) return;             /* 0x3706..0x3712 */

    draw_travel_scene_32_0bd4();                                       /* 0x371A */
    /* "From <current location> it is <N> miles to <next landmark>." */
    far_sprintf(/* buf, cs:0x35ff "From " */);           /* 0x3743 */
    far_print(/* current location name (g_location) */ 0); /* 0x3758 */
    far_print(S(0x3605) /* " it is " */);                /* 0x3762 */
    far_print(/* <miles> */ 0);                          /* 0x376D */
    far_print(S(0x360d) /* " miles to " */);             /* 0x3777 */
    far_print(/* <next landmark name> */ 0);             /* 0x378E */
    far_print(S(0x3618) /* "." */);                      /* 0x3798 */
    press_any_key();                                     /* 0x37E7 */
}                                                        /* 0x3A6F exit / retf */

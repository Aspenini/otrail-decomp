/*
 * seg_health.c - the daily health / calendar simulation (segment 0x1049).
 *
 *   daily_update @ 0x1049:0x3be4  (far)
 *
 * Run once per simulated day (from rest, travel, and the per-advance paths).
 * It advances the calendar, rolls each party member's illness/condition, and
 * recomputes the party's overall health from rations, pace and conditions.
 * The actual formulas are 32-bit (long) arithmetic and are summarised here; the
 * value of this lift is the recovered data model.
 *
 * Address-annotated structural reconstruction; not yet compile-verified.
 *
 * --- recovered data model ------------------------------------------------
 * Calendar (DGROUP):
 *   g_day    0x15c0   day counter within the period
 *   g_month  0x15c1   current month 1..12 (wraps at 13 -> year++);
 *                     initialised to the chosen departure month
 *   g_year   0x15c2   calendar year (starts 1848)
 *
 * Party health: two parallel 5-entry arrays, one per member (leader + 4):
 *   g_member_status 0x15de[5]   condition/illness flag per member
 *   g_member_health 0x15e3[5]   health value per member
 *
 * Illness/condition roll for the day:
 *   g_illness 0x1730   illness level (0 = well; 2,3 = severity tiers)
 *   g_1732             illness flag
 *   g_172a             count of afflicted members
 *   0x172e .. 0x173e   long accumulators for the daily health computation
 */

#include <stdint.h>

/* Calendar */
extern uint8_t  g_day;            /* 0x15c0 */
extern uint8_t  g_month;          /* 0x15c1 */
extern uint16_t g_year;           /* 0x15c2 */

/* Per-member arrays (leader + 4 others). */
extern uint8_t  g_member_status[5];  /* 0x15de */
extern uint8_t  g_member_health[5];  /* 0x15e3 */

/* Illness / health state */
extern uint16_t g_illness;        /* 0x1730 */
extern uint16_t g_1732;           /* 0x1732 */
extern uint8_t  g_172a;           /* 0x172a */
extern uint8_t  g_pace;           /* 0x15e8 */
extern uint8_t  g_rations;        /* 0x15e9 */
extern uint16_t g_15cc;           /* 0x15cc */

extern int  rand_1049_008c(int n);          /* 0x1049:0x008c */
extern int  party_size_1049_2e57(void);     /* 0x1049:0x2e57: number of living members */
extern int  health_roll_1049_2e9c(void);    /* 0x1049:0x2e9c */
extern int  illness_pick_1049_2ed5(void);   /* 0x1049:0x2ed5 */

#define MONTHS_PER_YEAR 12

/* ---------------------------------------------------------- 0x1049:0x3be4 */
void daily_update(void)
{
    int m;   /* member index [bp-0x2] */

    /* --- advance the calendar one day --- */
    g_day = 1;                                          /* 0x3C0C */
    if (++g_month == MONTHS_PER_YEAR + 1) {             /* 0x3C1A: wrap at 13 */
        g_month = 1;
        g_year++;                                       /* 0x3C26 */
    }

    /* reset the day's distance/condition accumulators (0x15d8 long, etc.) */

    /* --- per-member illness/condition pass --- */
    g_172a = 0;
    for (m = 0; m < 4; m++) {                           /* 0x3C65..0x3CAC: members */
        if (g_member_status[m] == 0 || g_member_status[m] == 0xFF)
            continue;                                   /* skip well/dead */
        /* tally afflicted members; severity feeds the illness roll below */
        if (g_member_health[m] == 1)
            g_172a++;
    }

    /* --- roll the day's illness/condition --- */
    g_illness = rand_1049_008c(/* range */ 0) + health_roll_1049_2e9c();  /* 0x3CC6..0x3CD1 */
    illness_pick_1049_2ed5();                           /* 0x3CD5: choose illness type */
    g_1732 = /* condition flag */ 0;                    /* 0x3CF1 */

    /* --- recompute party health from the illness level, rations and pace ---
     * The result feeds the long accumulators at 0x172e..0x173e, branching on
     * g_illness severity (0x3DA3: ==2, 0x3DE6: ==3) and using g_15cc. The
     * formulas are 32-bit arithmetic and are summarised here. */
}

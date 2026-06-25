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
 * Calendar uses a days-in-month table in DGROUP:
 *   g_days_in_month 0x0889   month 1..12 -> days; drives the day/month/year wrap
 *
 * Party health: two parallel 5-entry arrays, one per member (leader + 4):
 *   g_member_ailment  0x15de[5]   current ailment code (0 = well, 0xff = dead)
 *   g_member_recovery 0x15e3[5]   days left to recover; at 0 the ailment clears
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

/* Days in each month, indexed by g_month (DGROUP 0x0889). */
extern uint8_t  g_days_in_month[13]; /* 0x0889 */

/* Per-member arrays (leader + 4 others). */
extern uint8_t  g_member_ailment[5];  /* 0x15de: current ailment code        */
extern uint8_t  g_member_recovery[5]; /* 0x15e3: days remaining until well    */

/* Illness / health state */
extern uint16_t g_illness;        /* 0x1730 */
extern uint16_t g_1732;           /* 0x1732 */
extern uint8_t  g_172a;           /* 0x172a: count of afflicted members */
extern uint8_t  g_pace;           /* 0x15e8 */
extern uint8_t  g_rations;        /* 0x15e9 */
extern uint16_t g_food;           /* 0x15ca: wagon food in pounds */
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
    if (++g_day > g_days_in_month[g_month]) {           /* 0x3BF2..0x3C0A */
        g_day = 1;                                      /* 0x3C0C */
        if (++g_month == MONTHS_PER_YEAR + 1) {         /* 0x3C1A: wrap at 13 */
            g_month = 1;
            g_year++;                                   /* 0x3C26 */
        }
    }

    /* clamp the day's distance accumulator (long at 0x15d8) to its max (0x3C2D) */

    /* --- per-member recovery pass --- */
    g_172a = 0;                                         /* 0x3C59 */
    for (m = 0; m <= 4; m++) {                          /* 0x3C60..0x3CAC: 5 members */
        if (g_member_ailment[m] == 0 || g_member_ailment[m] == 0xFF)
            continue;                                   /* skip the well and the dead */
        g_member_recovery[m]--;                         /* 0x3C7F: one day closer to well */
        g_172a++;                                       /* 0x3C8D: still afflicted today */
        if (g_member_recovery[m] < 1)
            g_member_ailment[m] = 0;                    /* 0x3CA3: recovered */
    }

    /* --- roll the day's illness/condition (only some days; 0x3CAE rand gate) --- */
    g_illness = health_roll_1049_2e9c() + rand_1049_008c(0x29);  /* 0x3CC6..0x3CD1 */
    illness_pick_1049_2ed5();                           /* 0x3CD5: choose illness type */
    g_1732 = /* new-illness flag from the roll */ 0;    /* 0x3CF1 */
    /* g_illness is then rescaled by ((g_illness+10) * 0x85.. / 0x2000), 0x3CF4 */

    /* --- consume the day's food --- (0x3E91..0x402C)
     * If the wagon still has food, the daily ration is g_rations * 2 scaled by a
     * health factor (5 - 2*g_illness) via 32-bit arithmetic; g_food (0x15ca) is
     * reduced by it (clamped at 0). Running out drives starvation through the
     * health roll above. The full long formula also reads g_15cc. */
    if (g_food > 0) {
        /* g_food -= ration_for(g_rations, g_illness);   0x4014..0x402C */
    }

    /* The remaining long accumulators (0x172e..0x173e) carry the recomputed
     * party-health terms, branching on g_illness severity (0x3DA3: ==2,
     * 0x3DE6: ==3). Summarised; the value of this lift is the data model. */
}

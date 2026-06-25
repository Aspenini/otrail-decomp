/*
 * seg032_events.c - random trail events (segment 0x0032, the trail engine).
 *
 * The misfortunes that strike during travel. Each event is a small handler that
 * formats a message and applies a random effect (lost supplies, illness, injury,
 * death) through shared helpers. They are triggered from the advance/daily-update
 * paths rather than one central dispatcher.
 *
 * Event roster found in segment 0x0032:
 *   snakebite     @ 0x0f05  "<member> has a snakebite."
 *   fire          @ 0x11da  "A fire in the wagon results in the loss of: ..."
 *   broken_wagon  @ 0x1711  "Broken wagon ... repair the broken wagon )"
 *   broken_part   @ 0x1b71  "<member/wagon> has a broken <part>"
 *   thief         @ 0x23fb  "A thief comes during the night and steals ..."
 *   (plus ox death/injury, illnesses, bad water, etc.)
 *
 * Illnesses are named from a table in DGROUP (0x2231:~0x0d67):
 *   exhaustion, typhoid, cholera, measles, dysentery, a fever
 *
 * Address-annotated structural reconstruction; the random loss/effect
 * arithmetic in each handler is summarised. Not yet compile-verified.
 */

#include <stdint.h>

extern uint16_t g_1730;        /* 0x1730: event/illness state */

/* Shared event helpers. */
extern int  rand_1049_008c(int n);          /* 0x1049:0x008c: random 0..n-1     */
extern int  pick_member_1049_39b9(void);    /* 0x1049:0x39b9: choose a party member */
extern void draw_msg_box_1049_3910(const char far *msg);  /* 0x1049:0x3910: centered message box */
extern void member_status_1049_36de(void);  /* 0x1049:0x36de: apply illness/injury to member */
extern void store_setup_1049_30c8(void);    /* 0x1049:0x30c8: supplies display     */
extern void far_sprintf(/* ... */);         /* 0x20a4:0x0634 */
extern void far_print(const char far *s);   /* 0x20a4:0x06c1 */
extern void strncat_n(char far *d, const char far *s, int n);  /* 0x20a4:0x064e */
extern void finalize_action_1049_1ee3(void);/* 0x1049:0x1ee3 */

#define S(off) ((const char far *)(off))

/* ---------------------------------------------------------- 0x0032:0x0f05
 * A party member is bitten by a snake; applies the injury and reports it.
 */
void event_snakebite(int member)
{
    /* "<member> has a snakebite." */
    far_sprintf(/* buf, member name */);                 /* 0x0F3D */
    far_print(S(0xef3) /* " has a snakebite." */);        /* 0x0F47 */
    /* show the message and apply the injury to the chosen member */
    draw_msg_box_1049_3910(/* msg */ 0);                  /* 0x0F7C */
    member_status_1049_36de();                            /* 0x0F81: apply injury */
    finalize_action_1049_1ee3();                          /* 0x0F86 */
}                                                         /* 0x0F8E ret 2 */

/* ---------------------------------------------------------- 0x0032:0x11da
 * The wagon catches fire, destroying a random amount of supplies.
 */
void event_fire(void)
{
    /* "A fire in the wagon results in the loss of:" */
    strncat_n(/* msg */ 0, S(0x11a9), 0);                 /* 0x11FE */
    store_setup_1049_30c8();                              /* 0x1209: supplies panel */
    /* pick random supply categories and subtract a random quantity from each
     * (long arithmetic on the supply records). */
}                                                         /* ~0x1310 */

/* ---------------------------------------------------------- 0x0032:0x23fb
 * A thief steals a random item during the night.
 */
void event_thief(void)
{
    int item;
    /* "A thief comes during the night and steals " */
    strncat_n(/* msg */ 0, S(0x23c3), 0);                 /* 0x2425 */
    item = rand_1049_008c(/* item count */ 0);            /* 0x242E: pick what's stolen */
    /* subtract the stolen amount from the matching supply record and report it */
    (void)item;
}

/* ---------------------------------------------------------- 0x0032:0x2046
 * Beneficial event: the party finds wild fruit, adding food to the wagon.
 * Food is capped at FOOD_MAX (2000 lb) -- the same ceiling the hunt enforces.
 */
#define FOOD_MAX  0x7d0   /* 2000 lb */
#define FRUIT_GAIN 0x14   /* +20 lb  */
extern uint16_t g_food;                     /* 0x15ca */
extern void image_show_14c6_0321(const char far *n, void far *h); /* 0x14c6:0x0321 */
extern void image_free_14c6_043c(int a, int b);                   /* 0x14c6:0x043c */

void event_find_fruit(void)
{
    if (g_food < FOOD_MAX - FRUIT_GAIN)         /* 0x2054: < 1980 */
        g_food += FRUIT_GAIN;                   /* 0x205F: +20 lb */
    else
        g_food = FOOD_MAX;                      /* 0x2067: cap at 2000 */

    image_show_14c6_0321(S(0x202a) /* "events.pcc" */, 0);   /* 0x20CE */
    image_free_14c6_043c(0, 0);                              /* 0x2101 */
    draw_msg_box_1049_3910(S(0x2035) /* "Find wild fruit." */); /* 0x210E */
    member_status_1049_36de();                               /* 0x2113 */
}

/* ---------------------------------------------------------- 0x0032:0x1711
 * A wagon part breaks: a random wheel, axle or tongue. The party may try to
 * repair it; if that fails, the part is replaced from the wagon's spares, and
 * if there is no spare, travel is blocked until the party trades for one.
 *
 * Spare-parts inventory (DGROUP, one byte each):
 *   g_spare_wheels  0x15cf
 *   g_spare_axles   0x15d0
 *   g_spare_tongues 0x15d1
 * Part names are counted strings in this segment: "wheel" (0x15f0), "axle"
 * (0x15f6), "tongue" (0x15fb).
 */
extern uint8_t g_sound_on;      /* 0x1410 */
extern uint8_t g_must_trade;    /* 0x177c: travel blocked until a part is traded */
extern char    g_needed_item[]; /* 0x177e: the part to trade for (counted string) */
extern uint8_t g_spare_wheels;  /* 0x15cf */
extern uint8_t g_spare_axles;   /* 0x15d0 */
extern uint8_t g_spare_tongues; /* 0x15d1 */
extern uint8_t g_input_buf;     /* 0x141a */
extern uint8_t g_quit_flag;     /* 0x1520 */
extern void size_up_prompt_32_080d(int tone, void far *ctx);  /* 0x0032:0x080d */
extern void press_any_key_1049_15a0(void);
extern int  streq_20a4_724(const char far *a, const char far *b);  /* 0 = equal */

void event_broken_wagon(void far *ctx)
{
    int  part;          /* [bp-0x02] 0=wheel 1=axle 2=tongue            */
    int  repaired;      /* [bp-0x07] repair attempt succeeded            */
    uint8_t *spare;

    size_up_prompt_32_080d(/* tone g_663 */ 0, ctx);   /* 0x172F "Press ENTER to size up..." */
    /* draw the broken-wagon scene (events art, 0xe70); animate if sound is on */
    if (g_sound_on) {                          /* 0x175B: 8-frame break animation */
        /* ... draw sprite frames via 0x1049:0x0fbd ... */
    }

    part = rand_1049_008c(3);                  /* 0x17A0: wheel / axle / tongue */
    /* strncpy the matching part name into the local buffer (0x15f0/0x15f6/0x15fb) */

    /* "Broken wagon <part>.  Would you like to try to repair it? [Y/N]" */
    /* draw_msg_box + read a Y/N reply (0x1602..); Esc bails out            */
    repaired = streq_20a4_724((const char far *)&g_input_buf, S(0x1639) /* "Y" */) == 0
               && /* repair roll succeeded (long-compare flag, 0x18ae..0x18c7) */ 1;
    if (g_quit_flag) return;

    if (repaired) {
        /* "You were able to repair the wagon <part>." then a time/health cost */
        size_up_prompt_32_080d(/* tone g_662 */ 0, ctx);   /* 0x192E */
        member_status_1049_36de();                /* 0x1931 */
        return;
    }

    /* "You [were unable to/did not] repair the broken wagon <part>.  You must
     * replace it with a spare." - take it from spares, or block on a trade. */
    g_must_trade = 0;                          /* 0x19D4 */
    spare = (part == 0) ? &g_spare_wheels
          : (part == 1) ? &g_spare_axles
          : &g_spare_tongues;
    if (*spare > 0) {
        (*spare)--;                            /* use a spare part */
    } else {
        /* no spare: record the needed part and block travel */
        /* strncpy(g_needed_item, <part name 0x16c0/0x16c8/0x16d0>, 0xa) */
        g_must_trade = 1;                      /* 0x1A06 / 0x1A37 / 0x1A68 */
    }

    if (!g_must_trade)
        member_status_1049_36de();             /* 0x1A74: had a spare, apply cost */
    else
        press_any_key_1049_15a0();             /* 0x1A7B: wait, then the trade prompt */
}

/* ---------------------------------------------------------- 0x0032:0x1b71
 * A party member breaks a limb: "<member> has a broken arm." / "...leg."
 * Records the injury on that member and starts a 30-day recovery.
 *
 * Per-member health arrays (DGROUP, 5 members each, indexed by member 0..4):
 *   g_member_ailment   0x15de  current ailment/injury code
 *   g_member_recovery  0x15e3  days remaining until recovered
 * Member names live at g_party_names (0x1589, 5 records x 11 bytes).
 */
extern char    g_party_names[5][0xb];   /* 0x1589 */
extern uint8_t g_member_ailment[5];     /* 0x15de */
extern uint8_t g_member_recovery[5];    /* 0x15e3 */

void event_broken_part(void far *ctx)
{
    int part;       /* [bp-0x2] 0 = arm, 1 = leg */
    int member;     /* [bp-0x3] chosen party member */

    part   = rand_1049_008c(2);            /* 0x1B84 arm or leg */
    member = pick_member_1049_39b9();      /* 0x1B91 */

    g_member_ailment[member]  = (uint8_t)part;   /* 0x1BA0 */
    g_member_recovery[member] = 0x1e;            /* 0x1BAB 30 days */

    /* "<member> has a broken <arm/leg>." */
    far_sprintf(/* buf, g_party_names[member] */);    /* 0x1BC8 */
    far_print(S(0x1b58) /* " has a broken " */);      /* 0x1BD2 */
    far_print(part == 0 ? S(0x1b67) /* "arm." */
                        : S(0x1b6c) /* "leg." */);    /* 0x1C08 / 0x1C3A */

    draw_msg_box_1049_3910(/* assembled msg */ 0);    /* 0x1C5D */
    member_status_1049_36de();                        /* 0x1C62: apply the injury */
    finalize_action_1049_1ee3();                      /* 0x1C67 */
}

/* ---------------------------------------------------------- 0x0032:0x14cd
 * An ox dies or is injured: "One of the oxen has died." / "...is injured."
 * Removes one ox from the team (long arithmetic on g_oxen, 0x15c4); the result
 * of that arithmetic decides the "died" vs "injured" message.
 */
extern long g_oxen;                          /* 0x15c4 */
extern long lsub_20a4_af4(long a, long b);
extern int  lcmp_20a4_b10(long a, long b);   /* <0 / 0 / >0 */

void event_ox_dies(void far *ctx)
{
    strncat_n(/* ctx msg */ 0, S(0x14a0) /* "One of the oxen " */, 0xff);  /* 0x14EE */

    /* lose an ox: g_oxen is reduced (lsub/ldiv on 0x15c4); the remainder of
     * that math selects the branch below. */
    g_oxen = lsub_20a4_af4(g_oxen, /* one team unit */ 0x80);              /* 0x1505 */

    if (/* arithmetic result == 0 -> dead (0x1536 lcmp / 0x153b jnz) */ 1)
        far_print(S(0x14b1) /* "has died." */);                            /* 0x1556 */
    else
        far_print(S(0x14bb) /* "is injured." */);                          /* 0x1588 */

    draw_msg_box_1049_3910(/* msg */ 0);
    member_status_1049_36de();
    finalize_action_1049_1ee3();
    (void)ctx;
}

/* ---------------------------------------------------------- 0x0032:0x1c72
 * Roll a random "wagon mishap" and dispatch one of three handlers: a broken
 * wagon part, an ox dying/injured, or a party member breaking a limb. The
 * branch weights come from the Borland random-long helper (0x20a4:0xb50)
 * compared (0x20a4:0xb10) against fixed thresholds; the exact probabilities
 * are left to those helpers rather than guessed here.
 */
extern long event_roll_20a4_b50(void);       /* 0x20a4:0xb50: bounded random long */

void roll_wagon_event(void far *ctx)
{
    if (lcmp_20a4_b10(event_roll_20a4_b50(), /* weight */ 0x28f5c28f) < 0)  /* 0x1C8F */
        event_broken_wagon(ctx);             /* 0x1711 */
    else if (lcmp_20a4_b10(event_roll_20a4_b50(), /* weight */ 0x80) < 0)   /* 0x1CA8 */
        event_ox_dies(ctx);                  /* 0x14CD */
    else
        event_broken_part(ctx);              /* 0x1B71 */
}

/*
 * Weather events. Both are gated by the trail region (g_location) and the
 * conditions tier g_1730 (the same state the illness model uses, here selecting
 * which weather can occur). They draw from events.pcc and apply a delay/loss.
 */
extern uint16_t g_location;                  /* 0x15ea */
extern uint16_t g_1730;                      /* conditions/severity tier        */
extern long g_rain;     /* 0x15ec: rain/mud accumulator (thunderstorm adds)     */
extern long g_snow;     /* 0x15f2: snow depth (blizzard adds; slows mileage)    */
extern void image_show_14c6_0321(const char far *n, void far *h);
extern void image_blit_14c6_03ea(const char far *data, int x, int y, int flag);
extern void image_free_14c6_043c(int a, int b);
extern void fill_rect_1049_155e(int x1, int y1, int x2, int y2);
extern long ladd_20a4_aee(long a, long b);
extern void weather_effect_32_0d35(int kind, char far *msg, void far *ctx); /* 0x0032:0x0d35 */

/* ---------------------------------------------------------- 0x0032:0x1cde
 * "Heavy fog" (mountain region, g_location > 0x0b) or "Hail storm" (plains,
 * g_location < 0x0b with a high conditions tier). Fog can cost the party time
 * (weather_effect); hail shows the scene and applies its effect.
 */
void event_weather(void far *ctx)
{
    char msg[0x1b];

    if (g_location > 0x0b && g_1730 < 5) {       /* 0x1CED: Heavy fog */
        /* strncpy(msg, "Heavy fog" 0x1cbf, 0x1b) */
        if (lcmp_20a4_b10(event_roll_20a4_b50(), /* weight */ 0x80) > 0)  /* 0x1D1A */
            weather_effect_32_0d35(1, msg, ctx); /* 0x1D2C: lose time in the fog */
        else {
            draw_msg_box_1049_3910(msg);         /* 0x1D39 */
            member_status_1049_36de();
            finalize_action_1049_1ee3();
        }
    }

    if (g_location < 0x0b && g_1730 > 4) {        /* 0x1D48: Hail storm */
        image_show_14c6_0321(S(0x1cc9) /* "events.pcc" */, ctx);     /* 0x1D64 */
        image_blit_14c6_03ea(S(0xe98) /* hail sprite */, 0, 0x94, 0);/* 0x1D80 */
        image_free_14c6_043c(0, 0);                                  /* 0x1D90 */
        draw_msg_box_1049_3910(S(0x1cd4) /* "Hail storm" */);        /* 0x1D9D */
        member_status_1049_36de();
        finalize_action_1049_1ee3();
        if (!g_quit_flag)
            fill_rect_1049_155e(0, 0x94, 0xbe, 0x11);                /* 0x1DC4 */
    }
}

/* ---------------------------------------------------------- 0x0032:0x1df8
 * "Severe blizzard" (cold tier, g_1730 < 2) or "severe thunderstorm" (warm
 * tier). Does nothing when g_1730 is 2 or 3. Each animates the weather
 * (snow/rain over the trail scene) and adds a delay to a weather resource.
 */
void event_storm(void far *ctx)
{
    char head[0x1b];     /* [bp-0x1c]   "Severe "        */
    char msg[0x102];     /* [bp-0x11e]  laid-out message */

    if (g_1730 == 2 || g_1730 == 3)              /* 0x1E07 */
        return;

    image_show_14c6_0321(S(0x1dcf) /* "events.pcc" */, ctx);   /* 0x1E26 */
    /* strncpy(head, "Severe " 0x1dda, 0x1b) */

    if (g_1730 < 2) {                            /* 0x1E3E: blizzard */
        /* msg = head + "blizzard" (0x1de2) */
        g_snow = ladd_20a4_aee(g_snow, 0x84);     /* 0x1E82: blizzard piles on snow */
        /* draw the snow over the scene (gfx 0x1ceb + blit 0xe88) */
    } else {                                     /* 0x1ED3: thunderstorm */
        /* msg = head + "thunderstorm" (0x1deb) */
        g_rain = ladd_20a4_aee(g_rain, 0x81);     /* 0x1F0D: storm adds rain/mud */
        /* animate rain frames (blit 0xea0), with thunder tones when sound is on */
    }
    /* ... show the message + member_status, then restore the scene (to 0x2024) ... */
    (void)head; (void)msg;
}

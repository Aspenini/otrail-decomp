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
extern void apply_illness_1049_3910(int member, int illness);  /* 0x1049:0x3910 */
extern void member_status_1049_36de(void);  /* 0x1049:0x36de: update member status */
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
    /* apply the injury to the chosen member and refresh status */
    apply_illness_1049_3910(member, /* injury */ 0);      /* 0x0F7C */
    member_status_1049_36de();                            /* 0x0F81 */
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

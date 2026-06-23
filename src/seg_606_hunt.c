/*
 * seg_606_hunt.c - the hunting minigame (segment 0x0606).
 *
 *   hunt @ 0x0606:0x1968  (far)
 *
 * Reached from the travel menu (option 8 "Hunt for food" on the open trail).
 * Loads the terrain and animal graphics, runs the real-time aim/shoot action,
 * converts the animals shot into pounds of meat, applies Oregon Trail's carry
 * rules, and adds the food to the wagon.
 *
 * Rules recovered from the code:
 *   - the wagon holds at most FOOD_MAX (2000) pounds of food (g_food);
 *   - you can only carry CARRY_MAX (100) pounds back from a hunt.
 *
 * Address-annotated structural reconstruction of Borland Turbo C output; the
 * meat-amount long arithmetic and the real-time shoot loop (0x1480) are named
 * and summarised rather than traced statement-by-statement. Not yet compile-
 * verified against the binary.
 */

#include <stdint.h>

#define FOOD_MAX  0x7d0   /* 2000 lb: wagon food capacity */
#define CARRY_MAX 0x64    /* 100 lb: max carried back per hunt */

extern uint8_t  g_quit_flag;   /* 0x1520 */
extern uint16_t g_food;        /* 0x15ca: wagon food supply (pounds)            */

/* Image module (segment 0x14c6). */
extern void image_show_14c6_0321(const char far *name, void far *handle); /* 0x14c6:0x0321 */
extern void image_free_14c6_043c(int lo, int hi);                         /* 0x14c6:0x043c */
extern void image_blit_14c6_03ea(int x, void far *handle);                /* 0x14c6:0x03ea */
extern void gfx_screen_init_1ceb_0b71(void);                              /* 0x1ceb:0x0b71 */
extern void gfx_sync_1ce1_0018(void);                                     /* 0x1ce1:0x0018 */

/* Hunt internals (segment 0x0606, near). */
extern void hunt_setup_terrain_606_03bf(void);  /* 0x0606:0x03bf */
extern void hunt_init_animals_606_05ae(void);   /* 0x0606:0x05ae */
extern int  hunt_shoot_606_1480(void);          /* 0x0606:0x1480 - real-time aim/shoot */

/* Text / format helpers. */
extern void strncat_n(char far *d, const char far *s, int n);  /* 0x20a4:0x064e */
extern void far_sprintf(/* ... */);                            /* 0x20a4:0x0634 */
extern void far_print(const char far *s);                      /* 0x20a4:0x06c1 */
extern void draw_text(const char far *s, int x, int y);        /* 0x1049:0x1855 */

#define S(off) ((const char far *)(off))

/* ---------------------------------------------------------- 0x0606:0x1968 */
void hunt(void)
{
    int handle_terrain;   /* [bp-...] */
    int handle_animals;
    int shot;             /* [bp-0xa] number of animals shot */
    char msg[256];        /* result message buffer */

    if (g_quit_flag) return;                              /* 0x1977 */

    gfx_screen_init_1ceb_0b71();                          /* 0x1981 */
    image_show_14c6_0321(S(0x1846) /* "terrain.pcc" */, &handle_terrain); /* 0x1990 */
    hunt_setup_terrain_606_03bf();                        /* 0x1996 */
    if (g_quit_flag) { image_free_14c6_043c(0, 0); return; }  /* 0x1999 */

    hunt_init_animals_606_05ae();                         /* 0x19B1 */
    image_show_14c6_0321(S(0x1852) /* "animals.pcc" */, &handle_animals); /* 0x19C6 */

    gfx_sync_1ce1_0018();                                 /* 0x19D6 */
    shot = hunt_shoot_606_1480();                         /* 0x19DC: aim & shoot */
    gfx_sync_1ce1_0018();                                 /* 0x19EA */
    image_free_14c6_043c(0, 0);   /* animals  */          /* 0x19F7 */
    image_free_14c6_043c(0, 0);   /* terrain  */          /* 0x1A04 */
    if (g_quit_flag) return;                              /* 0x1A09 */

    /* shot >= 3 caps the meat via long arithmetic (pounds-per-kill scaling). */
    if (shot >= 3) { /* meat = lcap(...) */ }             /* 0x1A13 */

    if (shot == 0) {
        strncat_n(msg, S(0x185e) /* "You were unable to shoot any food." */, 0); /* 0x1A3C */
    } else {
        /* "From the animals you shot, you got <N> pound[s] of meat.  " */
        if (shot != 1)
            strncat_n(msg, S(0x1881) /* "s" */, 0);       /* 0x1A78 */
        far_sprintf(/* msg, cs:0x1883 "From the animals you shot, you got ", N */); /* 0x1A92 */
        far_print(S(0x18a7) /* " pound" */);              /* 0x1AA7 */
        far_print(S(0x18ae) /* " of meat.  " */);         /* 0x1ABC */

        /* Apply the carry rules. */
        if (g_food >= FOOD_MAX) {                         /* 0x1AD5 */
            far_print(S(0x18ba) /* "However, your wagon is full." */);            /* 0x1AEE */
        } else if (g_food + shot > FOOD_MAX) {            /* 0x1B15 */
            /* "However, your wagon will only hold another <K> pounds of food." */
            far_print(S(0x18d7));                         /* 0x1B53 */
            far_print(S(0x1903) /* " pounds of food." */);/* 0x1B68 */
        } else if (shot > CARRY_MAX) {                    /* 0x1B24/0x1B81 */
            /* "However, you were only able to carry 100 pounds back to the wagon." */
            far_print(S(0x1914));                         /* 0x1B9D */
            far_print(S(0x193e) /* "pounds back to the wagon." */);              /* 0x1BA7 */
        }
    }

    /* Show the result over the supplies screen and add the meat to the wagon. */
    gfx_screen_init_1ceb_0b71();                          /* 0x1BC0 */
    image_show_14c6_0321(S(0x1958) /* "supplies.pcc" */, /* handle */ 0);        /* 0x1BCF */
    image_blit_14c6_03ea(0xdc0, /* handle */ 0);          /* 0x1BED */
    image_free_14c6_043c(0, 0);                           /* 0x1BFA */
    draw_text(msg, 0, 0);                                 /* 0x1C0F.. */
    /* (food total redrawn; wait for key) */              /* 0x1C45..0x1C75 */
}                                                         /* 0x1C7D retf */

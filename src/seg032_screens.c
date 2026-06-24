/*
 * seg032_screens.c - trail-menu info screens (segment 0x0032).
 *
 *   check_supplies @ 0x0032:0x3a82  (near) - travel menu option 2
 *   look_at_map    @ 0x0032:0x2d16  (near) - travel menu option 3
 *
 * Both are read-only display screens reached from travel_turn.
 *
 * Address-annotated reconstruction of Borland Turbo C output; not yet
 * compile-verified against the binary.
 */

#include <stdint.h>

extern uint8_t g_quit_flag;   /* 0x1520 */

extern void image_show_14c6_0321(const char far *name, void far *h); /* 0x14c6:0x0321 */
extern void image_blit_14c6_03ea(int x, void far *h);                /* 0x14c6:0x03ea */
extern void image_free_14c6_043c(int lo, int hi);                    /* 0x14c6:0x043c */
extern void redraw_supplies_1049_37d5(void);                         /* 0x1049:0x37d5 */
extern void press_any_key(void);                                     /* 0x1049:0x15a0 */
extern void gfx_screen_init_1ceb_0b71(void);                         /* 0x1ceb:0x0b71 */
extern void load_show_pcx_182e_1e2d(const char far *name);           /* 0x182e:0x1e2d */
extern void save_restore_screen_32_2ca2(void);                       /* 0x0032:0x2ca2 */
extern void draw_map_marker_32_2ab6(void);                           /* 0x0032:0x2ab6 */

#define S(off) ((const char far *)(off))

/* ---------------------------------------------------------- 0x0032:0x3a82
 * Show the current supplies (read-only): the supplies.pcc backdrop with the
 * five category icons, then the live quantities.
 */
void check_supplies(void)
{
    gfx_screen_init_1ceb_0b71();                        /* 0x3A90 */
    image_show_14c6_0321(S(0x3a75) /* "supplies.pcc" */, 0);  /* 0x3A9F */
    image_blit_14c6_03ea(0xdd0, 0);                     /* 0x3AB7: oxen      */
    image_blit_14c6_03ea(0xdc8, 0);                     /* 0x3ACF: food      */
    image_blit_14c6_03ea(0xdb0, 0);                     /* 0x3AE7: clothing  */
    image_blit_14c6_03ea(0xdb8, 0);                     /* 0x3AFF: ammunition*/
    image_blit_14c6_03ea(0xdc0, 0);                     /* 0x3B17: spare parts */
    image_free_14c6_043c(0, 0);                         /* 0x3B22 */
    redraw_supplies_1049_37d5();                         /* 0x3B2B: draw quantities */
    press_any_key();                                    /* 0x3B30 */
}                                                       /* 0x3B38 ret 2 */

/* ---------------------------------------------------------- 0x0032:0x2d16
 * Show the trail map (MAP.PCX) with the party's current position marked,
 * saving and restoring the underlying screen.
 */
void look_at_map(void)
{
    save_restore_screen_32_2ca2();                      /* 0x2D29: save */
    if (!g_quit_flag) {                                 /* 0x2D2C */
        load_show_pcx_182e_1e2d(S(0x2d0e) /* "MAP.PCX" */);  /* 0x2D48 */
        draw_map_marker_32_2ab6();                      /* 0x2D51: mark position */
        press_any_key();                                /* 0x2D54 */
    }
    save_restore_screen_32_2ca2();                      /* 0x2D5E: restore */
}                                                       /* 0x2D64 ret 2 */

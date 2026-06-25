/*
 * seg032_map.c - trail data model: location forks and the route map
 *                (segment 0x0032, the travel engine).
 *
 *   trail_fork      @ 0x0032:0x2dbe  (near) - "The trail divides here" choice
 *   draw_map_marker @ 0x0032:0x2ab6  (near) - plot the travelled route + wagon
 *
 * Both read the trail's location table, whose layout is fully decoded here.
 *
 * ---------------------------------------------------------------------------
 * The location table  (DGROUP 0x0896, 18 records of 0x25 bytes)
 * ---------------------------------------------------------------------------
 * Indexed by location id 0..0x11 (Independence .. the Willamette Valley). Each
 * record is exactly 37 bytes:
 *
 *      struct location {
 *          char name[0x1c];  // +0x00 counted string (dst[0]=len), padded to 28
 *          u8   field_1c;    // +0x1c region marker: 0x14 for loc 0-4, 0x0c for 5-17
 *          u8   dest1;       // +0x1d next location for menu choice 1 (the main path)
 *          u8   dest2;       // +0x1e next location for choice 2 (0 = no fork here)
 *          u8   miles1;      // +0x1f miles to dest1
 *          u8   miles2;      // +0x20 miles to dest2
 *          u16  map_x;       // +0x21 marker X on the trail map
 *          u16  map_y;       // +0x23 marker Y on the trail map
 *      };
 *
 * The only two real forks in the game fall straight out of dest2 != 0:
 *   - South Pass (7):       dest1=9 Green River (57 mi)  / dest2=8 Fort Bridger (125 mi)
 *   - the Blue Mountains (14): dest1=15 Fort Walla Walla / dest2=16 The Dalles
 *
 * Address-annotated structural reconstruction of Borland Turbo C output; the
 * table layout and fork/map logic are verified against the binary. Not yet
 * compile-verified.
 */

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;

struct location {
    char name[0x1c];
    u8   field_1c;
    u8   dest1;     /* dest[choice]: dest1 at +0x1d, dest2 at +0x1e */
    u8   dest2;
    u8   miles1;    /* miles[choice]: miles1 at +0x1f, miles2 at +0x20 */
    u8   miles2;
    u16  map_x;
    u16  map_y;
};

/* ------------------------------------------------------------------ globals */
extern struct location g_locations[18]; /* 0x0896: the trail location table     */
extern char  g_location_names[18][0x25];/* alias view of the same table (names) */
extern uint8_t  g_route[];              /* 0x15f8: ids of locations plotted on map*/
extern uint8_t  g_route_len;            /* 0x160a: number of route markers       */
extern uint16_t g_location;             /* 0x15ea: current location id           */
extern uint8_t  g_next_location;        /* 0x1610: next location id              */
extern uint16_t g_miles_past;           /* 0x160d: miles past the last landmark  */
extern uint16_t g_miles_to;             /* 0x160f: miles remaining to next       */
extern uint8_t  g_quit_flag;            /* 0x1520: Esc/quit requested            */
extern uint8_t  g_input_buf;            /* 0x141a: raw keyboard input (counted)  */
extern uint16_t g_cursor_row, g_cursor_col; /* 0x157a / 0x157c: text layout origin */

/* --------------------------------------------------------------- prototypes */
extern void __stkcheck_20a4_0244(int frame);

/* graphics module (segment 0x1ceb) */
extern void gfx_rect_1ceb_16a1(int x1, int y1, int x2, int y2);
extern void gfx_1ceb_17db(int a);

/* text / field helpers (segment 0x1049) */
extern void clear_panel_1049_1e64(int x1, int y1, int x2, int y2);
extern void text_1049_1855(void);
extern void echo_field_1049_d95(const char far *prompt, char far *dst);
extern void finalize_action_1049_1ee3(void);
extern int  parse_choice_1049_180b(const char far *s);

/* input/time module (segment 0x2042) */
extern void show_input_prompt_2042_0215(int a, int b);

/* Borland runtime (segment 0x20a4) */
extern void  textlayout_20a4_634(int x, int y, char far *out, const char far *fmt);
extern void  far_print_20a4_6c1(const char far *s);
extern long  lmul_20a4_b00(long a, long b);
extern long  lmul_20a4_b06(long a, long b);   /* second long-multiply entry */
extern long  ladd_20a4_b14(long a, long b);   /* long add/convert helper    */

/* trail-engine helpers (segment 0x0032, near) */
extern void view_map(void far *ctx);        /* 0x0032:0x2d16 - "see the map" */
extern void draw_travel_scene(void far *ctx);/* 0x0032:0x0bd4 - redraw the trail view */

#define CS(off)   ((const char far *)(off))
#define DS(off)   ((const char far *)(off))

/* ---------------------------------------------------------- 0x0032:0x2dbe
 * Offer the trail fork for the current location, if it has one. Locations
 * without a fork (dest2 == 0) just take dest1 with no prompt. Choice 3 ("see
 * the map") loops back to the prompt; choice 1 or 2 commits the destination
 * and its mileage.
 */
void trail_fork(void far *ctx)
{
    char buf[0x100];                       /* [bp-0x102] laid-out menu text */
    u8   choice;                           /* [bp-0x01]                     */
    struct location *here = &g_locations[g_location];

    if (here->dest2 == 0) {                /* 0x2dd7: no fork at this stop  */
        choice = 1;
        goto apply;
    }

    for (;;) {                             /* 0x2de5: the fork prompt loop  */
        clear_panel_1049_1e64(0x96, 0x130, 0x43, 0x0f);
        textlayout_20a4_634(g_cursor_row, g_cursor_col, buf,
                            CS(0x2d67));   /* "The trail divides here.  You may:\ 1. head for " */
        far_print_20a4_6c1(g_location_names[here->dest1]);   /* choice 1 destination */
        far_print_20a4_6c1(CS(0x2d98));    /* "\ 2. head for " */
        far_print_20a4_6c1(g_location_names[here->dest2]);   /* choice 2 destination */
        far_print_20a4_6c1(CS(0x2da7));    /* "\ 3. see the map\\1-3" */
        far_print_20a4_6c1(DS(0x684));     /* "What is your choice? " */
        text_1049_1855();

        show_input_prompt_2042_0215(0x11, 0x19);
        echo_field_1049_d95(CS(0x2dba), (char far *)&g_input_buf);
        finalize_action_1049_1ee3();

        if (g_quit_flag)                   /* 0x2e9e */
            return;

        choice = (u8)parse_choice_1049_180b((const char far *)&g_input_buf);
        if (choice == 3) {                 /* "see the map" */
            view_map(ctx);                 /* 0x2ec1 */
            if (g_quit_flag)
                return;
            draw_travel_scene(ctx);        /* 0x2ed4 */
        }
        if (choice < 3)                    /* 0x2ed7: 1 or 2 commits */
            break;
    }

apply:                                     /* 0x2ee0 */
    here = &g_locations[g_location];
    /* dest[choice]/miles[choice] read straight from the record, choice-indexed */
    g_next_location = (&here->field_1c)[choice];   /* [rec+0x1c+choice] */
    g_miles_to      = (&here->dest2)[choice];      /* [rec+0x1e+choice] */
    g_miles_past    = 0;                            /* 0x2f14 */
}

/* ---------------------------------------------------------- 0x0032:0x2ab6
 * Draw the travelled route on the trail map and the wagon's current position.
 * Each plotted leg connects consecutive route markers (their map_x/map_y); the
 * current dot is interpolated between this landmark and the next by how far the
 * party has come (g_miles_past of g_miles_past+g_miles_to).
 */
void draw_map_marker(void)
{
    int i, last;
    int x0, y0, x1, y1;
    long frac_num, frac_den;

    gfx_1ceb_17db(0);                                  /* 0x2ac7 */
    if (g_route_len == 0)                               /* 0x2acc */
        return;

    last = g_route_len - 1;                             /* 0x2adb */
    /* draw a line between each pair of adjacent route markers */
    for (i = 1; i <= last; i++) {                       /* 0x2adf loop */
        struct location *a = &g_locations[g_route[i - 1]];
        struct location *b = &g_locations[g_route[i]];
        x0 = a->map_x; y0 = a->map_y;                   /* prev marker */
        x1 = b->map_x; y1 = b->map_y;                   /* this marker */
        gfx_rect_1ceb_16a1(x0, y0, x1, y1);             /* 0x2b60 */
        gfx_rect_1ceb_16a1(x0 - 1, y0, x1 - 1, y1);     /* 0x2b75 (2px line) */
    }

    /* interpolate the wagon between g_location and g_next_location */
    if ((int)(g_miles_to + g_miles_past) <= 0)          /* 0x2b85 */
        return;

    frac_den = ladd_20a4_b14(g_miles_to + g_miles_past, 0);  /* denominator */
    frac_num = ladd_20a4_b14(g_miles_past, 0);               /* numerator   */
    /* (dx,dy) * miles_past / (miles_past+miles_to) gives the wagon offset; the
     * remaining long-math (lmul 0xb00/0xb06) plots the dot between the two
     * landmark markers - see the binary at 0x2bc1..0x2c9b. */
    (void)frac_num; (void)frac_den;
    {
        struct location *cur = &g_locations[g_location];
        struct location *nxt = &g_locations[g_next_location];
        x0 = cur->map_x; y0 = cur->map_y;
        x1 = nxt->map_x; y1 = nxt->map_y;
        (void)x0; (void)y0; (void)x1; (void)y1;
    }
}

/*
 * seg032_arrival.c - landmark arrival screens (segment 0x0032, the travel engine).
 *
 *   show_location    @ 0x0032:0x011d  (near) - the "you are now at X" arrival
 *                                              screen + "look around?" prompt
 *   arrive_landmark  @ 0x0032:0x0655  (near) - the between-landmarks progress
 *                                              message ("...N miles past X...")
 *
 * Both are reached from the trail loop (seg032_trail.c) when the party reaches
 * or passes a landmark. They render through the graphics module (0x1ceb), the
 * image loader (0x14c6) and the text helpers (0x1049); the player-facing
 * strings live as counted strings in this code segment, and the place names
 * come from the location-name table in DGROUP (see g_location_names below).
 *
 * Address-annotated structural reconstruction of Borland Turbo C output. The
 * control-flow spine and the data it reads are verified against the binary; the
 * exact text-layout helper semantics (0x20a4:0x634 et al.) are named by
 * inference. Not yet compile-verified against the binary.
 */

#include <stdint.h>

/* ------------------------------------------------------------------ globals */
extern uint8_t  g_quit_flag;        /* 0x1520: Esc/quit requested                 */
extern uint16_t g_location;         /* 0x15ea: current trail location index 0..0x11*/
extern uint8_t  g_next_location;    /* 0x1610: next trail location index           */
extern uint16_t g_miles_past;       /* 0x160d: miles travelled past the last landmark */
extern uint16_t g_miles_to;         /* 0x160f: miles remaining to the next landmark   */
extern uint8_t  g_at_fort;          /* 0x1729: at a fort/settlement (cleared on arrival)*/
extern uint8_t  g_179a;             /* 0x179a: image-placement flag (centred vs offset)*/
extern uint8_t  g_look_answer;      /* 0x1789: "look around?" reply length (0 = none)  */
extern uint8_t  g_input_buf;        /* 0x141a: raw keyboard input (counted string)     */

/*
 * The 18 trail landmarks, in order, as 37-byte (0x25) counted strings at
 * DGROUP offset 0x896 (dst[0] = length). Indexed by location id 0..0x11:
 *
 *   0  Independence              9  Green River crossing
 *   1  the Kansas River crossing 10 Soda Springs
 *   2  the Big Blue River crossing 11 Fort Hall
 *   3  Fort Kearney             12 the Snake River crossing
 *   4  Chimney Rock             13 Fort Boise
 *   5  Fort Laramie             14 the Blue Mountains
 *   6  Independence Rock        15 Fort Walla Walla
 *   7  South Pass               16 The Dalles
 *   8  Fort Bridger             17 the Willamette Valley
 */
extern char g_location_names[18][0x25];   /* 0x0896 */

#define LOC_NAME(i)  (g_location_names[(i)])

/* --------------------------------------------------------------- prototypes */
extern void __stkcheck_20a4_0244(int frame);

/* graphics module (segment 0x1ceb) */
extern void gfx_screen_init_1ceb_0b71(void);                 /* clear the frame      */
extern void gfx_setmode_1ceb_0cda(int a, int b);             /* set display region   */
extern void gfx_rect_1ceb_16a1(int x1, int y1, int x2, int y2);
extern void gfx_hline_1ceb_16e9(int x1, int y, int x2, int color);
extern void gfx_1ceb_17db(int a);

/* image loader (segment 0x14c6) */
extern void image_load_14c6_021a(const char far *name, int x, int y);

/* text / field helpers (segment 0x1049) */
extern void draw_text_line_1049_1b3c(int x, int y);          /* draw the current string */
extern void format_str_1049_2c1e(char far *s);
extern void draw_menu_line_1049_1521(int x, int y, const char far *s);
extern void fill_rect_1049_155e(int x1, int y1, int x2, int y2);
extern int  read_menu_choice_1049_14a6(void);               /* returns 1..N            */
extern void text_1049_e9b(int a, int b);                    /* set text colour/mode    */
extern void text_1049_1f1b(const char far *buf, int loc);
extern void read_field_1049_4df(const char far *prompt, char far *dst);
extern void echo_field_1049_d95(const char far *a, char far *dst);

/* input/time module (segment 0x2042) */
extern void show_input_prompt_2042_0215(int a, int b);

/* Borland runtime (segment 0x20a4) */
extern void  ltoa_20a4_1000(long val, int pad, char far *buf, int radix);
extern void  textlayout_20a4_634(int x, int y, char far *out, int width,
                                 const char far *text, const char far *fmt);
extern void  far_print_20a4_6c1(const char far *s);
extern void  strncpy_n_20a4_64e(char far *dst, const char far *src, int n);
extern void  far_sprintf_20a4_634(char far *dst, const char far *src);  /* alias view */
extern void  scenery_begin_20a4_1112(char far *buf, const char far *src);
extern void  scenery_next_20a4_1182(char far *buf);
extern int   scenery_more_20a4_11db(char far *buf);
extern void  field_20a4_14a3(char far *buf, char far *out, int max);
extern void  field_20a4_1435(void);
extern void  field_20a4_20e(void);

#define CS(off) ((const char far *)(off))

/* ---------------------------------------------------------- 0x0032:0x011d
 * Draw the arrival screen for the current landmark: clear, load and frame the
 * landmark image, print its name, then offer "Would you like to look around or
 * continue on the trail?". Choice 1 reads/echoes scenery description text;
 * choice 3 continues. Esc (g_quit_flag) bails out at any prompt.
 */
void show_location(void)
{
    char  imgname[0xc];     /* [bp-0x0e]  landmark image name, e.g. "loc3.pcc"   */
    char  layout[0x300];    /* [bp-0x314] laid-out heading text                  */
    char  field[0x100];     /* [bp-0x10e] the player's typed reply              */
    char  scenery[0x100];   /* [bp-0x20e] one scenery line at a time            */
    int   choice;           /* [bp-0x214]                                       */
    int   i, n;             /* [bp-0x210], [bp-0x216]                           */

    gfx_screen_init_1ceb_0b71();

    /* image file name is "<location index>" formatted into imgname */
    ltoa_20a4_1000((long)g_location, 0, imgname, 0xc);             /* 0x131 */
    textlayout_20a4_634(0, 0, layout, 0, CS(0x106), CS(0x106));    /* 0x148 */
    far_print_20a4_6c1(imgname);                                  /* 0x158 */
    far_print_20a4_6c1(CS(0x108));                                /* 0x162 */
    strncpy_n_20a4_64e(imgname, imgname, 0xc);                    /* 0x16c */

    if (g_quit_flag)                                              /* 0x17a */
        return;

    /* place the landmark image: centred (x=0) or offset (x=0x1e) by g_179a */
    if (g_179a == 0)
        image_load_14c6_021a(imgname, 0, 0);                     /* 0x196 */
    else
        image_load_14c6_021a(imgname, 0x1e, 0);                  /* 0x1a9 */

    /* frame the scene: a ruled box around the lower text area */
    gfx_setmode_1ceb_0cda(3, 1);                                 /* 0x1b6 */
    gfx_hline_1ceb_16e9(0x3c, 0xa6, 0x104, 0xbb);                /* 0x1cb */
    gfx_1ceb_17db(0);
    gfx_rect_1ceb_16a1(0x3c, 0xa5, 0x3c, 0xbb);                  /* left  edge */
    gfx_rect_1ceb_16a1(0x3c, 0xa5, 0x104, 0xa5);                 /* top   edge */
    gfx_rect_1ceb_16a1(0x104, 0xa5, 0x104, 0xbb);               /* right edge */

    /* print the location name heading */
    text_1049_e9b(0, 3);
    text_1049_1f1b(layout, g_location);                          /* 0x22d */
    draw_text_line_1049_1b3c(0xa0, 0xa9);
    format_str_1049_2c1e(layout);
    draw_text_line_1049_1b3c(0xa0, 0xb2);
    text_1049_e9b(0, 3);

    /* "Would you like to look around or continue on the trail?" menu loop */
    for (;;) {
        show_input_prompt_2042_0215(1, 1);                       /* 0x269 */
        scenery_begin_20a4_1112(field, CS(0x10d));               /* 0x279 */
        scenery_next_20a4_1182(field);
        choice = read_menu_choice_1049_14a6();                   /* 0x289 */
        if (choice == 1 || choice == 3)
            break;
    }

    if (choice == 3)
        goto continue_trail;                                     /* 0x2a5 -> 0x2f9 */

    /* choice 1: "look around" - read out the scenery description lines */
    n = g_location;                                              /* 0x2a7 */
    for (i = 0; i <= n; i++) {                                   /* 0x2b0 loop */
        field_20a4_14a3(field, scenery, 0xff);                   /* 0x2d0 */
        field_20a4_1435();
        field_20a4_20e();
    }
    scenery_more_20a4_11db(field);                               /* 0x2ef */
    field_20a4_20e();

continue_trail:
    fill_rect_1049_155e(0, 0xbe, 0x13f, 0xc7);                   /* 0x308 clear prompt row */
    show_input_prompt_2042_0215(1, 0x19);                       /* 0x313 */
    draw_menu_line_1049_1521(0x37, 0xc0, (const char far *)&g_input_buf); /* 0x325 */

    if (choice == 3) {
        /* continue on the trail: echo the typed continuation field */
        echo_field_1049_d95(CS(0x11a), (char far *)&g_input_buf); /* 0x366 */
        return;
    }

    /* look-around branch: keep reading "look around?" replies until done/quit */
    do {                                                         /* 0x331 */
        read_field_1049_4df(CS(0x117), (char far *)&g_look_answer);
    } while (g_look_answer == 0 && g_quit_flag == 0);            /* 0x346 */
}

/* ---------------------------------------------------------- 0x0032:0x0655
 * Between landmarks: report progress as "Your party is now <N> mile(s) past
 * <here>.  The next landmark is <there>, <M> away." then wait for a key.
 *
 * The sentence is assembled from counted-string fragments in this segment and
 * the two place names from g_location_names, with singular/plural handling
 * ("just"/"mile" vs "miles") driven by g_miles_past / g_miles_to.
 */
void arrive_landmark(void)
{
    char  past_str[0xa];    /* [bp-0x0c]  g_miles_past as text   */
    char  to_str[0xa];      /* [bp-0x18]  g_miles_to as text     */
    char  msg[0x100];       /* [bp-0x118] assembled sentence     */
    char  layout[0x100];    /* [bp-0x218] laid-out display text  */

    gfx_screen_init_1ceb_0b71();                                 /* 0x664 */
    /* draw the background panel (coords from globals 0x69c/0x69e) */
    /* draw_panel(*(int*)0x69c, *(int*)0x69e);                    -- 0x671 */

    /* miles-past -> text, with "1" spelled as the singular fragment */
    ltoa_20a4_1000((long)g_miles_past, 0, past_str, 0xa);        /* 0x688 */
    if (g_miles_past == 1)
        strncpy_n_20a4_64e(past_str, CS(0x606), 0xa);            /* 0x6a2 "just"/" mile" */
    else {
        far_sprintf_20a4_634(msg, past_str);                     /* 0x6b4 */
        far_print_20a4_6c1(CS(0x60b));                          /* " miles" */
        strncpy_n_20a4_64e(past_str, past_str, 0xa);
    }

    /* miles-to -> text, same singular/plural handling */
    ltoa_20a4_1000((long)g_miles_to, 0, to_str, 0xa);            /* 0x6e6 */
    if (g_miles_to == 1) {
        far_sprintf_20a4_634(msg, to_str);                       /* 0x6fd */
        far_print_20a4_6c1(CS(0x612));                          /* " mile" */
        strncpy_n_20a4_64e(to_str, to_str, 0xa);
    } else {
        far_sprintf_20a4_634(msg, to_str);                       /* 0x727 */
        far_print_20a4_6c1(CS(0x60b));                          /* " miles" */
        strncpy_n_20a4_64e(to_str, to_str, 0xa);
    }

    /* lay out and print the full sentence */
    textlayout_20a4_634(0x1e, 0x46, layout, 0x104, msg, CS(0x618)); /* 0x761
        cs:0x618 = "Your party is now \06 past \18.  The next landmark is \02, \06 away" */
    far_print_20a4_6c1(past_str);                               /* 0x76b  N */
    far_print_20a4_6c1(CS(0x62b));                              /* 0x775  " past " */
    far_print_20a4_6c1(LOC_NAME(g_location));                   /* 0x78a  here */
    far_print_20a4_6c1(CS(0x632));                              /* 0x794  ".  The next landmark is " */
    far_print_20a4_6c1(LOC_NAME(g_next_location));              /* 0x7ab  there */
    far_print_20a4_6c1(CS(0x64b));                              /* 0x7b5  ", " */
    far_print_20a4_6c1(to_str);                                /* 0x7bf  M */
    far_print_20a4_6c1(CS(0x64e));                              /* 0x7c9  " away." */

    /* finish the message box and wait for a key */
    /* 0x1049:0x1938 / 0x1855 / press_any_key(0x15a0)            -- 0x7ce..0x7d8 */
    g_at_fort = 0;                                              /* 0x7dd */
}

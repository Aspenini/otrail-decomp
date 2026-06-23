/*
 * seg1049_text.c - graphical text-rendering helpers (code segment 0x1049).
 *
 * The Oregon Trail runs in a graphics mode and draws text as bitmaps, so there
 * is no BIOS text cursor; instead a pixel cursor is kept in two DGROUP words and
 * the low-level glyph blitting lives in the graphics module (segment 0x150c).
 *
 *   gotoxy        @ 0x1049:0x0e60  (far, retf 4)
 *   print_string  @ 0x1049:0x0ebd  (far)
 *   draw_text     @ 0x1049:0x1855  (far, retf 8) - multi-line, '\'-separated
 *
 * Recovered from build/OREGON_unpacked.exe. Address-annotated reconstruction of
 * Borland Turbo C output; the '\'-line splitting in draw_text uses the runtime
 * string library (count/extract/delete-field) and is rendered structurally.
 * Not yet compile-verified against the binary.
 */

#include <stdint.h>

#define LINE_DELIM '\\'   /* cs:0x1853 - the game uses '\' as the newline char */
#define LINE_DY    9      /* pixel rows between successive text lines           */

/* ------------------------------------------------------------------ globals */
extern uint16_t g_text_x;   /* 0x1792: text cursor X (pixels)                  */
extern uint16_t g_text_y;   /* 0x1794: text cursor Y (pixels)                  */
extern uint16_t g_157a;     /* 0x157a: last draw_text end X (saved cursor)     */
extern uint16_t g_157c;     /* 0x157c: last draw_text end Y                    */
extern uint16_t g_157e;     /* 0x157e: scratch position passed to gfx helper   */
extern uint16_t g_1580;     /* 0x1580                                          */

/* --------------------------------------------------------------- prototypes */
extern void __stkcheck_20a4_0244(int frame);
extern void strncpy_20a4_064e(char far *dst, const char far *src, int n);

/* Runtime string-field helpers (Borland library, segment 0x20a4). */
extern int  str_count_fields_20a4_06ed(const char far *s, const char far *delim); /* 0x6ed */
extern void str_get_field_20a4_0680(const char far *s, char far *out, int first, int count); /* 0x680 */
extern void str_del_fields_20a4_07d0(char far *s, int first, int count);          /* 0x7d0 */

/* Graphics module (segment 0x150c) - glyph blitting. */
extern int  gfx_draw_text_150c_301f(const char far *s, int x, int y);  /* 0x150c:0x301f */
extern int  gfx_150c_09ab(int a);                                      /* 0x150c:0x09ab */

extern int  strlen_1049_0057(const char far *s);   /* 0x1049:0x0057 */
extern void sub_1049_0e3e(/* g_157e, g_1580 */);   /* 0x1049:0x0e3e */

/* ---------------------------------------------------------- 0x1049:0x0e60 */
void gotoxy(int x, int y)
{
    g_text_x = x;   /* [bp+0x8] -> 0x1792 */
    g_text_y = y;   /* [bp+0x6] -> 0x1794 */
}

/* ---------------------------------------------------------- 0x1049:0x0ebd
 * Draw one string at the current pixel cursor and advance X by 8 px/char.
 */
void print_string(const char far *s)
{
    char buf[256];                          /* [bp-0x100] */
    strncpy_20a4_064e(buf, s, 0xFF);        /* 0x0EDB */
    gfx_draw_text_150c_301f(buf, g_text_x, g_text_y + 6);  /* 0x0EF1 */
    g_text_x += strlen_1049_0057(buf) << 3; /* 0x0F00: advance 8 px per char  */
}

/* ---------------------------------------------------------- 0x1049:0x1855
 * Render a '\'-separated multi-line string starting at (x, y); each subsequent
 * line is drawn LINE_DY pixels lower. The final cursor is saved to g_157a/c.
 */
void draw_text(const char far *str, int x, int y)
{
    char buf[256];     /* [bp-0x100]  working copy of the whole string */
    char line[256];    /* [bp-0x200]  one extracted line               */
    char field[256];   /* [bp-0x302]  field-extract scratch            */
    int  nlines;       /* [bp-0x202]                                   */

    strncpy_20a4_064e(buf, str, 0xFF);                  /* 0x1873 */
    gotoxy(x, y);                                        /* 0x187F */
    nlines = str_count_fields_20a4_06ed(buf, /* cs:0x1853 "\" */ 0);  /* 0x188D */

    if (nlines == 0) {                                  /* 0x1896: single line */
        print_string(buf);                              /* 0x18A4 */
    } else {
        /* Pull off the first line, print it, delete it, drop down a row;
         * repeat until the string is exhausted. */
        while (nlines >= 1) {                            /* 0x18A9 loop */
            str_get_field_20a4_0680(buf, field, 1, nlines - 1);  /* 0x18C6 */
            strncpy_20a4_064e(line, field, 0xFF);                /* 0x18D5 */
            print_string(line);                                  /* 0x18E1 */
            str_del_fields_20a4_07d0(buf, 1, nlines);            /* 0x18F2 */
            y += LINE_DY;                                        /* 0x18F7: add 9 */
            sub_1049_0e3e(/* g_157e, g_1580 */);                 /* 0x190B */
            gotoxy(x, y);                                        /* 0x1915 */
            nlines = (buf[0] != 0) ? nlines : 0;                 /* 0x1918: loop while non-empty */
        }
    }

    g_157a = x;   /* 0x1922: remember where text ended */
    g_157c = y;   /* 0x1928 */
}

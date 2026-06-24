/*
 * seg_150c_gfxtext.c - graphics-module text rendering (segment 0x150c).
 *
 *   draw_string @ 0x150c:0x301f  (far) - draw a string, with alignment
 *   draw_glyph  @ 0x150c:0x164d  (far) - draw one character's bitmap
 *
 * These are the low-level routines behind the higher-level text helpers in
 * segment 0x1049 (print_string / draw_text). They consume the proportional
 * bitmap font loaded from BIT8X8.GFT and write into the 320x200 video buffer.
 * The DGROUP data segment is 0x2231.
 *
 * Recovered from build/OREGON_unpacked.exe. These are hand-written graphics
 * primitives (no Borland stack frame); the lift is structural and focuses on
 * the algorithm and the recovered font-descriptor layout. The final per-pixel
 * blit is dispatched through a function-pointer vtable (jmp far [drv+0x24]).
 */

#include <stdint.h>

/* Font descriptor as accessed by these routines (offsets are exact). The font
 * is loaded from BIT8X8.GFT; see port/assets/FORMATS.md. */
typedef struct Font {
    /* 0x02,0x04 */ uint16_t locator[2]; /* id passed to the font-data locator   */
    /* 0x06 */      uint16_t align;       /* hi=h-align (2=center,4=right), lo=v   */
    /* ...  */
    /* 0x24 */      uint16_t first_char;  /* lowest character code present         */
    /* 0x26 */      uint16_t last_char;   /* highest character code present        */
    /* 0x2a */      uint16_t valign_off;
    /* 0x2e */      uint16_t descent;
    /* 0x44 */      uint16_t *width_tab;  /* per-glyph advance width (u16/glyph)    */
    /* 0x48 */      uint16_t *off_tab;    /* per-glyph bitmap offset (u16/glyph)    */
    /* 0x4c */      uint8_t  *glyph_data; /* base of glyph bitmap data              */
    /* 0x52 */      uint16_t height;      /* font cell height                       */
} Font;

extern Font *font_lookup_150c_019d(int id);       /* 0x150c:0x019d */
extern int   font_select_150c_09f7(void);         /* 0x150c:0x09f7 */
extern void *font_data_150c_0ea4(int a, int b);   /* 0x150c:0x0ea4: locate glyph data */
extern int   blit_setup_150c_0043(void);          /* 0x150c:0x0043 */
extern void *blit_ctx_150c_010a(int mode);        /* 0x150c:0x010a: get a blit driver */

/* DGROUP work buffer the offset table is copied into. */
extern uint16_t g_font_offtab_18ae[256];   /* 0x2231:0x18ae */

/* ---------------------------------------------------------- 0x150c:0x301f
 * Draw a counted string `s` at (x, y) in the current font. Computes the total
 * pixel width first (for centre/right alignment), then draws each glyph,
 * advancing x by each glyph's width plus inter-character spacing.
 * Returns 0 on success (negative error code otherwise).
 */
int draw_string(const char far *s, int x, int y)
{
    Font *f;
    int   width = 0;
    int   i, n;
    int   fixed_w;     /* nonzero -> fixed-width font (skip per-glyph table) */

    if ((f = font_lookup_150c_019d(font_select_150c_09f7())) == 0)
        return -1;
    /* The font's per-glyph offset table is copied into g_font_offtab_18ae. */
    fixed_w = /* (f->flags & 0x10) ? f->fixed_width : 0 */ 0;

    /* --- pass 1: total width (for alignment) --- */
    n = s[-1];                                     /* counted-string length */
    for (i = 0; i < n; i++) {
        unsigned c = (unsigned char)s[i];
        if (c < f->first_char || c > f->last_char) continue;
        if (!fixed_w)
            width += g_font_offtab_18ae[c - f->first_char] + /* spacing */ 0;
    }

    /* --- alignment: adjust the start (x,y) from f->align --- */
    /* hi byte: 2 = centre (x -= width/2), 4 = right (x -= width).
     * lo byte: vertical alignment using f->height / f->valign_off. */

    /* --- pass 2: draw each glyph and advance --- */
    for (i = 0; i < n; i++) {
        unsigned c = (unsigned char)s[i];
        if (c < f->first_char || c > f->last_char) continue;
        draw_glyph(c, x, y);                        /* 0x150c:0x164d */
        if (!fixed_w)
            x += g_font_offtab_18ae[c - f->first_char] + /* spacing */ 0;
    }
    return 0;
}

/* ---------------------------------------------------------- 0x150c:0x164d
 * Draw a single character `c` at (x, y). Looks up the glyph's width and bitmap
 * from the font descriptor, then hands off to the active blit driver, which
 * does the per-pixel work (the far call through drv+0x24).
 *
 * Font glyph lookup (exact):
 *   idx        = (c - f->first_char) * 2
 *   width      = f->width_tab[c - f->first_char]          ; must be > 0
 *   bitmap_ptr = f->glyph_data + f->off_tab[c - f->first_char]
 *   cell_h     = f->height - f->descent + drv.ascent
 */
int draw_glyph(int c, int x, int y)
{
    Font *f;
    int   width;
    uint8_t *bitmap;

    if (font_select_150c_09f7() == 0)
        return 0;
    if ((f = font_lookup_150c_019d(font_select_150c_09f7())) == 0)
        return -1;

    if (c < f->first_char || c > f->last_char)
        return 0xF82D;                             /* char not in font */

    width  = f->width_tab[c - f->first_char];      /* 0x16C8: [bx + f+0x44] */
    bitmap = f->glyph_data + f->off_tab[c - f->first_char];  /* 0x16D1: f+0x4c + [bx + f+0x48] */
    if (width <= 0)
        return 0xF82D;

    /* Set up the blit (origin, size, clip from the driver fields) and dispatch
     * to the active per-pixel routine: jmp far [drv + 0x24]. */
    blit_setup_150c_0043();
    /* drv = blit_ctx_150c_010a(...); ((blit_fn)drv->draw)(...); */
    return 0;
}

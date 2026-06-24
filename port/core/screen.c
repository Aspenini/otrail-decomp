/*
 * screen.c - software framebuffer + 8x8 text rendering.
 */
#include "screen.h"
#include "font8x8.h"

static uint8_t  s_fb[PAL_SCREEN_W * PAL_SCREEN_H];
static uint32_t s_pal[256];

uint8_t  *scr_fb(void)      { return s_fb; }
uint32_t *scr_palette(void) { return s_pal; }

void scr_set_ega_palette(void)
{
    static const uint32_t ega[16] = {
        0x000000, 0x0000AA, 0x00AA00, 0x00AAAA, 0xAA0000, 0xAA00AA, 0xAA5500,
        0xAAAAAA, 0x555555, 0x5555FF, 0x55FF55, 0x55FFFF, 0xFF5555, 0xFF55FF,
        0xFFFF55, 0xFFFFFF
    };
    int i;
    for (i = 0; i < 16; i++) s_pal[i] = ega[i];
    for (; i < 256; i++)     s_pal[i] = 0;
}

void scr_clear(uint8_t color)
{
    int i;
    for (i = 0; i < PAL_SCREEN_W * PAL_SCREEN_H; i++) s_fb[i] = color;
}

void scr_putc(int x, int y, char c, uint8_t color)
{
    int row, col;
    unsigned uc = (unsigned char)c;
    if (uc >= 'a' && uc <= 'z') uc -= 32;          /* fold to uppercase */
    if (uc < FONT_FIRST || uc > FONT_LAST) uc = ' ';
    const uint8_t *g = font8x8[uc - FONT_FIRST];
    for (row = 0; row < 8; row++) {
        int py = y + row;
        if (py < 0 || py >= PAL_SCREEN_H) continue;
        for (col = 0; col < 8; col++) {
            int px = x + col;
            if (px < 0 || px >= PAL_SCREEN_W) continue;
            if (g[row] & (0x80 >> col))
                s_fb[py * PAL_SCREEN_W + px] = color;
        }
    }
}

void scr_text(int x, int y, const char *s, uint8_t color)
{
    for (; *s; s++, x += 8) scr_putc(x, y, *s, color);
}

int scr_text_w(const char *s)
{
    int n = 0;
    while (*s++) n++;
    return n * 8;
}

void scr_present(void)
{
    pal_present(s_fb, s_pal);
}

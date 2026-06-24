/*
 * pcx.c - 8-bit ZSoft PCX decoder. Mirrors port/assets/pcx.py; this is the C
 * the port uses at runtime to turn the game's PCX art into framebuffer pixels.
 */
#include "pcx.h"

int pcx_decode(const uint8_t *d, size_t len,
               uint8_t *pixels, size_t max_pixels,
               uint32_t palette[256],
               int *width, int *height, int *has_palette)
{
    int xmin, ymin, xmax, ymax, w, h, bpl, x, y;
    size_t p, total, i;

    if (len < 128 || d[0] != 0x0A || d[3] != 8 || d[65] != 1)
        return 1;                                  /* not 8-bit single-plane PCX */

    xmin = d[4]  | (d[5]  << 8);
    ymin = d[6]  | (d[7]  << 8);
    xmax = d[8]  | (d[9]  << 8);
    ymax = d[10] | (d[11] << 8);
    bpl  = d[66] | (d[67] << 8);                   /* bytes per line */
    w = xmax - xmin + 1;
    h = ymax - ymin + 1;
    if (w <= 0 || h <= 0 || (size_t)w * h > max_pixels)
        return 2;

    /* RLE-decode the scanlines (encoding byte d[2]==1) into a temp via pixels.
     * We decode straight into the destination, trimming each line's padding. */
    p = 128;
    for (y = 0; y < h; y++) {
        x = 0;
        int line = 0;                              /* bytes consumed this line */
        while (line < bpl) {
            uint8_t b, val;
            int count, k;
            if (p >= len) return 3;
            b = d[p++];
            if (d[2] == 1 && (b & 0xC0) == 0xC0) { /* run */
                count = b & 0x3F;
                if (p >= len) return 3;
                val = d[p++];
            } else {
                count = 1;
                val = b;
            }
            for (k = 0; k < count && line < bpl; k++, line++) {
                if (x < w)                         /* drop scanline padding */
                    pixels[(size_t)y * w + x++] = val;
            }
        }
    }

    /* 256-colour palette: trailing 0x0C marker + 768 RGB bytes at end of file. */
    if (has_palette) *has_palette = 0;
    if (len >= 769 && d[len - 769] == 0x0C) {
        const uint8_t *pal = d + len - 768;
        for (i = 0; i < 256; i++)
            palette[i] = ((uint32_t)pal[i * 3] << 16) |
                         ((uint32_t)pal[i * 3 + 1] << 8) |
                          (uint32_t)pal[i * 3 + 2];
        if (has_palette) *has_palette = 1;
    }

    total = (size_t)w * h;
    (void)total;
    *width = w;
    *height = h;
    return 0;
}

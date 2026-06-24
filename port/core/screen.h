/*
 * screen.h - software text/drawing into the 320x200 indexed framebuffer.
 * Portable core; mirrors what the game's graphics module does, but writes into
 * memory and presents through the PAL.
 */
#ifndef OTRAIL_SCREEN_H
#define OTRAIL_SCREEN_H

#include <stdint.h>
#include "../pal.h"

/* 16 EGA-style colours live in palette indices 0..15. */
enum {
    C_BLACK = 0, C_BLUE, C_GREEN, C_CYAN, C_RED, C_MAGENTA, C_BROWN, C_LGRAY,
    C_DGRAY, C_LBLUE, C_LGREEN, C_LCYAN, C_LRED, C_LMAGENTA, C_YELLOW, C_WHITE
};

uint8_t  *scr_fb(void);            /* the PAL_SCREEN_W*PAL_SCREEN_H framebuffer */
uint32_t *scr_palette(void);       /* 256-entry palette                        */
void scr_set_ega_palette(void);    /* fill indices 0..15 with the EGA colours   */
void scr_clear(uint8_t color);
void scr_putc(int x, int y, char c, uint8_t color);
void scr_text(int x, int y, const char *s, uint8_t color);  /* lowercase -> upper */
int  scr_text_w(const char *s);    /* pixel width of a string                   */
void scr_present(void);            /* push the frame through the PAL            */

#endif /* OTRAIL_SCREEN_H */

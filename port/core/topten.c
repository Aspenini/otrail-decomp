/*
 * topten.c - "See the Oregon Top Ten" (menu option 3), ported from
 * src/seg_c00_topten.c. Renders the framed high-score screen.
 *
 * The live list lives in HISCORES.REC, whose record format isn't decoded yet,
 * so we show the game's default Top Ten (historical Oregon Trail figures) as a
 * stand-in. The layout matches the original: rank, name, and score.
 */
#include <stdio.h>
#include "screen.h"
#include "../pal.h"

struct entry { const char *name; int score; };

/* Default Oregon Top Ten (stand-in for HISCORES.REC). */
static const struct entry TOP_TEN[10] = {
    { "Stephen Meek",      7650 },
    { "Celinda Hines",     6820 },
    { "Andrew Sublette",   5740 },
    { "David Hastings",    5100 },
    { "Ezra Meeker",       4565 },
    { "Marcus Whitman",    3990 },
    { "Mary Sublette",     3320 },
    { "Jesse Applegate",   2980 },
    { "John Fremont",      2460 },
    { "William Sublette",  1950 },
};

void run_topten(void)
{
    PalEvent ev;
    int i, y;

    scr_clear(C_BLACK);
    /* frame */
    for (i = 0; i < PAL_SCREEN_W; i++) {
        scr_fb()[2 * PAL_SCREEN_W + i] = C_LGREEN;
        scr_fb()[(PAL_SCREEN_H - 3) * PAL_SCREEN_W + i] = C_LGREEN;
    }
    scr_text(88, 12, "THE OREGON TOP TEN", C_YELLOW);

    y = 40;
    for (i = 0; i < 10; i++, y += 14) {
        char line[48];
        sprintf(line, "%2d. %-20s %5d", i + 1, TOP_TEN[i].name, TOP_TEN[i].score);
        scr_text(40, y, line, C_LCYAN);
    }
    scr_text(88, 186, "PRESS A KEY", C_LGREEN);
    scr_present();

    for (;;) {
        if (pal_should_quit()) return;
        if (pal_poll_event(&ev) && ev.key != PAL_KEY_NONE) return;
        pal_sleep_ms(16);
    }
}

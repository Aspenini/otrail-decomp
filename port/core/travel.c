/*
 * travel.c - a minimal "Travel the Trail" status screen (menu option 1).
 *
 * The full trail loop (src/seg032_trail.c et al.) isn't ported yet; this shows
 * the iconic travel status panel using the decompiled state model (date,
 * weather, health, pace, rations, food, miles) so there's a taste of the trail.
 */
#include <stdio.h>
#include "screen.h"
#include "../pal.h"

void run_travel(void)
{
    PalEvent ev;

    scr_clear(C_BLACK);
    scr_text(72, 8, "AUGUST 1, 1848", C_YELLOW);

    scr_text(24, 40,  "Weather........ warm",        C_WHITE);
    scr_text(24, 56,  "Health......... good",        C_WHITE);
    scr_text(24, 72,  "Pace........... steady",      C_WHITE);
    scr_text(24, 88,  "Rations........ filling",     C_WHITE);
    scr_text(24, 104, "Food........... 1800 pounds", C_WHITE);

    scr_text(24, 132, "Next landmark:",              C_LGREEN);
    scr_text(24, 148, "Fort Kearney - 304 miles",    C_LCYAN);

    scr_text(16, 184, "(TRAIL LOOP NOT YET PORTED)",  C_DGRAY);
    scr_present();

    for (;;) {
        if (pal_should_quit()) return;
        if (pal_poll_event(&ev) && ev.key != PAL_KEY_NONE) return;
        pal_sleep_ms(16);
    }
}

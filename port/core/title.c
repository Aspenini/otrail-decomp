/*
 * title.c - the interactive title menu, ported from src/seg000_main.c.
 *
 * Mirrors main()'s menu loop: draw the title and the six options, read a 1-6
 * choice (Esc / End quits), and dispatch. The sub-screens (Travel, Learn, …)
 * are not ported yet, so non-quit choices show a placeholder; option 4 toggles
 * sound just like the original (g_sound_on), and 6 ends.
 *
 * This is the decompiled logic running on the portable framebuffer + PAL input.
 */
#include <stdio.h>

#include "screen.h"
#include "../pal.h"

static int g_sound_on = 1;   /* the original's g_sound_on (0x1410) */

/* Block until the player presses 1-6 or quits (Esc / window close). */
static int read_choice(void)
{
    PalEvent ev;
    for (;;) {
        if (pal_should_quit())
            return 6;                          /* treat as End */
        while (pal_poll_event(&ev)) {
            if (ev.key == PAL_KEY_ESCAPE)
                return 6;
            if (ev.key == PAL_KEY_CHAR && ev.ch >= '1' && ev.ch <= '6')
                return ev.ch - '0';
        }
        pal_sleep_ms(16);
    }
}

static void draw_menu(void)
{
    char line4[40];
    scr_clear(C_BLUE);
    scr_text(96, 16, "THE OREGON TRAIL", C_YELLOW);
    scr_text(40, 44, "YOU MAY:", C_WHITE);
    scr_text(56,  64, "1. TRAVEL THE TRAIL",          C_LCYAN);
    scr_text(56,  76, "2. LEARN ABOUT THE TRAIL",     C_LCYAN);
    scr_text(56,  88, "3. SEE THE OREGON TOP TEN",    C_LCYAN);
    sprintf(line4, "4. TURN SOUND %s", g_sound_on ? "OFF" : "ON");
    scr_text(56, 100, line4,                          C_LCYAN);
    scr_text(56, 112, "5. CHOOSE MANAGEMENT OPTIONS", C_LCYAN);
    scr_text(56, 124, "6. END",                       C_LCYAN);
    scr_text(40, 150, "WHAT IS YOUR CHOICE? 1-6", C_WHITE);
    scr_present();
}

void run_title(void)
{
    for (;;) {
        draw_menu();
        int choice = read_choice();
        if (choice == 6)
            break;
        if (choice == 4) {
            g_sound_on = !g_sound_on;           /* like main() case 4 */
            continue;
        }
        /* sub-screens not ported yet */
        scr_clear(C_BLUE);
        {
            char msg[40];
            sprintf(msg, "OPTION %d SELECTED", choice);
            scr_text(72, 88, msg, C_YELLOW);
        }
        scr_text(32, 108, "(SUB-SCREEN NOT YET PORTED)", C_WHITE);
        scr_present();
        read_choice();                          /* wait, then back to menu */
    }
}

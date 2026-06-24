/*
 * learn.c - "Learn about the Trail" (menu option 2), ported from
 * src/seg_c6b_learn.c. The page text is the real content recovered from the
 * binary (the '\'-separated lines become array entries). Each page is shown
 * until a key is pressed; Esc backs out (like the original's quit flag).
 */
#include "screen.h"
#include "../pal.h"

static const char *page1[] = {
    "Try taking a journey by",
    "covered wagon across 2000",
    "miles of plains, rivers, and",
    "mountains.  Try!  On the",
    "plains, will you slosh your",
    "oxen through mud and",
    "water-filled ruts or will you",
    "plod through dust six inches",
    "deep?", 0
};
static const char *page2[] = {
    "How will you cross the rivers?",
    "If you have money, you might",
    "take a ferry (if there is a",
    "ferry).  Or, you can ford the",
    "river and hope you and your",
    "wagon aren't swallowed alive!", 0
};
static const char *page3[] = {
    "What about supplies?  Well, if",
    "you're low on food you can",
    "hunt.  You might get a",
    "buffalo... you might.  And",
    "there are bear in the",
    "mountains.", 0
};
static const char *page4[] = {
    "At the Dalles, you can try",
    "navigating the Columbia River,",
    "but if running the rapids with",
    "a makeshift raft makes you",
    "queasy, better take the Barlow",
    "Road.", 0
};
static const char *page5[] = {
    "If for some reason you don't",
    "survive -- your wagon burns,",
    "or thieves steal your oxen, or",
    "you run out of provisions, or",
    "you die of cholera -- don't",
    "give up!  Try again...and",
    "again...until your name is up",
    "with the others on The Oregon",
    "Top Ten.", 0
};
static const char *credits[] = {
    "The software team responsible",
    "for creation of this product",
    "includes:", "",
    "Ed Gratz", "Charolyn Kapplinger",
    "Mark Paquette", "Larry Phenow",
    "Julie Redland", 0
};

static const char **pages[] = { page1, page2, page3, page4, page5, credits, 0 };

/* Wait for any key; return 1 if the player wants to back out (Esc / quit). */
static int wait_key(void)
{
    PalEvent ev;
    for (;;) {
        if (pal_should_quit()) return 1;
        while (pal_poll_event(&ev)) {
            if (ev.key == PAL_KEY_ESCAPE) return 1;
            if (ev.key != PAL_KEY_NONE)   return 0;
        }
        pal_sleep_ms(16);
    }
}

void run_learn(void)
{
    int p, i, y;
    for (p = 0; pages[p]; p++) {
        scr_clear(C_BLUE);
        scr_text(56, 8, "LEARN ABOUT THE TRAIL", C_YELLOW);
        y = 36;
        for (i = 0; pages[p][i]; i++, y += 11)
            scr_text(16, y, pages[p][i], C_WHITE);
        scr_text(96, 186, "PRESS A KEY", C_LGREEN);
        scr_present();
        if (wait_key())
            return;
    }
}

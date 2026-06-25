/*
 * trail.c - the playable trail loop (menu option 1, "Travel the Trail").
 *
 * Ported from the decompiled travel engine: src/seg032_trail.c (the per-turn
 * loop), src/seg032_arrival.c (landmark arrival) and src/seg032_map.c (the
 * location table + forks). Distances and the fork structure are the originals
 * read out of OREGON.EXE; this drives them as a portable, playable loop:
 * a status screen, an action menu, day-by-day travel with food consumption,
 * landmark arrivals, the South Pass / Blue Mountains forks, and win/lose.
 */
#include <stdio.h>
#include <string.h>

#include "screen.h"
#include "game.h"
#include "../pal.h"

static const char *MONTHS[] = { "", "JANUARY", "FEBRUARY", "MARCH", "APRIL",
    "MAY", "JUNE", "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER" };
static const int MDAYS[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static const char *HEALTH_W[]  = { "good", "fair", "poor", "very poor" };
static const char *PACE_W[]    = { "steady", "strenuous", "grueling" };
static const char *RATIONS_W[] = { "filling", "meager", "bare bones" };

/* ------------------------------------------------------------- input helpers */

/* Block for a single key; return its PalEvent. Quit/close reports ESCAPE. */
static PalEvent wait_key(void)
{
    PalEvent ev;
    for (;;) {
        if (pal_should_quit()) { ev.key = PAL_KEY_ESCAPE; ev.ch = 0; return ev; }
        while (pal_poll_event(&ev)) {
            if (ev.key != PAL_KEY_NONE) return ev;
        }
        pal_sleep_ms(16);
    }
}

/* Block for a digit in [lo..hi]; ESCAPE returns 0. */
static int read_choice(int lo, int hi)
{
    for (;;) {
        PalEvent ev = wait_key();
        if (ev.key == PAL_KEY_ESCAPE) return 0;
        if (ev.key == PAL_KEY_CHAR && ev.ch >= '0' + lo && ev.ch <= '0' + hi)
            return ev.ch - '0';
    }
}

static void press_any_key(int x, int y)
{
    scr_text(x, y, "PRESS ANY KEY TO CONTINUE", C_DGRAY);
    scr_present();
    wait_key();
}

/* --------------------------------------------------------------- date helper */

static void advance_one_day(void)
{
    GameState *s = &g_state;
    if (++s->day > MDAYS[s->month]) {
        s->day = 1;
        if (++s->month > 12) { s->month = 1; s->year++; }
    }
}

/* ------------------------------------------------------------- status screen */

static void draw_status(void)
{
    GameState *s = &g_state;
    char line[48];

    scr_clear(C_BLACK);
    sprintf(line, "%s %d, %d", MONTHS[s->month], s->day, s->year);
    scr_text(160 - scr_text_w(line) / 2, 8, line, C_YELLOW);

    scr_text(24, 36,  "Health.........", C_LGRAY);
    scr_text(160, 36, HEALTH_W[s->health], C_WHITE);
    scr_text(24, 50,  "Pace...........", C_LGRAY);
    scr_text(160, 50, PACE_W[s->pace], C_WHITE);
    scr_text(24, 64,  "Rations........", C_LGRAY);
    scr_text(160, 64, RATIONS_W[s->rations], C_WHITE);
    scr_text(24, 78,  "Food (lbs).....", C_LGRAY);
    sprintf(line, "%ld", s->food);
    scr_text(160, 78, line, C_WHITE);
    scr_text(24, 92,  "Miles travelled", C_LGRAY);
    sprintf(line, "%ld", s->total_miles);
    scr_text(160, 92, line, C_WHITE);

    scr_text(24, 116, "Next landmark:", C_LGREEN);
    sprintf(line, "%s", g_landmarks[s->next_location].name);
    scr_text(24, 130, line, C_LCYAN);
    sprintf(line, "%d miles ahead", s->miles_to);
    scr_text(24, 144, line, C_LCYAN);

    scr_text(24, 170, "1.GO  2.SUPPLIES  3.PACE", C_WHITE);
    scr_text(24, 182, "4.RATIONS  5.REST  6.QUIT", C_WHITE);
    scr_present();
}

/* --------------------------------------------------------- travel mechanics */

static int miles_per_day(void)
{
    int base = (g_state.pace == PACE_STEADY) ? 14
             : (g_state.pace == PACE_STRENUOUS) ? 18 : 22;
    if (g_state.health >= HEALTH_POOR) base -= 4;   /* sick parties slow down */
    return base + game_rand(7) - 3;                 /* +-3 wobble */
}

static int food_per_day(void)
{
    int per = (g_state.rations == RATIONS_FILLING) ? 3
            : (g_state.rations == RATIONS_MEAGER)  ? 2 : 1;
    return g_state.party * per;
}

/* Arrival at the landmark we just reached; resolve the next leg (with fork). */
static void arrive(void)
{
    GameState *s = &g_state;
    char line[48];

    s->location   = s->next_location;
    s->miles_past = 0;

    scr_clear(C_BLUE);
    scr_text(160 - scr_text_w("YOU HAVE REACHED") / 2, 60, "YOU HAVE REACHED", C_YELLOW);
    scr_text(160 - scr_text_w(g_landmarks[s->location].name) / 2, 80,
             g_landmarks[s->location].name, C_WHITE);
    sprintf(line, "%s %d, %d", MONTHS[s->month], s->day, s->year);
    scr_text(160 - scr_text_w(line) / 2, 104, line, C_LCYAN);
    press_any_key(160 - scr_text_w("PRESS ANY KEY TO CONTINUE") / 2, 150);

    if (s->location == LOC_OREGON) { s->won = 1; return; }

    /* Fork? dest2 != 0 means the trail divides here. */
    const Landmark *L = &g_landmarks[s->location];
    int choice = 1;
    if (L->dest2 != 0) {
        scr_clear(C_BLACK);
        scr_text(24, 24, "THE TRAIL DIVIDES HERE.", C_YELLOW);
        scr_text(24, 40, "YOU MAY:", C_WHITE);
        sprintf(line, "1. HEAD FOR %s", g_landmarks[L->dest1].name);
        scr_text(24, 64, line, C_LCYAN);
        sprintf(line, "   (%d MILES)", L->miles1);
        scr_text(24, 76, line, C_DGRAY);
        sprintf(line, "2. HEAD FOR %s", g_landmarks[L->dest2].name);
        scr_text(24, 96, line, C_LCYAN);
        sprintf(line, "   (%d MILES)", L->miles2);
        scr_text(24, 108, line, C_DGRAY);
        scr_text(24, 140, "WHAT IS YOUR CHOICE? 1-2", C_WHITE);
        scr_present();
        choice = read_choice(1, 2);
        if (choice == 0) choice = 1;
    }

    s->next_location = (choice == 2) ? L->dest2 : L->dest1;
    s->miles_to      = (choice == 2) ? L->miles2 : L->miles1;
}

/* "Continue on the trail": advance up to a week of travel, then redraw. */
static void travel(void)
{
    GameState *s = &g_state;
    int days;

    for (days = 0; days < 7; days++) {
        int m = miles_per_day();
        if (m < 1) m = 1;
        advance_one_day();
        s->food -= food_per_day();
        if (s->food < 0) s->food = 0;

        s->miles_past  += m;
        s->total_miles += m;
        s->miles_to    -= m;

        /* starvation slowly wrecks the party's health */
        if (s->food == 0 && game_rand(3) == 0 && s->health < HEALTH_VPOOR)
            s->health++;
        if (s->food == 0 && s->health == HEALTH_VPOOR && game_rand(8) == 0) {
            s->dead = 1; return;
        }

        if (s->miles_to <= 0) { arrive(); return; }
    }
}

/* ------------------------------------------------------------- sub-actions */

static void show_supplies(void)
{
    GameState *s = &g_state;
    char line[48];
    scr_clear(C_BLACK);
    scr_text(24, 16, "SUPPLIES", C_YELLOW);
    sprintf(line, "Food.......... %ld lbs", s->food);
    scr_text(24, 44, line, C_WHITE);
    sprintf(line, "Party......... %d people", s->party);
    scr_text(24, 60, line, C_WHITE);
    sprintf(line, "Eating %d lbs/day (%s)", food_per_day(), RATIONS_W[s->rations]);
    scr_text(24, 84, line, C_LGRAY);
    press_any_key(24, 160);
}

static void cycle(int *v, int n, const char *title, const char **words)
{
    char line[48];
    *v = (*v + 1) % n;
    scr_clear(C_BLACK);
    scr_text(24, 60, title, C_YELLOW);
    sprintf(line, "now: %s", words[*v]);
    scr_text(24, 84, line, C_WHITE);
    press_any_key(24, 160);
}

static void rest(void)
{
    GameState *s = &g_state;
    int i;
    for (i = 0; i < 3; i++) { advance_one_day(); s->food -= food_per_day(); }
    if (s->food < 0) s->food = 0;
    if (s->health > HEALTH_GOOD) s->health--;   /* resting recovers a notch */
    scr_clear(C_BLACK);
    scr_text(24, 70, "YOU REST FOR 3 DAYS.", C_YELLOW);
    scr_text(24, 90, "THE PARTY FEELS BETTER.", C_WHITE);
    press_any_key(24, 160);
}

/* ------------------------------------------------------------------- entry */

void run_travel(void)
{
    GameState *s = &g_state;

    /* fresh journey on first entry, or after a finished game */
    if (s->year == 0 || s->won || s->dead)
        game_new();

    for (;;) {
        draw_status();
        int choice = read_choice(1, 6);
        switch (choice) {
        case 1: travel(); break;
        case 2: show_supplies(); break;
        case 3: cycle(&s->pace, 3, "TRAVEL PACE", PACE_W); break;
        case 4: cycle(&s->rations, 3, "RATIONS", RATIONS_W); break;
        case 5: rest(); break;
        case 0:                 /* Esc */
        case 6: return;         /* back to the main menu */
        }

        if (s->won) {
            scr_clear(C_GREEN);
            scr_text(160 - scr_text_w("YOU REACHED OREGON!") / 2, 80,
                     "YOU REACHED OREGON!", C_YELLOW);
            scr_text(160 - scr_text_w("CONGRATULATIONS, PIONEER!") / 2, 100,
                     "CONGRATULATIONS, PIONEER!", C_WHITE);
            press_any_key(160 - scr_text_w("PRESS ANY KEY TO CONTINUE") / 2, 150);
            return;
        }
        if (s->dead) {
            scr_clear(C_BLACK);
            scr_text(160 - scr_text_w("YOUR PARTY HAS PERISHED") / 2, 80,
                     "YOUR PARTY HAS PERISHED", C_LRED);
            scr_text(160 - scr_text_w("ON THE TRAIL.") / 2, 100, "ON THE TRAIL.", C_LGRAY);
            press_any_key(160 - scr_text_w("PRESS ANY KEY TO CONTINUE") / 2, 150);
            return;
        }
    }
}

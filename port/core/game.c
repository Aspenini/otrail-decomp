/*
 * game.c - trail data + game state for the port.
 *
 * g_landmarks is the decompiled location table (src/seg032_map.c); the values
 * are the originals read straight from OREGON.EXE (DGROUP 0x0896).
 */
#include "game.h"

const Landmark g_landmarks[LM_COUNT] = {
    { "Independence",                 1,  0, 102,   0 },
    { "the Kansas River crossing",    2,  0,  83,   0 },
    { "the Big Blue River crossing",  3,  0, 119,   0 },
    { "Fort Kearney",                 4,  0, 250,   0 },
    { "Chimney Rock",                 5,  0,  86,   0 },
    { "Fort Laramie",                 6,  0, 190,   0 },
    { "Independence Rock",            7,  0, 102,   0 },
    { "South Pass",                   9,  8,  57, 125 },  /* fork: Green River / Fort Bridger */
    { "Fort Bridger",                10,  0, 162,   0 },
    { "Green River crossing",        10,  0, 144,   0 },
    { "Soda Springs",                11,  0,  57,   0 },
    { "Fort Hall",                   12,  0, 182,   0 },
    { "the Snake River crossing",    13,  0, 114,   0 },
    { "Fort Boise",                  14,  0, 160,   0 },
    { "the Blue Mountains",          15, 16,  55, 125 },  /* fork: Walla Walla / The Dalles */
    { "Fort Walla Walla",            16,  0, 120,   0 },
    { "The Dalles",                  17,  0, 100,   0 },
    { "the Willamette Valley",       DEST_END, 0, 0, 0 },
};

GameState g_state;

int game_rand(int n)
{
    /* small portable LCG (no libc rand dependency in the core) */
    g_state.rng = g_state.rng * 1103515245u + 12345u;
    return n > 0 ? (int)((g_state.rng >> 16) % (unsigned)n) : 0;
}

void game_new(void)
{
    GameState *s = &g_state;
    int keep_sound = s->sound_on;
    for (unsigned i = 0; i < sizeof *s; i++) ((char *)s)[i] = 0;

    s->location      = 0;                 /* Independence */
    s->next_location = g_landmarks[0].dest1;
    s->miles_to      = g_landmarks[0].miles1;
    s->miles_past    = 0;
    s->total_miles   = 0;
    s->food          = 2000;              /* a full wagon; "filling" is still risky */
    s->party         = 5;
    s->day = 1; s->month = 3; s->year = 1848;   /* March 1, 1848 */
    s->health  = HEALTH_GOOD;
    s->pace    = PACE_STEADY;
    s->rations = RATIONS_FILLING;
    s->sound_on = keep_sound;
    s->rng = 0xA5A5u;
}

/*
 * game.h - portable game state for the Oregon Trail port.
 *
 * The trail data (landmarks, distances, forks) is the decompiled location table
 * from src/seg032_map.c (DGROUP 0x0896), transcribed as portable data. The
 * state struct mirrors the run-time globals documented in docs/ARCHITECTURE.md.
 */
#ifndef OTRAIL_GAME_H
#define OTRAIL_GAME_H

#define LM_COUNT   18
#define LOC_OREGON 17        /* the Willamette Valley = trail's end */
#define DEST_END   99        /* dest1 sentinel at the final landmark */

/* One trail landmark. dest/miles are choice-indexed: choice 1 -> dest1/miles1,
 * choice 2 -> dest2/miles2. dest2 == 0 means there is no fork here. */
typedef struct {
    const char   *name;
    unsigned char dest1, dest2;
    unsigned char miles1, miles2;
} Landmark;

extern const Landmark g_landmarks[LM_COUNT];

/* Health / pace / rations are small enums shown as words on the status screen. */
enum { HEALTH_GOOD, HEALTH_FAIR, HEALTH_POOR, HEALTH_VPOOR };
enum { PACE_STEADY, PACE_STRENUOUS, PACE_GRUELING };
enum { RATIONS_FILLING, RATIONS_MEAGER, RATIONS_BARE };

typedef struct {
    int  location;       /* current landmark id (g_location, 0x15ea)            */
    int  next_location;  /* g_next_location (0x1610)                            */
    int  miles_to;       /* miles remaining to next landmark (g_miles_to,0x160f)*/
    int  miles_past;     /* miles past the last landmark (g_miles_past, 0x160d) */
    long total_miles;    /* miles travelled so far                              */
    long food;           /* wagon food in pounds (g_food, 0x15ca)              */
    int  party;          /* living party members                               */
    int  day, month, year;
    int  health;         /* HEALTH_*                                            */
    int  pace;           /* PACE_*                                              */
    int  rations;        /* RATIONS_*                                           */
    int  sound_on;
    unsigned rng;        /* simple LCG state                                    */
    int  won, dead;      /* terminal flags                                      */
} GameState;

extern GameState g_state;

void game_new(void);         /* start a fresh journey from Independence */
int  game_rand(int n);       /* 0..n-1 */

void run_travel(void);       /* the trail loop (menu option 1) */

#endif /* OTRAIL_GAME_H */

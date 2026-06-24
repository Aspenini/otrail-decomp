/*
 * seg1049_status.c - travel-status formatters (segment 0x1049).
 *
 *   fmt_weather @ 0x1049:0x2d8b
 *   fmt_health  @ 0x1049:0x2db7
 *   fmt_pace    @ 0x1049:0x2dfd
 *   fmt_rations @ 0x1049:0x2e2a
 *
 * Each copies the name of the current state into a buffer (via strncpy at
 * 0x20a4:0x064e) for the travel status panel (see travel_turn). Together they
 * pin down the readable state model.
 *
 * Address-annotated reconstruction; not yet compile-verified.
 */

#include <stdint.h>

/* Status state (DGROUP). */
extern uint16_t g_weather;     /* 0x172e: weather index                          */
extern uint16_t g_health_lo;   /* 0x15d8: party health value (low word of long)  */
extern uint16_t g_health_mid;  /* 0x15da                                         */
extern uint16_t g_health_hi;   /* 0x15dc                                         */
extern uint8_t  g_pace;        /* 0x15e8: 0=steady, 1=strenuous, 2=grueling      */
extern uint8_t  g_rations;     /* 0x15e9: 0=filling, 1=meager, 2=bare bones      */

extern void strncpy_n(char far *dst, const char far *src, int n);  /* 0x20a4:0x064e */

/* Name tables (\-separated, in DGROUP / CONST):
 *   weather temperature : cold / cool / warm ;  precipitation: rainy / snowy /
 *                         very rainy / very snowy
 *   health levels       : good / fair / poor / very poor   (thresholds 500/400/300/200)
 *   pace                : steady / strenuous / grueling
 *   rations             : filling / meager / bare bones
 */
extern const char far *g_weather_names[];   /* DGROUP ~0x0c8e */
extern const char far *g_health_names[4];   /* "good","fair","poor","very poor" */
extern const char far *g_pace_names[3];
extern const char far *g_ration_names[3];

#define HEALTH_DIV 0xc00   /* 3072: health value scaled down to a level index */

/* ---------------------------------------------------------- 0x1049:0x2d8b */
void fmt_weather(char far *out)
{
    strncpy_n(out, g_weather_names[g_weather], 0xFF);   /* 0x2D95 */
}

/* ---------------------------------------------------------- 0x1049:0x2db7
 * Maps the 32-bit health value down (÷ HEALTH_DIV) to one of four levels:
 * good / fair / poor / very poor.
 */
void fmt_health(char far *out)
{
    long health = (long)g_health_lo | ((long)g_health_mid << 16);  /* 0x2DC1 */
    int  level  = (int)(health / HEALTH_DIV);                      /* 0x2DD4: ÷0xc00 */
    strncpy_n(out, g_health_names[level], 0xFF);
}

/* ---------------------------------------------------------- 0x1049:0x2dfd */
void fmt_pace(char far *out)
{
    strncpy_n(out, g_pace_names[g_pace], 0xFF);          /* 0x2E07 */
}

/* ---------------------------------------------------------- 0x1049:0x2e2a */
void fmt_rations(char far *out)
{
    strncpy_n(out, g_ration_names[g_rations], 0xFF);     /* 0x2E34 */
}

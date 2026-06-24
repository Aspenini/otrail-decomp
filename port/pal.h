/*
 * pal.h - Platform Abstraction Layer for the Oregon Trail port.
 *
 * This is the ONLY interface the portable game core (port/core/) uses to talk to
 * the outside world. Implement it once per backend (port/platform/<name>/) and
 * the whole game runs there. The surface is deliberately tiny: the game is a
 * 320x200 256-colour indexed framebuffer plus a stream of key events.
 *
 * Design rules for portability (Wii U / Switch / iOS / Android / web / desktop):
 *   - plain C99, no dynamic allocation in the core, no platform types here;
 *   - the core never blocks — it renders a frame, polls events, and returns;
 *   - all I/O (assets, saves, audio, time) goes through this header.
 */
#ifndef OREGON_PAL_H
#define OREGON_PAL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- display ------------------------------------------------------------- */
/* The game renders into a fixed 320x200 indexed buffer; the backend scales and
 * presents it (integer-scaled, letterboxed) however it likes. */
#define PAL_SCREEN_W 320
#define PAL_SCREEN_H 200

/* Push one finished frame. `indexed` is PAL_SCREEN_W*PAL_SCREEN_H bytes; each
 * byte selects a colour in `palette` (256 entries, 0x00RRGGBB). */
void pal_present(const uint8_t *indexed, const uint32_t palette[256]);

/* ---- input --------------------------------------------------------------- */
/* The game only needs a handful of keys: digits/letters for menus and names,
 * Y/N, Enter, Esc (quit/back), Ctrl-S (sound), and arrows + fire for hunting.
 * Backends translate gamepad buttons / touch / on-screen keyboard into these. */
typedef enum {
    PAL_KEY_NONE = 0,
    PAL_KEY_CHAR,      /* a printable character is in PalEvent.ch          */
    PAL_KEY_ENTER,
    PAL_KEY_ESCAPE,
    PAL_KEY_BACKSPACE,
    PAL_KEY_UP, PAL_KEY_DOWN, PAL_KEY_LEFT, PAL_KEY_RIGHT,
    PAL_KEY_FIRE,      /* hunting: shoot                                   */
    PAL_KEY_TOGGLE_SOUND
} PalKey;

typedef struct {
    PalKey key;
    char   ch;         /* valid when key == PAL_KEY_CHAR                   */
} PalEvent;

/* Pop the next input event. Returns 1 and fills `out`, or 0 if the queue is
 * empty. Never blocks. */
int pal_poll_event(PalEvent *out);

/* ---- timing -------------------------------------------------------------- */
uint32_t pal_ticks_ms(void);     /* monotonic milliseconds                 */
void     pal_sleep_ms(uint32_t ms);

/* ---- audio (PC-speaker-style tones; safe to stub as no-ops) ------------- */
void pal_tone(uint32_t freq_hz, uint32_t duration_ms); /* async beep       */
void pal_tone_stop(void);

/* ---- assets (read-only bundled game data) -------------------------------- */
/* Load a named asset (e.g. "family.pcc") fully into a caller-supplied buffer.
 * Returns the byte count read, or -1 on error. Pass buf=NULL to query size. */
long pal_asset_load(const char *name, void *buf, size_t buf_size);

/* ---- persistent storage (saved games / high scores) ---------------------- */
/* Read/write a small named blob in platform-appropriate storage (a user dir,
 * NAND, IndexedDB, …). Return bytes read / 0 on success / -1 on error. */
long pal_storage_read(const char *key, void *buf, size_t buf_size);
int  pal_storage_write(const char *key, const void *buf, size_t size);

/* ---- lifecycle ----------------------------------------------------------- */
/* Set up the backend (window/audio/input). Returns 0 on success. */
int  pal_init(const char *title);
/* True once the user asked to close the app (window close, HOME, etc.). */
int  pal_should_quit(void);
void pal_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* OREGON_PAL_H */

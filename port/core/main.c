/*
 * main.c - minimal boot for the Oregon Trail port.
 *
 * This is the first vertical slice: bring up the platform, load the real title
 * art (LOGO.256) through the asset interface, decode it, and present it through
 * the PAL. It exercises the whole pipeline (pal_init -> pal_asset_load -> PCX
 * decode -> pal_present -> pal_poll_event) in portable C, independent of any
 * specific backend. The game logic from ../../src/ slots in above this over
 * time; for now this proves the boot path end-to-end.
 */
#include <stdio.h>

#include "../pal.h"
#include "pcx.h"
#include "screen.h"

extern void run_title(void);   /* title.c */

static uint8_t g_asset[70000];

/* Show the MECC title splash (LOGO.256) until a key is pressed. */
static void splash(void)
{
    long n = pal_asset_load("LOGO.256", g_asset, sizeof(g_asset));
    int  w, h, hp;
    PalEvent ev;
    if (n < 0 || pcx_decode(g_asset, (size_t)n, scr_fb(), PAL_SCREEN_W * PAL_SCREEN_H,
                            scr_palette(), &w, &h, &hp))
        return;                                  /* no splash, go straight to menu */
    scr_present();
    for (;;) {
        if (pal_should_quit()) return;
        if (pal_poll_event(&ev) && ev.key != PAL_KEY_NONE) return;
        pal_sleep_ms(16);
    }
}

int main(void)
{
    if (pal_init("The Oregon Trail")) {
        fprintf(stderr, "boot: pal_init failed\n");
        return 1;
    }
    splash();                                    /* MECC logo (its own palette) */
    scr_set_ega_palette();                       /* menu uses the EGA palette */
    run_title();                                 /* interactive title menu */
    pal_shutdown();
    return 0;
}

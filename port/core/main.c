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
#include <string.h>

#include "../pal.h"
#include "pcx.h"

static uint8_t  g_fb[PAL_SCREEN_W * PAL_SCREEN_H];
static uint32_t g_palette[256];
static uint8_t  g_asset[70000];   /* big enough for LOGO.256 / a 320x200 PCX */

/* Load a PCX asset into the framebuffer + palette. Returns 0 on success. */
static int load_pcx_screen(const char *name)
{
    long n;
    int  w, h, hp;

    n = pal_asset_load(name, g_asset, sizeof(g_asset));
    if (n < 0) {
        fprintf(stderr, "boot: cannot load asset '%s'\n", name);
        return 1;
    }
    if (pcx_decode(g_asset, (size_t)n, g_fb, sizeof(g_fb), g_palette, &w, &h, &hp)) {
        fprintf(stderr, "boot: '%s' is not a decodable 8-bit PCX\n", name);
        return 1;
    }
    return 0;
}

int main(void)
{
    PalEvent ev;

    if (pal_init("The Oregon Trail")) {
        fprintf(stderr, "boot: pal_init failed\n");
        return 1;
    }

    /* The title screen: MECC's LOGO.256 (a 320x200 PCX with its own palette). */
    if (load_pcx_screen("LOGO.256")) {
        pal_shutdown();
        return 1;
    }

    /* Present and wait for a key / quit. */
    while (!pal_should_quit()) {
        pal_present(g_fb, g_palette);
        while (pal_poll_event(&ev)) {
            if (ev.key == PAL_KEY_ESCAPE || ev.key == PAL_KEY_ENTER)
                goto done;
        }
        pal_sleep_ms(16);
    }
done:
    pal_shutdown();
    return 0;
}

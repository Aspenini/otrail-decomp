/*
 * pal_sdl.c - SDL2 PAL backend (desktop / web / mobile / Switch / Wii U).
 *
 * Implements pal.h on top of SDL2: the 320x200 indexed framebuffer is uploaded
 * to a streaming texture and presented integer-scaled into a resizable window.
 * Keyboard (and, where present, gamepad/touch via SDL) map to PalEvents.
 *
 * Build with SDL2 found by CMake (see port/CMakeLists.txt). This single backend
 * covers every SDL target; only the toolchain/input-mapping differs per device.
 */
#include "../../pal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#define SCALE 3

static SDL_Window   *s_win;
static SDL_Renderer *s_ren;
static SDL_Texture  *s_tex;
static int           s_quit;

int pal_init(const char *title)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    s_win = SDL_CreateWindow(title ? title : "Oregon Trail",
                             SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             PAL_SCREEN_W * SCALE, PAL_SCREEN_H * SCALE,
                             SDL_WINDOW_RESIZABLE);
    if (!s_win) return 1;
    s_ren = SDL_CreateRenderer(s_win, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!s_ren) return 1;
    SDL_RenderSetLogicalSize(s_ren, PAL_SCREEN_W, PAL_SCREEN_H);  /* letterbox */
    s_tex = SDL_CreateTexture(s_ren, SDL_PIXELFORMAT_ARGB8888,
                              SDL_TEXTUREACCESS_STREAMING,
                              PAL_SCREEN_W, PAL_SCREEN_H);
    return s_tex ? 0 : 1;
}

void pal_present(const uint8_t *indexed, const uint32_t palette[256])
{
    void *pix; int pitch, x, y;
    if (SDL_LockTexture(s_tex, NULL, &pix, &pitch) != 0) return;
    for (y = 0; y < PAL_SCREEN_H; y++) {
        uint32_t *row = (uint32_t *)((uint8_t *)pix + (size_t)y * pitch);
        for (x = 0; x < PAL_SCREEN_W; x++)
            row[x] = 0xFF000000u | palette[indexed[(size_t)y * PAL_SCREEN_W + x]];
    }
    SDL_UnlockTexture(s_tex);
    SDL_RenderClear(s_ren);
    SDL_RenderCopy(s_ren, s_tex, NULL, NULL);
    SDL_RenderPresent(s_ren);
}

int pal_poll_event(PalEvent *out)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { s_quit = 1; continue; }
        if (e.type == SDL_TEXTINPUT) {                  /* printable characters */
            out->key = PAL_KEY_CHAR; out->ch = e.text.text[0];
            return 1;
        }
        if (e.type == SDL_KEYDOWN) {
            SDL_Keycode k = e.key.keysym.sym;
            int ctrl = (e.key.keysym.mod & KMOD_CTRL) != 0;
            out->ch = 0;
            if (ctrl && k == SDLK_s)      out->key = PAL_KEY_TOGGLE_SOUND;
            else switch (k) {
                case SDLK_RETURN: case SDLK_KP_ENTER: out->key = PAL_KEY_ENTER; break;
                case SDLK_ESCAPE:    out->key = PAL_KEY_ESCAPE; break;
                case SDLK_BACKSPACE: out->key = PAL_KEY_BACKSPACE; break;
                case SDLK_UP:    out->key = PAL_KEY_UP; break;
                case SDLK_DOWN:  out->key = PAL_KEY_DOWN; break;
                case SDLK_LEFT:  out->key = PAL_KEY_LEFT; break;
                case SDLK_RIGHT: out->key = PAL_KEY_RIGHT; break;
                case SDLK_SPACE: out->key = PAL_KEY_FIRE; break;
                default: continue;                      /* handled by TEXTINPUT */
            }
            return 1;
        }
    }
    return 0;
}

int      pal_should_quit(void) { return s_quit; }
uint32_t pal_ticks_ms(void)    { return SDL_GetTicks(); }
void     pal_sleep_ms(uint32_t ms) { SDL_Delay(ms); }
void     pal_tone(uint32_t hz, uint32_t ms) { (void)hz; (void)ms; }  /* TODO: audio */
void     pal_tone_stop(void) {}

void pal_shutdown(void)
{
    if (s_tex) SDL_DestroyTexture(s_tex);
    if (s_ren) SDL_DestroyRenderer(s_ren);
    if (s_win) SDL_DestroyWindow(s_win);
    SDL_Quit();
}

/* Assets / saves: read the original game files from a directory. */
long pal_asset_load(const char *name, void *buf, size_t buf_size)
{
    const char *dir = getenv("OTRAIL_GAMEDIR");
    char path[512]; FILE *f; long n;
    if (!dir) dir = "Oregon_The_1990";
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    f = fopen(path, "rb");
    if (!f) return -1;
    if (!buf) { fseek(f, 0, SEEK_END); n = ftell(f); fclose(f); return n; }
    n = (long)fread(buf, 1, buf_size, f);
    fclose(f);
    return n;
}

long pal_storage_read(const char *key, void *buf, size_t n)
{ (void)key; (void)buf; (void)n; return -1; }
int  pal_storage_write(const char *key, const void *buf, size_t n)
{ (void)key; (void)buf; (void)n; return -1; }

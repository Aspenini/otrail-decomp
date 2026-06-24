/*
 * pal_file.c - headless PAL backend.
 *
 * Implements pal.h without any GUI: pal_present writes the frame to a PNG, and
 * assets are read from the original game directory. This lets the port be built
 * and run anywhere (CI, no display) and makes the rendered output inspectable.
 * It is the proof-of-life backend; pal_sdl.c is the interactive one.
 *
 * Config via environment:
 *   OTRAIL_GAMEDIR  directory holding the original game files (default
 *                   "Oregon_The_1990")
 *   OTRAIL_FRAME    output PNG path (default "build/port_frame.png")
 */
#include "../../pal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ----------------------------------------------------------- minimal PNG out */
static uint32_t crc_tab[256];
static void crc_init(void)
{
    uint32_t c; int n, k;
    for (n = 0; n < 256; n++) {
        c = (uint32_t)n;
        for (k = 0; k < 8; k++)
            c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
        crc_tab[n] = c;
    }
}
static uint32_t crc_upd(uint32_t c, const uint8_t *b, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) c = crc_tab[(c ^ b[i]) & 0xFF] ^ (c >> 8);
    return c;
}
static void put32(FILE *f, uint32_t v)
{
    fputc((v >> 24) & 0xFF, f); fputc((v >> 16) & 0xFF, f);
    fputc((v >> 8) & 0xFF, f);  fputc(v & 0xFF, f);
}
static void chunk(FILE *f, const char *tag, const uint8_t *data, size_t len)
{
    uint32_t c = 0xFFFFFFFFu;
    put32(f, (uint32_t)len);
    fwrite(tag, 1, 4, f);
    c = crc_upd(c, (const uint8_t *)tag, 4);
    if (len) { fwrite(data, 1, len, f); c = crc_upd(c, data, len); }
    put32(f, c ^ 0xFFFFFFFFu);
}

static uint8_t s_raw[PAL_SCREEN_H * (1 + PAL_SCREEN_W * 3)];
static uint8_t s_zlib[sizeof(s_raw) + 4096];

static int write_png(const char *path, const uint8_t *idx, const uint32_t pal[256])
{
    FILE *f = fopen(path, "wb");
    uint8_t ihdr[13];
    size_t rawlen = 0, zl = 0, off, a = 1, b = 0, i;
    int x, y;
    if (!f) return 1;
    crc_init();

    /* raw scanlines: filter byte 0 + RGB */
    for (y = 0; y < PAL_SCREEN_H; y++) {
        s_raw[rawlen++] = 0;
        for (x = 0; x < PAL_SCREEN_W; x++) {
            uint32_t c = pal[idx[(size_t)y * PAL_SCREEN_W + x]];
            s_raw[rawlen++] = (c >> 16) & 0xFF;
            s_raw[rawlen++] = (c >> 8) & 0xFF;
            s_raw[rawlen++] = c & 0xFF;
        }
    }
    /* zlib stream: header + stored deflate blocks + adler32 */
    s_zlib[zl++] = 0x78; s_zlib[zl++] = 0x01;
    off = 0;
    while (off < rawlen) {
        size_t n = rawlen - off; if (n > 65535) n = 65535;
        s_zlib[zl++] = (off + n >= rawlen) ? 1 : 0;     /* BFINAL for last */
        s_zlib[zl++] = n & 0xFF;        s_zlib[zl++] = (n >> 8) & 0xFF;
        s_zlib[zl++] = ~n & 0xFF;       s_zlib[zl++] = (~n >> 8) & 0xFF;
        memcpy(s_zlib + zl, s_raw + off, n); zl += n; off += n;
    }
    for (i = 0; i < rawlen; i++) {                      /* adler32 of raw */
        a = (a + s_raw[i]) % 65521; b = (b + a) % 65521;
    }
    s_zlib[zl++] = (b >> 8) & 0xFF; s_zlib[zl++] = b & 0xFF;
    s_zlib[zl++] = (a >> 8) & 0xFF; s_zlib[zl++] = a & 0xFF;

    fwrite("\x89PNG\r\n\x1a\n", 1, 8, f);
    ihdr[0] = (PAL_SCREEN_W >> 24) & 0xFF; ihdr[1] = (PAL_SCREEN_W >> 16) & 0xFF;
    ihdr[2] = (PAL_SCREEN_W >> 8) & 0xFF;  ihdr[3] = PAL_SCREEN_W & 0xFF;
    ihdr[4] = (PAL_SCREEN_H >> 24) & 0xFF; ihdr[5] = (PAL_SCREEN_H >> 16) & 0xFF;
    ihdr[6] = (PAL_SCREEN_H >> 8) & 0xFF;  ihdr[7] = PAL_SCREEN_H & 0xFF;
    ihdr[8] = 8; ihdr[9] = 2; ihdr[10] = 0; ihdr[11] = 0; ihdr[12] = 0;
    chunk(f, "IHDR", ihdr, 13);
    chunk(f, "IDAT", s_zlib, zl);
    chunk(f, "IEND", 0, 0);
    fclose(f);
    return 0;
}

/* ------------------------------------------------------------- PAL interface */
static const char *s_keys;     /* OTRAIL_KEYS: scripted keypresses */
static int s_keypos;
static int s_frame;

int pal_init(const char *title)
{
    (void)title;
    s_keys = getenv("OTRAIL_KEYS");
    return 0;
}

void pal_present(const uint8_t *indexed, const uint32_t palette[256])
{
    const char *base = getenv("OTRAIL_FRAME");
    char path[512];
    if (!base) base = "build/port_frame.png";
    /* number the frames: foo.png -> foo_000.png, foo_001.png, ... */
    {
        const char *dot = strrchr(base, '.');
        size_t stem = dot ? (size_t)(dot - base) : strlen(base);
        snprintf(path, sizeof(path), "%.*s_%03d%s",
                 (int)stem, base, s_frame, dot ? dot : ".png");
    }
    write_png(path, indexed, palette);
    fprintf(stderr, "pal_file: frame %d -> %s\n", s_frame, path);
    s_frame++;
}

/* Feed OTRAIL_KEYS one character per poll as PAL_KEY_CHAR events. */
int pal_poll_event(PalEvent *out)
{
    if (s_keys && s_keys[s_keypos]) {
        out->key = PAL_KEY_CHAR;
        out->ch = s_keys[s_keypos++];
        return 1;
    }
    return 0;
}

/* Quit once the scripted input is exhausted (or if none was given). */
int pal_should_quit(void) { return !s_keys || s_keys[s_keypos] == 0; }

uint32_t pal_ticks_ms(void) { return 0; }
void     pal_sleep_ms(uint32_t ms) { (void)ms; }
void     pal_tone(uint32_t hz, uint32_t ms) { (void)hz; (void)ms; }
void     pal_tone_stop(void) {}
void     pal_shutdown(void) {}

long pal_asset_load(const char *name, void *buf, size_t buf_size)
{
    const char *dir = getenv("OTRAIL_GAMEDIR");
    char path[512];
    FILE *f;
    long n;
    if (!dir) dir = "Oregon_The_1990";
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    f = fopen(path, "rb");
    if (!f) return -1;
    if (!buf) { fseek(f, 0, SEEK_END); n = ftell(f); fclose(f); return n; }
    n = (long)fread(buf, 1, buf_size, f);
    fclose(f);
    return n;
}

long pal_storage_read(const char *key, void *buf, size_t buf_size)
{ (void)key; (void)buf; (void)buf_size; return -1; }
int  pal_storage_write(const char *key, const void *buf, size_t size)
{ (void)key; (void)buf; (void)size; return -1; }

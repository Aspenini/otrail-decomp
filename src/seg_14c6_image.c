/*
 * seg_14c6_image.c - image (.PCC) loader module (segment 0x14c6).
 *
 *   image_load  @ 0x14c6:0x021a  (far) - load a .PCC and blit it to the screen
 *   image_show  @ 0x14c6:0x0321  (far) - load a .PCC into a caller buffer
 *   image_blit  @ 0x14c6:0x03ea  (far) - blit a previously-loaded image
 *   image_free  @ 0x14c6:0x043c  (far) - release a loaded image
 *
 * The game's art lives in .PCC (PCX-style) files next to OREGON.EXE
 * (family.pcc, supplies.pcc, terrain.pcc, animals.pcc, float.pcc, ...). This
 * module is the thin front-end the rest of the game calls; the actual PCX
 * open/decode/blit/close primitives live in the graphics codec at segment
 * 0x182e (named bgi_* below) and the device layer at 0x150c.
 *
 * A loaded-image buffer carries an 8-byte header: width (u16), height (u16),
 * and the codec's own 4-byte handle, followed by the pixel rows.
 *
 * Address-annotated reconstruction of Borland Turbo C output; the codec's
 * internal coordinate math is summarised. Not yet compile-verified.
 */

#include <stdint.h>

/* PCX/BGI codec primitives (segment 0x182e). */
extern int  bgi_open_image(char far *pathbuf, const char far *name,
                           void far *hdr);                  /* 0x182e:0x22a0 */
extern int  bgi_read_image(int mode, void far *dst, int w, int h); /* 0x182e:0x3b6d */
extern int  bgi_image_meta(void far *dst, int w, int h);    /* 0x182e:0x3e46 */
extern int  bgi_blit(void far *src, int x0, int y0, int x1, int y1,
                     int x, int y, int op);                 /* 0x182e:0x3f65 */
extern int  bgi_close_image(void far *src);                 /* 0x182e:0x3c35 */

extern void report_error(int code);                         /* 0x1049:0x13bb */
extern void memcpy_n(void far *d, const void far *s, int n);/* 0x20a4:0x025d */
extern void strncpy_n(char far *d, const char far *s, int n);/* 0x20a4:0x064e */
extern void _exit(int code);                                /* 0x20a4:0x00d8 */

extern char     g_image_pathbuf[256]; /* 0x179c: scratch path buffer for the load */
extern uint16_t g_bgi_mode;           /* 0x189c: active decode/palette mode */

/* ---------------------------------------------------------- 0x14c6:0x021a
 * Load a .PCC image and blit it straight to the screen at (x, y).
 * On a missing/bad file the codec returns -1 and report_error(0x98) fires.
 */
void image_load(int x, int y, char far *name)
{
    char  namebuf[256];           /* [bp-0x100] */
    char  hdr[128];               /* [bp-0x180] image header from the codec */
    int   w, h;
    int   rc;

    strncpy_n(namebuf, name, 0xff);                         /* 0x0238 */
    rc = bgi_open_image(g_image_pathbuf, namebuf, hdr);     /* 0x0250 */
    if (rc == -1)                                           /* 0x0259 */
        report_error(0x98);                                 /* 0x0263: load failed */
    if (rc == -1) return;                                   /* 0x0268 */

    w = /* hdr.x1 */ 0 - /* hdr.x0 */ 0;                    /* 0x026F: width  */
    h = /* hdr.y1 */ 0 - /* hdr.y0 */ 0;                    /* 0x027B: height */

    bgi_read_image(g_bgi_mode, /* tmp buf */ 0, w, h);      /* 0x029F: decode pixels */
    bgi_image_meta(/* tmp buf */ 0, 0, 0);                  /* 0x02BB */
    bgi_blit(/* tmp buf */ 0, 0, 0, x, y, x + w, y + h, 0); /* 0x0301: paint to screen */
    bgi_close_image(/* tmp buf */ 0);                       /* 0x0312: release */
}

/* ---------------------------------------------------------- 0x14c6:0x0321
 * Load a .PCC image into a caller-provided buffer (no screen output). The
 * buffer's first words receive the width/height; later image_blit paints it.
 * The codec error -0x1a (==226) triggers an _exit(0xcb) (out-of-memory abort).
 */
void image_show(void far *dst, char far *name)
{
    char  namebuf[256];           /* [bp-0x100] */
    char  hdr[128];               /* [bp-0x180] */
    int   w, h;
    int   rc;

    strncpy_n(namebuf, name, 0xff);                         /* 0x033F */
    rc = bgi_open_image(g_image_pathbuf, namebuf, hdr);     /* 0x0357 */
    if (rc == -1)                                           /* 0x0360 */
        report_error(0x98);                                 /* 0x036A */
    if (rc == -1) return;                                   /* 0x036F */

    w = /* hdr.x1 - hdr.x0 */ 0;                            /* 0x0376 */
    h = /* hdr.y1 - hdr.y0 */ 0;                            /* 0x0382 */

    rc = bgi_read_image(g_bgi_mode, dst, w, h);             /* 0x03A7: decode into dst */
    if (rc == 0)                                            /* 0x03B0 */
        /* stash width/height into the buffer header (dst[0]/dst[2]) */
        bgi_image_meta(dst, /* dst->w */ 0, /* dst->h */ 0);/* 0x03CC */
    if (rc == -0x1a)                                        /* 0x03D5: 226 = no memory */
        _exit(0xcb);                                        /* 0x03DF */
}

/* ---------------------------------------------------------- 0x14c6:0x03ea
 * Blit an image previously loaded by image_show. Reads the 8-byte buffer
 * header (width/height/handle) and paints the rectangle at (x, y).
 */
void image_blit(void far *buf, int x, int y, int w, int h)
{
    uint16_t meta[4];             /* [bp-0x8] copy of the buffer header */

    memcpy_n(meta, buf, 8);                                 /* 0x0406: read header */
    /* paint the image's own rect, offset to (x, y) + (w, h) */
    bgi_blit(buf, x, y, w, h, x + /* meta.dx */ 0,
             y + /* meta.dy */ 0, 0);                       /* 0x042E */
}

/* ---------------------------------------------------------- 0x14c6:0x043c
 * Release a loaded image (close its codec handle / free the pixel buffer).
 */
void image_free(void far *buf)
{
    bgi_close_image(buf);                                   /* 0x0450 */
}

/*
 * pcx.h - decode 8-bit ZSoft PCX images (the game's .256 / .pcc format).
 * Portable core logic (no platform calls).
 */
#ifndef OTRAIL_PCX_H
#define OTRAIL_PCX_H

#include <stddef.h>
#include <stdint.h>

/* Decode an 8-bit RLE PCX. Writes the indexed pixels (row-major, width*height
 * bytes) into `pixels` (must hold at least max_pixels) and, if the file carries
 * one, the 256-entry palette (0x00RRGGBB) into `palette`. width/height receive
 * the image size. Returns 0 on success, non-zero on error. If `has_palette` is
 * non-NULL it is set to 1 when an embedded palette was found. */
int pcx_decode(const uint8_t *data, size_t len,
               uint8_t *pixels, size_t max_pixels,
               uint32_t palette[256],
               int *width, int *height, int *has_palette);

#endif /* OTRAIL_PCX_H */

# Game asset formats

What the original game's data files are and how the converters handle them.
Run `xmake assets` to extract.

## PCX images — `.256`, `.pcc`  ✅ done

Standard 8-bit ZSoft PCX (`0A 05 01 08` header: v5, RLE, 8 bits/pixel, 1 plane).
`LOGO.256` is a 320×200 title image; `PAL.256` is a tiny image whose only job is
to carry the **global 256-colour palette** (trailing `0x0C` + 768 RGB bytes).

Decoder: [`pcx.py`](pcx.py). Verified by decoding `LOGO.256` to the MECC
"Oregon Trail™" logo.

## PCXLIB archives — `.PCL`  ✅ done

`OTMCGA.PCL` (MCGA/256-colour) and `OTCGA.PCL` (CGA) are **Genus PCXLIB**
archives (`"pcxLib"` signature, © Genus Microprogramming 1988-89). Each holds
28 PCX images. Directory entry layout:

```
name : 12 bytes  ("NAME    .EXT", 8.3, space-padded)
+12  : 1 byte    flag
+13  : uint32    PCX data size
...  : metadata  (entry header is 83 bytes total)
+83  : PCX image data
```

The archived images do **not** embed their own palette — they share the global
`PAL.256` palette. Contents: `animals`, `hunter`, `terrain`, `events`,
`supplies`, `family`, `travelox`, `float`, `banner`, `scenery`, and `p0`–`p17`
(the 18 landmark/location images, matching the trail's locations).

Extractor: [`pcxlib.py`](pcxlib.py). Verified: the hunter sprite sheet (aiming
in every direction) and the animals sheet (bison/deer/rabbits) decode with
correct colours.

## Bitmap font — `BIT8X8.GFT`  🟡 structure reversed (from the renderer)

A Genus "GFT" proportional bitmap font (`"BIT8X8"` signature). The layout below
was recovered authoritatively by decompiling the game's own text routines —
`draw_string` (`0x150c:0x301f`) and `draw_glyph` (`0x150c:0x164d`); see
`src/seg_150c_gfxtext.c`. The font descriptor (in memory) exposes:

| Field        | Meaning                                              |
|--------------|------------------------------------------------------|
| `+0x24`      | `first_char` (lowest character code present)         |
| `+0x26`      | `last_char`                                          |
| `+0x44`      | per-glyph **advance width** table (uint16/glyph)     |
| `+0x48`      | per-glyph **bitmap offset** table (uint16/glyph)     |
| `+0x4c`      | base of the glyph **bitmap data**                    |
| `+0x52`      | font cell height                                     |

In the file these map to: offset table near `0x54`, glyph bitmap data at
`0x256` (matches the monotonic uint16 array and ~2048-byte data region found
earlier). Glyph for char `c` lives at `data + off_tab[c - first_char]`.

**Key correction:** glyphs are indexed from `first_char`, *not* from char 0 —
which is why naive char-0-based renderings produced noise.

**Blit format (from the driver, `0x150c:0x191e` CGA / `0x1f7c` MCGA):** glyphs
are **row-major** — `height` rows, `ceil(width/8)` bytes per row, each bit
selecting foreground vs background colour. The MSB is the left pixel.

**Still TODO — the file glyph data is encoded.** Read as raw row-major bitmaps,
the bytes at `0x256` are dense/high-entropy and don't form letters under any
row/column × MSB/LSB layout. So the on-disk glyph data is compressed/encoded and
expanded at load time. The decoder lives in the **font loader** (reached via
`font_data_locate` `0x150c:0x0ea4` / `font_lookup` `0x150c:0x019d`), not the
blitter — tracing that is the remaining step to render the font from the
original file. (The art pipeline doesn't depend on this; only text rendering
does, and a modern port can ship its own 8×8 font if preferred.)

## Record/data files — `.REC`  ⬜ TODO

`HISCORES.REC` (the Oregon Top Ten), `GAMES.REC` (saved games — the 143-byte
records `prompt_continue_saved_game` reads), `DIALOGS.REC`, `TOMB.REC`. Read via
the Borland C stdio in segment `0x20a4` (`fopen` = `0x20a4:0x15ba`).

- **`DIALOGS.REC`** — fort NPC dialog: `talk_to_people` (`0x07ce:0x17eb`) reads a
  random record (a speaker name + a quote) and shows `<name> tells you: "…"`.
- **`GAMES.REC`** — saved games; 143-byte records (profession at byte 0, party
  names at +1), as used by `prompt_continue_saved_game`.

Exact record sizes/layout per file still to be confirmed by examining the files.

## Audio — `SONGS.TXT`  ⬜ TODO

Music data; not yet examined.

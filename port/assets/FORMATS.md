# Game asset formats

What the original game's data files are and how the converters handle them.
Run `make assets` to extract.

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

## Bitmap font — `BIT8X8.GFT`  🟡 container reversed, bit-packing TODO

A Genus "GFT" proportional bitmap font (`"BIT8X8"` signature). Structure so far:

```
0x00 .. 0x56   header (signature + glyph metrics: width/height 8, count, …)
0x56 .. 0x256  offset table: 256 × uint16, monotonic, max ~2043
0x256 .. EOF   glyph bitmap data (~2048 bytes), variable widths (~7-8 px)
```

The offset table and data region are confirmed (offsets span the data region;
deltas of 7-8 imply a proportional 8-tall font). **Not yet resolved:** the exact
glyph indexing (first-char base) and per-glyph bit packing — naive column/row ×
MSB/LSB renderings all produce noise, which means the indexing base and/or
packing differ from the obvious guesses.

**The authoritative source is the game's own glyph blitter**, `gfx_draw_text`
(`0x150c:0x301f`), which indexes this font per character. Reversing that function
will pin the format exactly — the right next step instead of more guessing.

## Record/data files — `.REC`  ⬜ TODO

`HISCORES.REC` (the Oregon Top Ten), `GAMES.REC` (saved games — the 143-byte
records `prompt_continue_saved_game` reads), `DIALOGS.REC`, `TOMB.REC`. Formats
not yet examined; the save-record layout is partly known from the decomp
(profession at byte 0, party names at +1).

## Audio — `SONGS.TXT`  ⬜ TODO

Music data; not yet examined.

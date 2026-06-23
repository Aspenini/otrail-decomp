# OREGON.EXE is LZEXE 0.91 — unpacking notes

`Oregon_The_1990/OREGON.EXE` (79,322 bytes) is **packed with LZEXE 0.91**
(Fabrice Bellard's DOS EXE compressor). The `LZ91` signature sits at offset
`0x1C` of the MZ header, where the relocation table would otherwise be
(`e_crlc = 0`).

This single fact reframes the whole project: **the bytes in the packed file are
LZ77-compressed and cannot be decompiled directly.** Any approach that chops the
packed file into byte-array "units" is storing compressed data verbatim, not
recovering program logic. Decompilation has to happen on the *unpacked* image.

## Packed file layout

| File range            | Contents                                     |
|-----------------------|----------------------------------------------|
| `0x0000 .. 0x0020`    | MZ header (2 paragraphs), `LZ91` at `0x1C`   |
| `0x0020 .. 0x126F0`   | LZ77-compressed program image (`0x126D` paras)|
| `0x126F0 .. end`      | LZEXE decompressor stub + RLE reloc table    |

The MZ header points the entry at `CS:IP = 0x126D:0x000E` (file `0x126FE`),
i.e. the stub. The LZEXE info block at `e_cs:0` (file `0x126F0`) holds the real
program's `IP=0x010A CS=0x0000 SP=0x9C40 SS=0x25EB` and the compressed size.

## Algorithm (transcribed from the stub in this binary)

Disassembling the stub at file `0x126F0` gives the exact decompressor. Two
details break naive ports and are reproduced in `tools/unlzexe.py`:

1. **Eager control-word reload.** The 16-bit control word is reloaded the moment
   the bit counter reaches zero (mid-token, before the triggering token reads
   its data byte), because control bits and literal/match bytes share one
   forward stream pointer (`lodsw`/`lodsb`/`movsb` all advance `si`).
2. **16-bit wrapped back-references.** Matches copy via `mov al,[es:bx+di]`, so
   the displacement wraps within the destination segment.

Token format (control bit read LSB-first):

- `1` → copy one literal byte.
- `0 0` → short match: 2 length bits (len `2..5`), 1 displacement byte
  (`0xFF00 | byte`, i.e. `-1..-256`).
- `0 1` → long match: 2 bytes → 13-bit displacement (`0xE000 |...`), low 3 bits
  of the 2nd byte are length; if `0`, an extra byte gives length (`0` = end of
  stream, `1` = segment boundary/no copy, else `len = byte + 1`).

The **RLE relocation table** starts at stub offset `0x158` (file `0x12848` —
note this is exactly the "code/data split" boundary earlier work had flagged
without identifying it). Decoder (from the stub routine at ds:`0xFC`): read a
byte `b`; if `b != 0` it is an offset delta; if `b == 0` read a word `w`
(`w == 0` → segment += `0xFFF`; `w == 1` → end; else `w` is a 16-bit delta).
After each delta, normalize `off`/`seg` and emit one fixup.

## Result

`python3 tools/unlzexe.py Oregon_The_1990/OREGON.EXE build/OREGON_unpacked.exe`
produces a plain relocatable MZ executable:

- image `0x26B20` bytes (158,496), entry `CS:IP = 0x0000:0x010A`
- `3325` relocations, **all** in-range and pointing at valid segment words
- entry disassembles as `main()`: C-runtime init far-calls then the program

Most-referenced segments in the unpacked image (candidate module/group bases):
`0x20a4` (DGROUP/data, 1586 refs), `0x1049` (797), `0x1ceb` (227), `0x182e`
(209), `0x150c` (145), `0x2231` (124), `0x2042` (105), `0x14c6` (102).

Real decompilation should proceed by loading `build/OREGON_unpacked.exe` into
Ghidra/IDA (16-bit real mode, the segments above) and lifting functions.

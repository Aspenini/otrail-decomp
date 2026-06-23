# OTrailDecomp

A decompilation effort for the MS-DOS release of **The Oregon Trail** (1990).

## What this is (and what it isn't)

The target, `Oregon_The_1990/OREGON.EXE`, is **packed with LZEXE 0.91** (the
`LZ91` signature is at MZ offset `0x1C`). That single fact drives everything:
the bytes in the packed file are LZ77-compressed, so they cannot be disassembled
or decompiled directly. **Step one is always to unpack.**

> Earlier iterations of this repo chopped the *packed* file into hundreds of
> `src/unit_*.c` byte-array files and reported "100% byte coverage." That only
> proved the compressed bytes could be copied verbatim — it was not
> decompilation. That scaffolding (and a homegrown, incorrect unpacker) has been
> removed; it remains in git history. See `docs/LZEXE_unpacking.md`.

## Layout

| Path                         | Purpose                                            |
|------------------------------|----------------------------------------------------|
| `Oregon_The_1990/`           | Original game files (the packed `OREGON.EXE`)      |
| `tools/unlzexe.py`           | Correct LZEXE 0.91 unpacker (decomp + relocations) |
| `tools/map_segments.py`      | Segment / function map from the unpacked image     |
| `tools/verify.py`            | Structural regression gate                         |
| `build/OREGON_unpacked.exe`  | Generated: plain relocatable MZ executable         |
| `config/segments.json`       | Generated: machine-readable segment/function map   |
| `docs/LZEXE_unpacking.md`    | How the packer works and how it was reversed       |
| `docs/segment_map.md`        | Generated: human-readable segment/function map     |
| `src/`, `logic/`             | (empty) home for lifted C as decompilation proceeds|

## Prerequisites

- Python 3.10+
- `ndisasm` (NASM) or Ghidra/IDA for disassembly
- `OREGON.EXE` present at `Oregon_The_1990/OREGON.EXE`

## Usage

```bash
make unpack    # -> build/OREGON_unpacked.exe (158,496 bytes, entry 0x0000:0x010A)
make map       # -> config/segments.json, docs/segment_map.md
make verify    # re-unpack and assert all structural invariants
make help      # list targets
```

## Status

- **Unpacking: done and verified.** `make verify` re-derives the unpacked image
  from scratch and checks it is byte-stable, that all 3325 relocations are valid,
  and that the entry point decodes as `main()`.
- **Segment map: bootstrapped.** 9 confirmed code segments holding ~237
  far-called functions are identified, plus the data group. See
  `docs/segment_map.md`. Identified names accumulate in `config/symbols.json`.
- **Decompilation: underway.** `main()` (program entry at `0x0000:0x010A`) is
  lifted in `src/seg000_main.c` — it is the title-screen menu loop (Travel /
  Learn / Top Ten / Sound / Management / End), confirmed against the in-binary
  menu strings. Next: lift the menu-option handlers (`travel_the_trail` at
  `0xd08:0x2217`, etc.) and the draw/input helpers in segment `0x1049`.
  Load `build/OREGON_unpacked.exe` into Ghidra/IDA as 16-bit real mode using
  the segment bases in the map.

## How to lift the next function

1. Pick a function entry from `docs/segment_map.md` (start with the entry chain
   in `main()` at `0x0000:0x010A`).
2. Disassemble it from the unpacked image (`ndisasm -b16` at the file offset, or
   in Ghidra/IDA).
3. Write the recovered C under `src/`, naming it by `seg_off` or its inferred
   role.
4. Keep `make verify` green (it guards the unpack, the foundation everything
   else rests on).

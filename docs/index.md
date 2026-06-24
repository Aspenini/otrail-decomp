---
title: OTrailDecomp
---

# The Oregon Trail — decompilation & portable port

A from-scratch reverse engineering of the MS-DOS release of **The Oregon Trail**
(MECC, 1990): unpack the binary, recover its logic as readable C, and build a
hyper-portable re-implementation on top.

![Decompilation progress](progress.svg)

_The dashboard is generated from the live symbol table on every `make svg`._

## What's been done

- **Unpacking, solved.** `OREGON.EXE` is LZEXE 0.91-packed; a faithful unpacker
  reproduces a 158 KB, fully relocatable image, verified byte-stable.
- **The whole game, mapped.** ~378 functions across 17 segments are identified,
  and the entire player-facing arc is lifted to readable, annotated C — title
  menu, new-game setup, the store, the trail loop, every travel action, hunting,
  river crossings, random events, the health/calendar model, death &
  tombstones, and the final scoring.
- **The data model, recovered** — calendar, supplies, cash, party health,
  weather, illness, pace & rations.
- **A portable port that boots.** A clean platform layer (PAL) over a 320×200
  framebuffer runs the decompiled title-menu logic as compiled C, with an SDL
  backend (desktop/web/mobile/Switch/Wii U) and a headless backend for CI.

## Documentation

- [**Architecture**](ARCHITECTURE.md) — how the game is structured: the
  segment→module map, program flow, data model, and assets.
- [**LZEXE unpacking**](LZEXE_unpacking.md) — how the packed EXE was reversed
  into an analysable image.
- [**Decompiling workflow**](decompiling.md) — the per-function lifting process
  and conventions.
- [**Segment & function map**](segment_map.md) — the generated reference.

## The port

The [`port/`](https://github.com/Aspenini/otrail-decomp/tree/main/port) tree
builds a maximally portable re-implementation from the decompiled C. See its
`README` and `assets/FORMATS.md` for the platform abstraction layer, the asset
pipeline (PCX / PCXLIB / fonts), and the boot-critical contracts.

## Source

All code, tools, lifted C, and the symbol table live in the
[repository](https://github.com/Aspenini/otrail-decomp). The decomp side is
reproducible from the original binary via `make` (`unpack` → `map` → `verify`);
the port builds with CMake or `make port`.

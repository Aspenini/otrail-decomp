# Oregon Trail — portable port (recomp)

A clean-room, maximally portable re-implementation of the game, built from the
decompilation in this repo. The goal: one game core in plain C99 that runs on
desktop (Windows/macOS/Linux), **web** (WASM), **Android**, **iOS**, and homebrew
**Switch** and **Wii U** — from a single codebase.

## Why this is portable

The original is a turn-based, menu-driven game that software-renders everything
into one **320×200, 256-colour indexed framebuffer** (DOS MCGA mode 13h). There
is no GPU, no 3D, and only one real-time element (the hunting minigame). So the
platform only ever has to:

> present a 320×200 indexed image + 256-colour palette, and report key presses.

That entire contract is [`pal.h`](pal.h) — ~12 functions. Everything platform-
specific lives behind it; the game core never knows what it's running on.

## Layout

```
port/
  pal.h            the platform contract (the only OS-facing interface)
  core/            portable game logic — renders into a uint8_t fb[320*200]
  platform/
    sdl/           one SDL2/SDL3 backend → desktop, web, mobile, Switch, Wii U
    null/          headless backend (CI / tests)
  assets/          converters: .PCC/.256/.GFT/.REC → bundled data + loaders
  CMakeLists.txt   build; a toolchain file selects the target
```

The `core/` is derived from the lifted C in `../src/` (see `../docs/ARCHITECTURE.md`).
The four device-touching modules become PAL calls:

| Original module        | Becomes in the port                              |
|------------------------|--------------------------------------------------|
| `0x150c`/`0x1ceb` gfx  | software blits into the indexed framebuffer      |
| `0x14c6` image loader  | `pal_asset_load` + a `.PCC` decoder              |
| `0x2042` input/time    | `pal_poll_event` / `pal_ticks_ms`                |
| DOS file I/O, saves    | `pal_asset_load` / `pal_storage_read|write`      |

The game-logic functions (`main`, the trail loop, per-turn actions, hunting,
rivers) port over almost unchanged.

## Backends — one SDL build covers everything

| Target            | Toolchain                                  |
|-------------------|--------------------------------------------|
| Windows/macOS/Linux | SDL2 + CMake (native)                    |
| Web               | SDL2 + Emscripten → WASM (`emcmake`)       |
| Android           | SDL2 Android project (NDK)                  |
| iOS               | SDL2 Xcode template                         |
| Switch (homebrew) | SDL2 via devkitPro / libnx (CMake toolchain)|
| Wii U (homebrew)  | SDL2 via devkitPro / wut (CMake toolchain)  |

SDL provides window/texture, keyboard + gamepad + touch, audio, and timing on
all of them. On consoles and touch devices the small input set (digits, Y/N,
Enter, Esc, arrows + fire) maps to buttons or an on-screen keyboard.

## Roadmap

1. **PAL contract** — [`pal.h`](pal.h). ✅ drafted; refine as the core lands.
2. **SDL backend + framebuffer test** — open a window, present a 320×200 test
   pattern with a palette, stream `PalEvent`s. Proves the surface end-to-end.
3. **Asset pipeline** — 🟡 in progress. Image art is decoding cleanly:
   - [`assets/pcx.py`](assets/pcx.py) decodes 8-bit ZSoft **PCX** (`.256`, `.pcc`);
   - [`assets/pcxlib.py`](assets/pcxlib.py) extracts the **Genus PCXLIB** archives
     (`OTMCGA.PCL`/`OTCGA.PCL`) — 28 images each (FAMILY, SUPPLIES, TERRAIN,
     ANIMALS, HUNTER, TRAVELOX, SCENERY, P0–P17 landmarks, …), which share the
     global **`PAL.256`** palette.
   - Run `make assets` to dump them to `build/`. The `BIT8X8.GFT` font's
     container is reversed but its glyph bit-packing needs the renderer traced;
     the `.REC` data files are still TODO. See [`assets/FORMATS.md`](assets/FORMATS.md).
4. **Port the core, top-down** — bring up `main`/the title menu first (text +
   input only), then the trail loop, then each subsystem, replacing device code
   with PAL calls. Reuse the decompiled logic from `../src/`.
5. **Fan out backends** — add Emscripten, then Android/iOS, then the devkitPro
   console toolchains. Each is a CMake toolchain file + input mapping; the core
   and SDL backend don't change.

## Boot sequence

`main` (`0x0000:0x010A`) starts with five init calls, then enters the title
menu. In the port these collapse into a single `pal_init()` + asset load:

| Original boot step        | Addr           | Does                              | Port replacement        |
|---------------------------|----------------|-----------------------------------|-------------------------|
| C runtime startup         | `0x20a4:0x0`   | Borland C `_main`/heap/stdio init | the C runtime / `main`  |
| `init_input`              | `0x2042:0x0`   | keyboard module (BIOS `int 16h`)  | `pal_init` + `pal_poll_event` |
| `init_graphics`           | `0x1ceb:0x1357`| BGI `initgraph` → MCGA 320×200×256 | `pal_init` (320×200 surface) |
| `init_graphics2`          | `0x182e:0x0`   | secondary graphics-module setup   | (folded in)             |
| `init_text`               | `0x150c:0x0`   | text/font module (loads `BIT8X8.GFT`) | bundled 8×8 font     |

Then: load the title art, draw the menu, read the choice. Video uses the
**BGI drivers** (`VGA256.BGI` / `CGA.BGI`) rather than a raw `int 10h` mode set,
so the whole graphics stack (segments `0x150c`/`0x182e`/`0x1ceb`) is the
device layer the port replaces with the framebuffer + `pal_present`.

## Boot-critical contracts (what the PAL must reproduce)

Recovered from the decomp; these are the behaviours the platform layer has to
match for the title screen + menus to work:

- **Video:** MCGA mode 13h (320×200, 256-colour indexed). `gfx_screen_init`
  (`0x1ceb:0x0b71`) clears the frame; images are 8-bit PCX with the global
  `PAL.256` palette. → `pal_present(framebuffer, palette)`.
- **Text:** drawn as glyphs into the framebuffer (`draw_string`/`draw_glyph`,
  `seg_150c_gfxtext.c`); a port may ship its own 8×8 font.
- **Input** (`read_field`, `src/seg1049_input.c`): reads a charset-filtered line
  into a **counted string** (`dst[0]=length`); **Enter** ends a field, **Esc**
  sets the quit flag (Esc-twice backs out a screen), **Ctrl-S** (key `0x13`)
  toggles sound at any time. → `pal_poll_event` + `PAL_KEY_*`.
- **Assets/saves:** `.PCL`/`.PCX` art (see `assets/FORMATS.md`) and `.REC` data
  files. → `pal_asset_load` / `pal_storage_*`.

## Principles

- **Plain C99, no `malloc` in the core.** Game state is small and static — ideal
  for console homebrew and WASM.
- **Core is platform-pure.** If a `core/` file includes anything but `pal.h` and
  the C standard library, that's a bug.
- **Assets are not shipped.** Like the decomp, the port ships converters and
  requires the user's original game files. See `assets/`.

# The Oregon Trail (1990) — program architecture

A map of how the game is structured, built up from the decompilation so far.
It ties the segment/function map (`docs/segment_map.md`) and the symbol table
(`config/symbols.json`) to what each part of the program actually does.

The binary is a 16-bit real-mode MS-DOS program built with **Borland Turbo C**
(every framed function opens with `push bp; mov bp,sp; mov ax,<frame>;
call __stkcheck`). It runs in **MCGA 320×200** graphics and draws all text as
bitmap glyphs — there is no BIOS text mode. See `docs/LZEXE_unpacking.md` for how
the packed `OREGON.EXE` is turned into the analysable `build/OREGON_unpacked.exe`.

## Memory model

Code is spread across several segments; `DGROUP` (data) sits high in the image
near the stack. The most-referenced data group is segment `0x2231`.

| Segment  | Role                                   | Lifted in                       |
|----------|----------------------------------------|---------------------------------|
| `0x0000` | program entry / title-screen menu loop | `src/seg000_main.c`             |
| `0x0032` | **trail engine** (travel loop, turns)  | `src/seg032_trail.c`            |
| `0x0442` | **river crossing**                     | `src/seg_442_river.c`           |
| `0x0606` | **hunting minigame**                   | `src/seg_606_hunt.c`            |
| `0x07ce` | player actions (pace/rations/rest/…)   | —                               |
| `0x0c00` | "See the Oregon Top Ten" (high scores) | —                               |
| `0x0c6b` | "Learn about the Trail" slideshow      | `src/seg_c6b_learn.c`           |
| `0x0d08` | new-game setup + store                 | `src/seg_d08_game_start.c`, `src/seg_d08_setup.c` |
| `0x0f34` | "Choose Management Options"            | —                               |
| `0x1049` | **shared library**: text, input, fmt, save I/O (82 fns) | `src/seg1049_text.c` |
| `0x14c6` | image loader (`.PCC` files)            | —                               |
| `0x150c` | graphics module (glyph blitting)       | —                               |
| `0x182e` | graphics module (42 fns)               | —                               |
| `0x1ceb` | graphics module (boxes/lines, screen save/restore) | —                   |
| `0x2042` | keyboard / input module                | —                               |
| `0x20a4` | Borland C runtime library (`__stkcheck`, `exit`, `sprintf`, long math, …) | — |

Segments with a non-zero **framed** count in `docs/segment_map.md` are
application logic; segments with `framed = 0` (`0x20a4`, `0x182e`, `0x150c`,
`0x1ceb`) are hand-written graphics primitives or the C runtime.

## Program flow

```
main()                                         0x0000:0x010A   seg000_main.c
  └─ title menu loop (choice 1..6)
       1 Travel the Trail
           travel_the_trail()                  0x0d08:0x2217   seg_d08_game_start.c
             ├─ prompt_continue_saved_game()   0x0d08:0x20ee
             └─ start_new_game()               0x0d08:0x1f7c
                  ├─ choose_profession()       0x0d08:0x01f2   seg_d08_setup.c
                  ├─ name_party()              0x0d08:0x04d0
                  ├─ choose_departure_month()  0x0d08:0x0a4f
                  └─ buy_supplies()            0x0d08:0x1e78  (store_buy_loop 0x1938)
           travel_loop()                       0x0032:0x3f93   seg032_trail.c
             └─ per location (0..0x11 = Oregon):
                  travel_turn()                0x0032:0x3cb3
                    ├─ 1 Continue → continue_travel()  0x0032:0x361a
                    │      └─ cross_river()     0x0442:0x18bb  seg_442_river.c
                    ├─ 2 Check supplies   3 Look at map         (0x0032)
                    ├─ 4 Pace  5 Rations  6 Rest  7 Trade       (0x07ce)
                    ├─ 8 Hunt → hunt()         0x0606:0x1968   seg_606_hunt.c
                    │      or Talk to people (at forts)         (0x07ce)
                    └─ 9 Buy supplies (at forts)                (0x07ce)
       2 Learn about the Trail  learn_about_trail()  0x0c6b:0x051b  seg_c6b_learn.c
       3 See the Oregon Top Ten see_top_ten()        0x0c00:0x0433
       4 Sound on/off           (handled inline in main)
       5 Management Options     choose_management()  0x0f34:0x0fad
       6 End                    exit()
```

## Data model (DGROUP)

Names below are the inferred ones in `config/symbols.json`.

**Active saved game** — `g_active_save` at `0x1588`, a 143-byte record:

| Offset       | Field                                                  |
|--------------|--------------------------------------------------------|
| `+0`         | profession (1=banker, 2=carpenter, 3=farmer)           |
| `+1` (`0x1589`) | party names: 5 records × 11 bytes (record 0 = leader) |

Saved games are stored as a list of up to **40 records** (143 bytes each);
`prompt_continue_saved_game` loads/selects/deletes slots.

**Trail location table** — `g_locations` at `0x0896`: **18 records of `0x25` (37)
bytes**, indexed by location id `0..0x11` (Independence … the Willamette Valley):

| Offset    | Field       | Meaning                                              |
|-----------|-------------|------------------------------------------------------|
| `+0x00`   | `name[0x1c]`| landmark name, counted string (`dst[0]` = length)    |
| `+0x1c`   | `field_1c`  | region marker (`0x14` for loc 0–4, `0x0c` for 5–17)  |
| `+0x1d`   | `dest1`     | next location id for menu choice 1 (the main path)   |
| `+0x1e`   | `dest2`     | next location id for choice 2 (`0` = no fork here)   |
| `+0x1f`   | `miles1`    | miles to `dest1`                                     |
| `+0x20`   | `miles2`    | miles to `dest2`                                     |
| `+0x21`   | `map_x`     | marker X on the trail map (`u16`)                    |
| `+0x23`   | `map_y`     | marker Y on the trail map (`u16`)                    |

`dest`/`miles` are choice-indexed (`record[+0x1c + choice]` / `record[+0x1e +
choice]`). The game's only two forks fall out of `dest2 != 0`: **South Pass** (7)
→ Green River (9, 57 mi) or Fort Bridger (8, 125 mi), and **the Blue Mountains**
(14) → Fort Walla Walla (15) or The Dalles (16). See `src/seg032_map.c`.

**Run-time game state:**

| Global             | Addr     | Meaning                                          |
|--------------------|----------|--------------------------------------------------|
| `g_location`       | `0x15ea` | current trail location 0..`0x11`; `0x11` = Oregon |
| `g_departure_month`| `0x15c1` | 3=March .. 7=July (drives a `7 - month` score factor) |
| `g_year`           | `0x15c2` | calendar year, starts 1848                        |
| `g_cash`           | `0x15d2` | money (long); seeded by occupation                |
| `g_food`           | `0x15ca` | wagon food in pounds; max 2000, hunt carry limit 100 |
| `g_oxen`           | `0x15c4` | ox count (long); 0 blocks travel (`check_oxen`)   |
| `g_spare_wheels` / `_axles` / `_tongues` | `0x15cf`/`0x15d0`/`0x15d1` | spare wagon parts; used to fix a `broken_wagon` event |
| `g_member_ailment` / `g_member_recovery` | `0x15de[5]` / `0x15e3[5]` | per-member ailment code + recovery-day countdown (`daily_update`) |
| `g_days_in_month` | `0x0889` | days per month, indexed by `g_month`; drives the calendar wrap |
| `g_rations` / `g_pace` | `0x15e9` / `0x15e8` | rations & pace; `daily_update` eats `rations*2` scaled by health |
| `g_died`           | `0x1586` | a death ended the game                            |
| `g_game_over`      | `0x1587` | reached Oregon                                    |
| `g_at_fort`        | `0x1729` | at a fort (enables Talk / Buy)                    |
| `g_must_trade`     | `0x177c` | a missing wagon part blocks travel                |
| `g_sound_on`       | `0x1410` | sound toggle (menu option 4)                      |

Text cursor and layout: `g_text_x`/`g_text_y` (`0x1792`/`0x1794`) pixel cursor;
`g_text_margin_x` (`0x06a0`), `g_text_top_y` (`0x1582`).

## Assets

Most art and data live in separate files next to `OREGON.EXE`, loaded at runtime
by the image module (`0x14c6`) and graphics modules:

| File(s)                                   | Used for                          |
|-------------------------------------------|-----------------------------------|
| `family.pcc`, `supplies.pcc`, `terrain.pcc`, `animals.pcc`, `travelox.pcc`, `scenery.pcc`, `P16.PCC` | screen graphics (`.PCC` images) |
| `*.256`, `PAL.256`                        | 256-colour images / palette       |
| `CGA.BGI`, `VGA256.BGI`                   | Borland Graphics Interface drivers |
| `BIT8X8.GFT`                              | 8×8 bitmap font (text is 8px/char) |
| `*.REC` (`HISCORES`, `GAMES`, `DIALOGS`, `TOMB`) | record/database files       |
| `SONGS.TXT`                               | music data                        |

Strings and small data tables (menu text, the party default-name pool at
`cs:0x5fa`, the fresh-game state vectors) are embedded in the code segments'
CONST pools and are recovered inline with the functions that use them. The game
uses `\` as its line-break character throughout.

## Where to go next

Open subsystems with named-but-not-yet-lifted entry points: the player actions
in `0x07ce` (pace/rations/rest/trade), the high-score screen (`0x0c00`), the
management options (`0x0f34`), and the graphics/runtime libraries. The per-turn
status formatters and the save-file I/O in `0x1049` are also good targets. See
`docs/decompiling.md` for the per-function workflow.

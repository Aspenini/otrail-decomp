# Matching: toward a byte-identical rebuild

The readable C in `src/` is the **spec** — a faithful, annotated reconstruction.
*Matching* is the stricter goal: C that, compiled with the original toolchain,
reproduces the game's machine code **byte for byte**. When a function matches,
it's proven equivalent to the original, not just plausibly so.

This is the hardest part of the project and is gated on the toolchain. The
infrastructure below is in place so the work can proceed function by function,
with a real "% matched" number at every step.

## The target

We match the **unpacked image** (`build/OREGON_unpacked.exe`), not the packed
`OREGON.EXE` — the LZEXE wrapper is irrelevant to the code. Matching is done
**per function**: `config/match_targets.json` (built by `tools/match_inventory.py`)
carves the image into the **218 framed functions**, ~**91 KB of code**, each with
its exact offset, size, and hash. That's the denominator.

## The toolchain (the gate)

**Identified: Borland Turbo C 2.0, large memory model, stack-checking on.**
Byte-identical output needs that exact period compiler + linker + libraries, run
under **DOSBox**.

The evidence, all from the runtime segment `0x20a4` of the unpacked image:

- **The runtime-error printer** at `0x20a4:0x01e0` holds the string
  `"Runtime error \0 at \0.\r\n\0"` — the three null-separated fragments that
  Borland's `C0.ASM` assembles into *"Runtime error N at SSSS:OOOO"*. This exact
  split form is the Turbo C 2.0 / Turbo C++ 1.0-era CRT (Borland C++ 3.x uses a
  larger, differently-worded message set).
- **`__stkcheck`** at `0x20a4:0x0244` is the textbook Borland stack-overflow
  guard: `mov si,sp; sub si,ax; sub si,0x200` (the 512-byte margin) `; cmp si,
  [_stklen]`, aborting with code `0xCA`. Every framed function calls it, so the
  game was built with stack checking **on** (`-N`).
- The CRT helpers sit packed adjacent right after it — `F_SCOPY@` (struct copy,
  `0x025d`) and `LXMUL@` (long multiply, `0x0275`) — the Borland large-model
  library layout. Far calls / `retf` everywhere confirm **large model** (`-ml`).
- It is **straight C**: no C++ name-mangling, exception, or RTTI machinery, which
  rules out the Borland C++ compilers and fits Turbo C 2.0 (1989). Turbo C++ 1.0
  (1990) ships a byte-identical C runtime and is the fallback to try if a
  function won't match.
- Graphics is **BGI** (`VGA256.BGI` / `CGA.BGI`) with the Genus PCX Programmer's
  Toolkit 2.0 linked in (`"Copyright (c) Genus Microprogramming, Inc. 1988-89"`).

Next: install Turbo C 2.0 in DOSBox and find the flag set (`-ml -N`, optimisation
level, `-O`/`-Z` off) that reproduces a few known-simple leaf functions — the
`strlen`, `rand`, and `gotoxy` entries in `config/match_targets.json` are the
smallest — byte-for-byte, then lock those flags in for the rest.

## The DOS runtime (`dosrun.py`)

The period compiler runs under **DOSBox-X**, driven by a small wrapper that acts
like a fake MS-DOS CLI runtime. `tools/dosrun.py` mounts the repo's `dos/` folder
as `C:` (and a Turbo C 2.0 install as `D:`), runs one DOS command, and exits,
propagating success/failure back to the host:

```bash
python3 tools/dosrun.py "DIR C:\\"                        # sanity-check the mount
python3 tools/dosrun.py --check OUT\\BUILD.OK "CALL BUILD.BAT"
```

Success is robust to DOS's lack of host errorlevels: the wrapper deletes the
`--check` sentinel up front and only declares success if the run re-creates it
(`dos/BUILD.BAT` writes `OUT\BUILD.OK` only after a clean compile). One-time
DOSBox-X / Turbo C setup is in [`../dos/README.md`](../dos/README.md).

## Per-function workflow

1. Pick a function from `config/match_targets.json` (start with small, lifted
   ones — the leaf helpers).
2. Write a **compilable** matching version as `match/<function>.c` (stricter than
   the `src/` spec: real types, real prototypes, no stubs).
3. Build and diff — the whole host → DOSBox-X → compare loop is one command:

   ```bash
   xmake match            # stage match/ -> DOS, TCC compiles, extract, diff
   ```

   Under the hood `tools/match_build.py` stages each source under a DOS 8.3 stub,
   `dos/BUILD.BAT` runs `TCC -ml -N -c`, `tools/objtext.py` lifts the `_TEXT`
   bytes out of the resulting `.OBJ` into `build/match/<function>.bin`, and
   `tools/match_compare.py` diffs them against the original — **masking the
   relocated segment words** (link-layout dependent, not part of the code match)
   and reporting matched bytes / byte-exact functions.
4. Iterate on the C and the compiler flags until the function is exact, then move
   on. `xmake dosbuild` does the build+extract without the diff while iterating.

Validate the harness itself any time with:

```bash
python3 tools/match_compare.py --self-test   # -> "HARNESS OK", 218/218 exact
```

## The metric

`matched bytes / total code bytes`, plus the count of byte-exact functions.
Starts at 0%; every matched function moves it. Compiled function bytes go in
`build/match/<name>.bin`; `xmake match` (or `python3 tools/match_compare.py`)
reports progress, and the **"Functions matched byte-exact"** bar on the README
dashboard is driven by the same `match_compare.compute()` call.

## Why relocation masking

Far-call and far-data references store a 16-bit *segment* word that the linker
fixes up; its value depends on where the linker places each segment, not on the
source. The unpacker recovered every fixup offset, so the harness ignores those
2-byte words when diffing — otherwise a perfectly-matching function would look
wrong purely because of link layout.

## Status

- Answer key + denominator: ✅ `config/match_targets.json` (218 fns, 91 KB),
  refreshed by `xmake decomp`.
- Diff harness + masking: ✅ `tools/match_compare.py` (self-test passes), run via
  `xmake match`; feeds the dashboard's matched-bytes bar.
- Toolchain identified: ✅ **Borland Turbo C 2.0**, large model, `-N` (see above).
- Build pipeline: ✅ `dosrun.py` + `dos/BUILD.BAT` + `objtext.py` + `match_build.py`,
  wired to `xmake dosbuild` / `xmake match` (DOSBox-X round-trip verified).
- Toolchain building under DOSBox: ⬜ install TC 2.0, pin the flags on leaf fns.
- Functions matched: 0 / 218.

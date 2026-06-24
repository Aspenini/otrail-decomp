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

The binary is **Borland Turbo C / C++** (large memory model — far pointers
everywhere; `__stkcheck` at every prologue; BGI graphics). Byte-identical output
needs that exact period compiler + linker + libraries, run under **DOSBox**.

First real task: **pin the exact version** (Turbo C 2.0 / Turbo C++ 1.0 / Borland
C++ 2.0, all circa 1990). Fingerprints to use:
- the C startup and `__stkcheck` code in segment `0x20a4`,
- the runtime-library function shapes (compare against known Borland CRTs / IDA
  FLIRT signatures),
- the BGI driver versions (`VGA256.BGI` / `CGA.BGI`).
Once identified, install it in DOSBox and find the flags (memory model `-ml`,
optimisation, stack-check on) that reproduce known-simple functions.

## Per-function workflow

1. Pick a function from `config/match_targets.json` (start with small, lifted
   ones — the leaf helpers).
2. Write a **compilable** matching version (stricter than the `src/` spec: real
   types, real prototypes, no stubs).
3. Compile it with the period compiler under DOSBox, and extract just that
   function's code bytes (from the `.obj` OMF record, or the linked `.exe` at its
   offset) into `build/match/<name>.bin`.
4. Diff:

   ```bash
   python3 tools/match_compare.py        # overall % matched
   ```

   The harness compares your bytes to the original, **masking the relocated
   segment words** (link-layout dependent, not part of the code match). It
   reports matched bytes / total and how many functions are byte-exact.
5. Iterate on the C and flags until the function is exact, then move on.

Validate the harness itself any time with:

```bash
python3 tools/match_compare.py --self-test   # -> "HARNESS OK", 218/218 exact
```

## The metric

`matched bytes / total code bytes`, plus the count of byte-exact functions.
Starts at 0%; every matched function moves it. Once functions land in
`build/match/`, this can feed the progress dashboard alongside the
named/lifted numbers.

## Why relocation masking

Far-call and far-data references store a 16-bit *segment* word that the linker
fixes up; its value depends on where the linker places each segment, not on the
source. The unpacker recovered every fixup offset, so the harness ignores those
2-byte words when diffing — otherwise a perfectly-matching function would look
wrong purely because of link layout.

## Status

- Answer key + denominator: ✅ `config/match_targets.json` (218 fns, 91 KB).
- Diff harness + masking: ✅ `tools/match_compare.py` (self-test passes).
- Toolchain identified + building: ⬜ the next real step.
- Functions matched: 0 / 218.

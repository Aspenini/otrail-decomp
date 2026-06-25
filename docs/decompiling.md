# Decompiling workflow

How functions get recovered in this repo. The goal is **readable, annotated C**
that faithfully mirrors the original, with each statement traceable back to an
address. (Strict byte-for-byte matching against a rebuilt EXE is a later phase;
see the README "Status" section.)

## Setup

```bash
xmake decomp   # unpack -> build/OREGON_unpacked.exe, map, verify, refresh the dashboard
```

This runs the tools in `tools/` in order; you can also invoke them directly
(`python tools/unlzexe.py …`, `python tools/map_segments.py …`).

Everything below works on `build/OREGON_unpacked.exe`. Its load image starts at
file offset `e_cparhdr * 16` (currently `0x3410`) — segment `S`, offset `O` is at
file offset `0x3410 + S*16 + O`. (`tools/verify.py` and `tools/map_segments.py`
read that base from the header; don't hard-code it.)

## Lifting a function

1. **Pick a target.** Use `docs/segment_map.md` for the code segments and
   `config/symbols.json` for already-named entry points. Lifting a caller and
   its callees together is worthwhile — it has repeatedly caught mislabels.

2. **Disassemble it.** From the unpacked image at the right file offset, e.g.:

   ```bash
   ndisasm -b16 -o<OFFSET> <slice.bin>
   ```

   A small Python helper that annotates `mov di,0xNNNN` / `push 0xNNNN` with the
   CONST string at that segment offset makes the intent obvious — most screens
   are driven by their text. Strings are length-prefixed (Pascal-style) and use
   `\` as the newline character.

3. **Read the idioms.**
   - Prologue `55 89 E5 B8 <frame> 9A 44 02 A4 20` = `push bp; mov bp,sp;`
     local frame; `call __stkcheck` (segment `0x20a4`). Ignore the stack check.
   - Far calls are `9A off seg`; the `seg` word is relocated. `push cs; call near`
     is a space-saving far call into the same segment.
   - `0x20a4` is the Borland C runtime: `sprintf`, `printf`-style output, string
     ops, and 32-bit long arithmetic helpers.

4. **Write the C** under `src/`, named `segSSS_<role>.c` (or by `seg_off` when
   the role is unclear). Keep a header comment with the address range and
   provenance, annotate each statement with its `0xADDR`, and quote the CONST
   strings inline. Mark anything summarised rather than traced.

5. **Record symbols.** Add functions and globals to `config/symbols.json` with a
   `confidence` (`high`/`med`/`low`), a `file` pointer once lifted, and a short
   `note`. Keep keys unique (`SSSS:OOOO` for functions, `OOOO` for globals).

6. **Regenerate + check.**

   ```bash
   xmake decomp   # refresh the progress dashboard from the symbol table
   xmake verify   # must stay green — it guards the unpack everything rests on
   ```

## Conventions

- **Names:** descriptive when confident (`choose_profession`, `g_food`),
  `sub_SSSS_OOOO` / `g_OOOO` otherwise. Use the same name for a shared global
  across files.
- **Honesty:** these reconstructions are not yet compile-verified against the
  binary — say so in file headers, and flag summarised regions explicitly.
- **Confidence:** prefer evidence (a recovered string, a cross-checked caller)
  over a plausible guess, and mark low-confidence inferences as such.

## Tools

| Tool                          | Purpose                                       |
|-------------------------------|-----------------------------------------------|
| `tools/unlzexe.py`            | unpack `OREGON.EXE` (LZEXE 0.91)              |
| `tools/map_segments.py`       | rebuild the segment/function map              |
| `tools/render_progress_svg.py`| regenerate `docs/progress.svg`                |
| `tools/verify.py`             | structural regression gate                    |

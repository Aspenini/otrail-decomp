# Matching sources

Compilable, **byte-matching** C - one function per file, named
`match/<function>.c` after a `config/match_targets.json` entry. These are
stricter than the readable spec in `src/`: real Turbo C 2.0 types and prototypes,
no stubs, written so the compiler reproduces the original code exactly.

The pipeline (host stays in control):

```
match/<fn>.c  ──stage──▶  dos/SRC/F##.C  ──TCC under DOSBox-X──▶  dos/OUT/F##.OBJ
                                                                        │ extract _TEXT
                                                                        ▼
        match_compare.py  ◀──diff vs original──  build/match/<fn>.bin
```

Run it with:

```sh
xmake dosbuild     # stage + compile under DOSBox-X (+ extract bytes)
xmake match        # the above, then diff and print % matched
```

See [`../docs/matching.md`](../docs/matching.md) for the toolchain (Turbo C 2.0,
large model, `-N`) and the per-function workflow, and [`../dos/README.md`](../dos/README.md)
for the one-time DOSBox-X / Turbo C setup.

Start with the smallest leaf functions - the `strlen`, `rand`, and `gotoxy`
entries in `config/match_targets.json` - to pin the compiler flags first.

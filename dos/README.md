# DOS build root

This folder is mounted as `C:` inside DOSBox-X by `tools/dosrun.py`. It is the
DOS side of the **matching** workflow: the host stages compilable C into `SRC/`,
DOSBox-X runs Turbo C 2.0 to compile it, and the harness diffs the result against
the original. See [`../docs/matching.md`](../docs/matching.md).

```
dos/
  BUILD.BAT   compile C:\SRC\*.C with Turbo C 2.0 -> C:\OUT\*.OBJ  (tracked)
  SRC/        matching sources, staged from the host's match/ dir  (contents ignored)
  OUT/        compiler output + BUILD.OK sentinel                  (contents ignored)
  INC/        project headers for the matching sources            (tracked)
  TC/         a Turbo C 2.0 install                                (ignored)
```

## One-time setup

1. **DOSBox-X** - `brew install dosbox-x` (macOS), your package manager on Linux,
   or the installer on Windows. `dosrun.py` finds it on `PATH`; otherwise set
   `DOSBOX_X` to the binary.

2. **Turbo C 2.0** - not redistributable, so it is **not** in the repo. Install
   it and either drop it at `dos/TC/` (so `dos/TC/BIN/TCC.EXE` exists) or point
   the `TURBOC` environment variable at the install. It is mounted as `D:`.

That's it - nothing else is committed, and the host stays in control.

## Use

```sh
xmake dosbuild      # stage match/ -> SRC/, run BUILD.BAT under DOSBox-X
xmake match         # the above, then extract code bytes and diff vs the original
```

or drive the runtime directly:

```sh
python3 tools/dosrun.py "DIR C:\\"                       # sanity check the mount
python3 tools/dosrun.py --check OUT\\BUILD.OK "CALL BUILD.BAT"
```

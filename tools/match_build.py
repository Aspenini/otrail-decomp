#!/usr/bin/env python3
"""match_build.py - the matching build pipeline: host -> DOSBox-X -> compare.

Stages the compilable matching sources from `match/` into the DOS root, builds
them with Turbo C 2.0 under DOSBox-X (via dosrun.py + dos/BUILD.BAT), extracts
each function's code bytes from its .OBJ, and diffs against the original image.

    python3 tools/match_build.py               # full pipeline (build + compare)
    python3 tools/match_build.py --build-only   # stage + build + extract, no diff

Matching sources live one-function-per-file as `match/<function>.c`, where
<function> is a name from config/match_targets.json. They are staged under DOS
8.3 stub names (F00.C, F01.C, ...) so long names compile, then mapped back so the
extracted bytes land at build/match/<function>.bin for the harness.

If there are no sources yet, or DOSBox-X / Turbo C are not set up, this prints
guidance and still reports current matching status. See dos/README.md.
"""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MATCH_SRC = ROOT / "match"
DOSROOT = ROOT / "dos"
OUT = ROOT / "build" / "match"
PY = sys.executable


def stage():
    """Copy match/*.c -> dos/SRC/F<NN>.C; return {stub: function_name}."""
    src = DOSROOT / "SRC"
    src.mkdir(parents=True, exist_ok=True)
    for old in src.glob("*.C"):
        old.unlink()
    for old in (DOSROOT / "OUT").glob("*.OBJ"):
        old.unlink()
    sources = sorted(MATCH_SRC.glob("*.c")) if MATCH_SRC.is_dir() else []
    mapping = {}
    for idx, f in enumerate(sources):
        stub = f"F{idx:02d}"
        (src / f"{stub}.C").write_bytes(f.read_bytes())
        mapping[stub] = f.stem
    return mapping


def extract(mapping):
    """Pull _TEXT from each built OBJ into build/match/<function>.bin."""
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    import objtext
    OUT.mkdir(parents=True, exist_ok=True)
    made = 0
    for stub, func in mapping.items():
        obj = DOSROOT / "OUT" / f"{stub}.OBJ"
        if not obj.exists():
            print(f"  ! {func}: no {stub}.OBJ (did it compile?)")
            continue
        code = objtext.extract_text(obj.read_bytes())
        (OUT / f"{func}.bin").write_bytes(code)
        print(f"  + {func}: {len(code)} bytes")
        made += 1
    return made


def main(argv) -> int:
    ap = argparse.ArgumentParser(description="run the matching build pipeline")
    ap.add_argument("--build-only", action="store_true",
                    help="stage + build + extract, skip the comparison")
    args = ap.parse_args(argv)

    mapping = stage()
    if not mapping:
        print("match_build: no sources in match/ yet - nothing to compile.")
        print("             add match/<function>.c (see docs/matching.md), then re-run.")
        if not args.build_only:
            subprocess.run([PY, "tools/match_compare.py"], cwd=ROOT)
        return 0

    print(f"match_build: staged {len(mapping)} source(s); building under DOSBox-X...")
    rc = subprocess.run(
        [PY, "tools/dosrun.py", "--check", "OUT\\BUILD.OK", "CALL BUILD.BAT"],
        cwd=ROOT).returncode
    if rc != 0:
        print("match_build: DOS build did not complete - DOSBox-X or Turbo C not\n"
              "             set up, or a source failed to compile. See dos/README.md.")

    extract(mapping)

    if not args.build_only:
        subprocess.run([PY, "tools/match_compare.py"], cwd=ROOT)
    return rc


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))

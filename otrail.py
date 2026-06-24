#!/usr/bin/env python3
"""
otrail.py - one simple command for the whole Oregon Trail project.

Just run it:

    python otrail.py            # opens a friendly menu - pick a number
    python otrail.py play       # build the port and make a screenshot
    python otrail.py all        # run every decompile step
    python otrail.py help       # list the commands

You do NOT need `make` or any libraries - just Python 3 (and, for building the
port, a C compiler like gcc). Everything writes into the `build/` folder.
"""
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PY = sys.executable or "python3"
GAME = ROOT / "Oregon_The_1990" / "OREGON.EXE"
UNPACKED = ROOT / "build" / "OREGON_unpacked.exe"
BUILD = ROOT / "build"

# ---- pretty output (degrades gracefully on dumb terminals) -----------------
_C = sys.stdout.isatty()
def _c(s, code):  return f"\033[{code}m{s}\033[0m" if _C else s
def bold(s):      return _c(s, "1")
def green(s):     return _c(s, "32")
def red(s):       return _c(s, "31")
def yellow(s):    return _c(s, "33")
def ok(msg):      print(green("  OK  ") + msg)
def info(msg):    print(yellow("  ..  ") + msg)
def fail(msg):    print(red("  !!  ") + msg)


def sh(cmd, label):
    """Run a command from the repo root; return True on success."""
    info(label)
    try:
        subprocess.run(cmd, cwd=ROOT, check=True)
        return True
    except subprocess.CalledProcessError as e:
        fail(f"{label} failed (exit {e.returncode})")
        return False
    except FileNotFoundError:
        fail(f"could not run: {cmd[0]}")
        return False


def have_game():
    if GAME.exists():
        return True
    fail(f"missing game file: {GAME}")
    fail("put the original OREGON.EXE there and try again.")
    return False


def find_cc():
    cc = shutil.which("gcc") or shutil.which("cc") or shutil.which("clang")
    if cc:
        return cc
    for p in (r"C:\mingw64\bin\gcc.exe", "/usr/bin/gcc"):
        if Path(p).exists():
            return p
    return None


# ---- commands ---------------------------------------------------------------
def cmd_unpack():
    """Unpack the original game (LZEXE) -> build/OREGON_unpacked.exe."""
    if not have_game():
        return False
    BUILD.mkdir(exist_ok=True)
    if sh([PY, "tools/unlzexe.py", str(GAME), str(UNPACKED)], "unpacking OREGON.EXE"):
        ok(f"unpacked -> {UNPACKED.relative_to(ROOT)}")
        return True
    return False


def cmd_map():
    """Rebuild the segment / function map from the unpacked image."""
    if not UNPACKED.exists() and not cmd_unpack():
        return False
    if sh([PY, "tools/map_segments.py", str(UNPACKED)], "mapping functions"):
        ok("wrote config/segments.json and docs/segment_map.md")
        return True
    return False


def cmd_verify():
    """Re-unpack and check everything is still correct (the safety net)."""
    if not have_game():
        return False
    if sh([PY, "tools/verify.py"], "verifying the unpacker"):
        ok("all good - the unpack is byte-stable")
        return True
    return False


def cmd_assets():
    """Extract the game's art (images) into build/pcl."""
    if not have_game():
        return False
    (BUILD / "pcl").mkdir(parents=True, exist_ok=True)
    a = sh([PY, "port/assets/pcxlib.py", "Oregon_The_1990/OTMCGA.PCL", "build/pcl"],
           "extracting images from OTMCGA.PCL")
    if a:
        ok("art extracted -> build/pcl/*.png")
    return a


def cmd_svg():
    """Regenerate the progress dashboard (docs/progress.svg)."""
    if sh([PY, "tools/render_progress_svg.py"], "updating the progress dashboard"):
        ok("dashboard -> docs/progress.svg")
        return True
    return False


def cmd_build():
    """Build the playable port (the recomp) with a C compiler."""
    cc = find_cc()
    if not cc:
        fail("no C compiler found. Install gcc (e.g. mingw-w64) and try again.")
        return False
    BUILD.mkdir(exist_ok=True)
    if not sh([PY, "port/assets/make_font.py"], "generating the font"):
        return False
    srcs = sorted(str(p) for p in (ROOT / "port" / "core").glob("*.c"))
    srcs.append(str(ROOT / "port" / "platform" / "file" / "pal_file.c"))
    out = BUILD / ("oregon_trail.exe" if os.name == "nt" else "oregon_trail")
    cmd = [cc, "-std=c99", "-O2", *srcs, "-o", str(out)]
    if sh(cmd, f"compiling the port with {Path(cc).name}"):
        ok(f"built -> {out.relative_to(ROOT)}")
        return True
    return False


def cmd_run(keys=None):
    """Run the port; writes a screenshot into build/."""
    out = BUILD / ("oregon_trail.exe" if os.name == "nt" else "oregon_trail")
    if not out.exists() and not cmd_build():
        return False
    env = dict(os.environ)
    env["OTRAIL_GAMEDIR"] = str(ROOT / "Oregon_The_1990")
    env["OTRAIL_FRAME"] = str(BUILD / "screenshot.png")
    if keys:
        env["OTRAIL_KEYS"] = keys
    info("running the port")
    subprocess.run([str(out)], cwd=ROOT, env=env)
    shots = sorted((BUILD).glob("screenshot_*.png"))
    if shots:
        ok(f"screenshots in build/  (e.g. {shots[-1].relative_to(ROOT)})")
    return True


def cmd_play():
    """Build the port and run it (the fun button)."""
    return cmd_build() and cmd_run()


def cmd_status():
    """Show how far the project has come."""
    import json
    print(bold("\nThe Oregon Trail - project status\n"))
    try:
        seg = json.loads((ROOT / "config" / "segments.json").read_text())
        sym = json.loads((ROOT / "config" / "symbols.json").read_text())
        total = seg.get("function_count", "?")
        named = len(sym.get("functions", {}))
        lifted = sum(1 for v in sym.get("functions", {}).values() if v.get("file"))
        print(f"  unpacked image : {'yes' if UNPACKED.exists() else 'no (run unpack)'}")
        print(f"  functions found: {total}")
        print(f"  named          : {named}")
        print(f"  lifted to C    : {lifted}")
        print(f"  globals named  : {len(sym.get('globals', {}))}")
    except FileNotFoundError:
        info("no map yet - run 'map' first")
    out = BUILD / ("oregon_trail.exe" if os.name == "nt" else "oregon_trail")
    print(f"  port built     : {'yes' if out.exists() else 'no (run build)'}")
    print()
    return True


def cmd_compare():
    """Show how close the rebuild is to byte-matching the original."""
    if not UNPACKED.exists() and not cmd_unpack():
        return False
    sh([PY, "tools/match_inventory.py", str(UNPACKED)], "building the match answer-key")
    sh([PY, "tools/match_compare.py", "--self-test"], "validating the diff harness")
    return sh([PY, "tools/match_compare.py"], "checking byte-match progress")


def cmd_all():
    """Run the whole decompile pipeline: unpack -> map -> verify -> dashboard."""
    return cmd_unpack() and cmd_map() and cmd_verify() and cmd_svg()


COMMANDS = {
    "unpack":  ("Unpack the original game (decompile step 1)", cmd_unpack),
    "map":     ("Build the function map", cmd_map),
    "verify":  ("Check everything still works", cmd_verify),
    "assets":  ("Extract the game art (images)", cmd_assets),
    "svg":     ("Update the progress dashboard", cmd_svg),
    "build":   ("Build the playable port (recomp)", cmd_build),
    "run":     ("Run the port (make a screenshot)", cmd_run),
    "play":    ("Build AND run the port", cmd_play),
    "compare": ("Check byte-match progress vs the original", cmd_compare),
    "status":  ("Show project status", cmd_status),
    "all":     ("Do every decompile step at once", cmd_all),
}
MENU_ORDER = ["unpack", "map", "verify", "assets", "svg",
              "build", "run", "play", "compare", "status", "all"]


def menu():
    print(bold("\n=== The Oregon Trail - decomp & port toolkit ===\n"))
    for i, key in enumerate(MENU_ORDER, 1):
        print(f"  {i:>2}) {COMMANDS[key][0]}")
    print("   0) Quit\n")
    while True:
        try:
            choice = input("Pick a number: ").strip()
        except (EOFError, KeyboardInterrupt):
            print()
            return
        if choice in ("0", "q", "quit", "exit"):
            return
        if choice.isdigit() and 1 <= int(choice) <= len(MENU_ORDER):
            print()
            COMMANDS[MENU_ORDER[int(choice) - 1]][1]()
            print()
        else:
            fail("type one of the numbers shown.")


def usage():
    print(__doc__)
    print(bold("Commands:"))
    for key in MENU_ORDER:
        print(f"  {key:<8} {COMMANDS[key][0]}")


def main(argv):
    if not argv:
        menu()
        return 0
    cmd = argv[0].lower()
    if cmd in ("help", "-h", "--help"):
        usage()
        return 0
    if cmd not in COMMANDS:
        fail(f"unknown command: {cmd}")
        usage()
        return 2
    # `run` accepts an optional key script, e.g. `python otrail.py run " 2   "`
    if cmd == "run" and len(argv) > 1:
        return 0 if cmd_run(argv[1]) else 1
    return 0 if COMMANDS[cmd][1]() else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))

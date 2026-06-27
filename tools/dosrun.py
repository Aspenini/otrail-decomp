#!/usr/bin/env python3
"""dosrun.py - run one DOS command (or batch) under DOSBox-X, repo DOS root mounted.

A tiny "fake MS-DOS runtime" for the matching workflow. It mounts `dos/` as
`C:`, optionally a Turbo C 2.0 install as `D:`, runs a single command, and exits
- propagating success/failure back to the caller (xmake).

    python3 tools/dosrun.py "CALL BUILD.BAT"
    python3 tools/dosrun.py --check OUT/BUILD.OK "CALL BUILD.BAT"
    python3 tools/dosrun.py "DIR C:\\"

Because a DOS program's errorlevel does not reliably reach the host, success is:
the DOSBox-X process exits 0 **and** - if `--check <C:-relative path>` is given -
that file exists and was (re)written by this run. Batch files should `del` their
sentinel up front and re-create it only on success (see dos/BUILD.BAT).

Configuration (environment, all optional):
    DOSBOX_X   path to the dosbox-x binary   (else searched on PATH / app dirs)
    TURBOC     a Turbo C 2.0 install dir, mounted as D:  (else <dosroot>/TC)
    DOSROOT    the DOS C: root                (else <repo>/dos)

Turbo C 2.0 and DOSBox-X are not redistributable; see dos/README.md for setup.
"""
from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path


def repo_root() -> Path:
    p = Path(__file__).resolve().parent
    for d in (p, *p.parents):
        if (d / "xmake.lua").exists() or (d / ".git").exists():
            return d
    return p.parent


def find_dosbox() -> str | None:
    if os.environ.get("DOSBOX_X"):
        return os.environ["DOSBOX_X"]
    for name in ("dosbox-x", "dosbox-x.exe", "dosbox"):
        hit = shutil.which(name)
        if hit:
            return hit
    # common macOS app bundles
    for app in ("/Applications/dosbox-x.app", "/Applications/DOSBox-X.app"):
        exe = Path(app) / "Contents/MacOS/dosbox-x"
        if exe.exists():
            return str(exe)
    return None


def main(argv) -> int:
    ap = argparse.ArgumentParser(description="run a DOS command under DOSBox-X")
    ap.add_argument("command", help='DOS command, e.g. "CALL BUILD.BAT"')
    ap.add_argument("--check", metavar="CPATH",
                    help="C:-relative file that must be (re)written for success")
    ap.add_argument("--timeout", type=int, default=180, help="seconds (default 180)")
    ap.add_argument("--show", action="store_true", help="show the DOSBox-X window")
    args = ap.parse_args(argv)

    root = repo_root()
    dosroot = Path(os.environ.get("DOSROOT", root / "dos")).resolve()
    if not dosroot.is_dir():
        print(f"dosrun: DOS root not found: {dosroot}", file=sys.stderr)
        return 2
    (dosroot / "OUT").mkdir(exist_ok=True)
    (dosroot / "SRC").mkdir(exist_ok=True)

    dosbox = find_dosbox()
    if not dosbox:
        print("dosrun: DOSBox-X not found. Install it (e.g. `brew install dosbox-x`)\n"
              "        or set DOSBOX_X. See dos/README.md.", file=sys.stderr)
        return 3

    turboc = os.environ.get("TURBOC") or (str(dosroot / "TC")
                                          if (dosroot / "TC").is_dir() else None)

    # Build the -c command sequence: mount, switch to C:, run, exit.
    cmds = [f'MOUNT C "{dosroot}"']
    if turboc:
        cmds.append(f'MOUNT D "{turboc}"')
    cmds += ["C:", args.command, "EXIT"]

    cli = [dosbox, "-silent", "-fastlaunch", "-noautoexec", "-exit",
           "-set", "cpu cycles=max"]
    if not args.show:
        cli.append("-nogui")
    for c in cmds:
        cli += ["-c", c]

    # The success contract: remove the sentinel up front, declare success only if
    # the run re-creates it. This sidesteps DOS->host errorlevel and guest-clock
    # mtime issues entirely (mere existence afterwards is the signal).
    check = (dosroot / args.check) if args.check else None
    if check is not None and check.exists():
        check.unlink()

    env = dict(os.environ)
    if not args.show:
        env.setdefault("SDL_VIDEODRIVER", "dummy")   # headless on CI / no display
    cap = not args.show                              # quiet DOSBox's logs on success
    try:
        proc = subprocess.run(cli, timeout=args.timeout, env=env,
                              capture_output=cap, text=True)
    except subprocess.TimeoutExpired:
        print(f"dosrun: timed out after {args.timeout}s", file=sys.stderr)
        return 4
    except FileNotFoundError:
        print(f"dosrun: cannot execute {dosbox}", file=sys.stderr)
        return 3

    ok = proc.returncode == 0
    if check is not None and not check.exists():
        print(f"dosrun: sentinel {args.check} not written - command failed.",
              file=sys.stderr)
        ok = False
    if not ok and cap:                              # surface DOSBox output on failure
        sys.stderr.write(proc.stdout or "")
        sys.stderr.write(proc.stderr or "")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))

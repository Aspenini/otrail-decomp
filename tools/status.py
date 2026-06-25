#!/usr/bin/env python3
"""Print a one-glance project status (used by `xmake status`)."""
import json
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def main():
    print("\nThe Oregon Trail - status\n")
    unpacked = ROOT / "build" / "OREGON_unpacked.exe"
    try:
        seg = json.loads((ROOT / "config" / "segments.json").read_text())
        sym = json.loads((ROOT / "config" / "symbols.json").read_text())
        named = len(sym["functions"])
        lifted = sum(1 for v in sym["functions"].values() if v.get("file"))
        print(f"  unpacked  : {'yes' if unpacked.exists() else 'no (run: xmake decomp)'}")
        print(f"  functions : {seg.get('function_count', '?')} found, "
              f"{named} named, {lifted} lifted to C")
        print(f"  globals   : {len(sym.get('globals', {}))} named")
    except FileNotFoundError:
        print("  (run 'xmake decomp' first)")
    port = ROOT / "build" / ("oregon_trail.exe" if os.name == "nt" else "oregon_trail")
    print(f"  port built: {'yes' if port.exists() else 'no (run: xmake)'}\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())

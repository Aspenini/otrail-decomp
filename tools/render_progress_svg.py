#!/usr/bin/env python3
"""Render docs/progress.svg from config/segments.json + config/symbols.json.

A small, dependency-free progress dashboard for the decompilation, linked at the
top of the README. Re-run via `make svg`; it stays honest as the work grows.
"""
from __future__ import annotations

import json
from collections import Counter

SEG = json.load(open("config/segments.json", encoding="utf-8"))
SYM = json.load(open("config/symbols.json", encoding="utf-8"))

conf = [c for c in SEG["code_segments"] if c["confident"]]
conf.sort(key=lambda c: -c["func_count"])
discovered = SEG["function_count"]
named = len(SYM["functions"])
lifted = sum(1 for v in SYM["functions"].values() if v.get("file"))
nglob = len(SYM["globals"])
img_kb = SEG["image_bytes"] / 1024.0
relocs = SEG["relocations"]

named_per = Counter()
lifted_per = Counter()
for k, v in SYM["functions"].items():
    s = int(k.split(":")[0], 16)
    named_per[s] += 1
    if v.get("file"):
        lifted_per[s] += 1

# ---- theme -----------------------------------------------------------------
BG, PANEL, STROKE = "#161b17", "#1e2620", "#313b32"
TXT, SUB = "#eef1ea", "#97a394"
GREEN, AMBER, MUTE = "#74c365", "#e0a44f", "#3a463b"
FS = "font-family='Segoe UI,Helvetica,Arial,sans-serif'"
FM = "font-family='ui-monospace,Consolas,monospace'"

W = 820
PAD = 28
y = 0
out = []


def rect(x, yy, w, h, r, fill, stroke=None, sw=1):
    s = f" stroke='{stroke}' stroke-width='{sw}'" if stroke else ""
    return f"<rect x='{x}' y='{yy}' width='{w}' height='{h}' rx='{r}' fill='{fill}'{s}/>"


def text(x, yy, s, size, fill, font=FS, anchor="start", weight="400", spacing=None):
    sp = f" letter-spacing='{spacing}'" if spacing else ""
    return (f"<text x='{x}' y='{yy}' {font} font-size='{size}' fill='{fill}' "
            f"text-anchor='{anchor}' font-weight='{weight}'{sp}>{s}</text>")


# ---- header ----------------------------------------------------------------
hdr_h = 92
out.append(rect(0, 0, W, hdr_h, 0, PANEL))
# dotted trail with a tiny covered wagon
ty = 30
out.append(f"<line x1='{PAD}' y1='{ty}' x2='{W-150}' y2='{ty}' stroke='{MUTE}' "
           f"stroke-width='2' stroke-dasharray='2 6' stroke-linecap='round'/>")
wx = W - 170
out.append(f"<g transform='translate({wx},{ty-12})'>"
           f"<path d='M2 14 Q2 2 14 2 L26 2 Q30 2 30 8 L30 14 Z' fill='{AMBER}'/>"
           f"<rect x='0' y='14' width='34' height='4' rx='1' fill='{TXT}'/>"
           f"<circle cx='8' cy='20' r='3.4' fill='none' stroke='{TXT}' stroke-width='1.6'/>"
           f"<circle cx='26' cy='20' r='3.4' fill='none' stroke='{TXT}' stroke-width='1.6'/>"
           f"</g>")
out.append(text(PAD, 56, "THE OREGON TRAIL", 26, TXT, FS, weight="700", spacing="1.5"))
out.append(text(PAD, 78, "decompilation progress &#183; MS-DOS 1990 &#183; LZEXE 0.91 unpacked",
                13, SUB, FS))
y = hdr_h + 22

# ---- stat chips ------------------------------------------------------------
chips = [
    (f"{img_kb:.0f} KB", "unpacked image"),
    (f"{relocs:,}", "relocations"),
    (f"{len(conf)}", "code segments"),
    (f"{discovered}", "functions found"),
]
cw = (W - 2 * PAD - 3 * 12) / 4
for i, (big, small) in enumerate(chips):
    cx = PAD + i * (cw + 12)
    out.append(rect(cx, y, cw, 62, 8, PANEL, STROKE))
    out.append(text(cx + 16, y + 32, big, 24, GREEN, FM, weight="700"))
    out.append(text(cx + 16, y + 50, small, 12, SUB, FS))
y += 62 + 26


# ---- progress bars ---------------------------------------------------------
def progress(label, val, total, color, yy):
    bw = W - 2 * PAD
    frac = max(0.0, min(1.0, val / total if total else 0))
    out.append(text(PAD, yy, label, 14, TXT, FS, weight="600"))
    out.append(text(W - PAD, yy, f"{val} / {total}  ({100*frac:.1f}%)", 14, color,
                    FM, anchor="end", weight="700"))
    by = yy + 10
    out.append(rect(PAD, by, bw, 12, 6, MUTE))
    out.append(rect(PAD, by, max(12, bw * frac), 12, 6, color))
    return by + 30


y = progress("Functions named / identified", named, discovered, AMBER, y)
y = progress("Functions lifted to C", lifted, discovered, GREEN, y)
y += 8

# ---- per-segment breakdown -------------------------------------------------
out.append(text(PAD, y, "BY CODE SEGMENT", 12, SUB, FS, weight="700", spacing="1.2"))
y += 18
bar_x = PAD + 96
bar_w = W - bar_x - 86
maxf = max(c["func_count"] for c in conf)
row_h = 24
for c in conf:
    s = c["segment"]
    fc = c["func_count"]
    nm = named_per.get(s, 0)
    lf = lifted_per.get(s, 0)
    cy = y + 13
    out.append(text(PAD, cy, f"{s:#06x}", 13, TXT, FM))
    full = bar_w * fc / maxf
    out.append(rect(bar_x, y + 3, full, 13, 3, MUTE))
    if nm:
        out.append(rect(bar_x, y + 3, max(3, full * nm / fc), 13, 3, AMBER))
    if lf:
        out.append(rect(bar_x, y + 3, max(3, full * lf / fc), 13, 3, GREEN))
    lab = f"{fc} fns"
    if nm:
        lab += f"  &#183; {nm} named"
    out.append(text(W - PAD, cy, lab, 11, SUB, FM, anchor="end"))
    y += row_h
y += 6

# ---- legend / footer -------------------------------------------------------
out.append(f"<circle cx='{PAD+5}' cy='{y}' r='5' fill='{GREEN}'/>")
out.append(text(PAD + 16, y + 4, "lifted to C", 11, SUB, FS))
out.append(f"<circle cx='{PAD+110}' cy='{y}' r='5' fill='{AMBER}'/>")
out.append(text(PAD + 121, y + 4, "named", 11, SUB, FS))
out.append(f"<circle cx='{PAD+185}' cy='{y}' r='5' fill='{MUTE}'/>")
out.append(text(PAD + 196, y + 4, "discovered", 11, SUB, FS))
out.append(text(W - PAD, y + 4, f"{named} named &#183; {lifted} lifted &#183; {nglob} globals",
                11, SUB, FM, anchor="end"))
y += 24

H = y
svg = (f"<svg xmlns='http://www.w3.org/2000/svg' width='{W}' height='{H}' "
       f"viewBox='0 0 {W} {H}' font-family='sans-serif'>"
       f"{rect(0,0,W,H,12,BG,STROKE)}" + "".join(out) + "</svg>\n")

open("docs/progress.svg", "w", encoding="utf-8").write(svg)
print(f"wrote docs/progress.svg ({W}x{H}); "
      f"named={named} lifted={lifted} discovered={discovered}")

# OTrailDecomp - The Oregon Trail (MS-DOS, 1990) decompilation
#
# OREGON.EXE is LZEXE 0.91-packed. The workflow is:
#   1. unpack   -> build/OREGON_unpacked.exe   (plain relocatable MZ)
#   2. map      -> config/segments.json + docs/segment_map.md
#   3. verify   -> structural regression gate
# Decompilation proceeds on the *unpacked* image (Ghidra/IDA, 16-bit real mode).

PYTHON  ?= python3
SRC      = Oregon_The_1990/OREGON.EXE
UNPACKED = build/OREGON_unpacked.exe

GAMEDIR  = Oregon_The_1990

CC       = gcc

.PHONY: all unpack map svg verify assets port clean help

all: verify map svg

## unpack: decompress OREGON.EXE (LZEXE 0.91) to a plain MZ executable
unpack: $(UNPACKED)

$(UNPACKED): tools/unlzexe.py $(SRC)
	@mkdir -p build
	$(PYTHON) tools/unlzexe.py $(SRC) $(UNPACKED)

## map: rebuild the segment / function map from the unpacked image
map: $(UNPACKED) tools/map_segments.py
	$(PYTHON) tools/map_segments.py $(UNPACKED)

## svg: regenerate docs/progress.svg from the map + symbol table
svg: config/segments.json config/symbols.json tools/render_progress_svg.py
	$(PYTHON) tools/render_progress_svg.py

## verify: re-unpack and check all structural invariants (regression gate)
verify: tools/verify.py tools/unlzexe.py $(SRC)
	$(PYTHON) tools/verify.py

## assets: extract the game art (PCX images + PCXLIB archives) to build/pcl
assets: port/assets/pcxlib.py port/assets/pcx.py
	@mkdir -p build/pcl build/img
	$(PYTHON) port/assets/pcxlib.py $(GAMEDIR)/OTMCGA.PCL build/pcl
	$(PYTHON) port/assets/pcx.py $(GAMEDIR)/LOGO.256 build/img/logo.png

## port: build the recomp (headless 'file' backend) and render the title menu
port: port/core/font8x8.h
	$(CC) -std=c99 -O2 port/core/main.c port/core/pcx.c port/core/screen.c \
	    port/core/title.c port/core/learn.c port/platform/file/pal_file.c -o build/oregon_trail
	OTRAIL_GAMEDIR=$(GAMEDIR) OTRAIL_FRAME=build/port_boot.png ./build/oregon_trail
	@echo "port booted -> build/port_boot_000.png"

port/core/font8x8.h: port/assets/make_font.py
	$(PYTHON) port/assets/make_font.py

## clean: remove generated artifacts
clean:
	rm -f $(UNPACKED) build/_*.bin

## help: list targets
help:
	@grep -E '^## ' $(MAKEFILE_LIST) | sed 's/## //'

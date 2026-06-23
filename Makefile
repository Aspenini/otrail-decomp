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

.PHONY: all unpack map svg verify clean help

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

## clean: remove generated artifacts
clean:
	rm -f $(UNPACKED) build/_*.bin

## help: list targets
help:
	@grep -E '^## ' $(MAKEFILE_LIST) | sed 's/## //'

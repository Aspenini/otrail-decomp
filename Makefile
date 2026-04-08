PYTHON ?= python3
CONFIG ?= config/functions.json
TARGET_EXE ?= Oregon_The_1990/OREGON.EXE

.PHONY: bootstrap materialize verify progress clean

bootstrap:
	@test -f "$(TARGET_EXE)" || (echo "Missing target executable: $(TARGET_EXE)" && exit 1)
	@test -f "$(CONFIG)" || (echo "Missing config file: $(CONFIG)" && exit 1)
	@echo "Bootstrap checks passed."
	@echo "Next step: add a contiguous target unit and iterate with 'make verify'."

materialize:
	@$(PYTHON) tools/materialize_baseline_candidates.py --config "$(CONFIG)"

verify:
	@$(MAKE) materialize
	@$(PYTHON) tools/check_match.py --config "$(CONFIG)"

progress:
	@$(PYTHON) tools/match_progress.py --config "$(CONFIG)"

clean:
	@rm -rf build/*
	@touch build/.gitkeep
	@echo "Cleaned generated artifacts under build/."

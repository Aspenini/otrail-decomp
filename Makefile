PYTHON ?= python3
CONFIG ?= config/functions.json
TARGET_EXE ?= Oregon_The_1990/OREGON.EXE
REPLAY_OFFSET ?= 0x1274F
REPLAY_DCAP ?= 65536
REPLAY_MODE ?= 1
SCAN_MODE ?= 1
CENTER_OFFSET ?= 0x12778
BAND_RADIUS ?= 64
TRACE_OFFSET ?= 0x1274F
TRACE_MAX_EVENTS ?= 100
TRACE_MODE ?= 1
TRACE_WINDOW_START ?= 0x1274F
TRACE_WINDOW_END ?= 0x12845
TRACE_WINDOW_SIZE ?= 0x10
TRACE_BLOCKS_JSON ?= config/entry_unpacker_blocks_1274f_12845.json

.PHONY: bootstrap materialize verify progress unpacker-scan unpacker-replay unpacker-fingerprint unpacker-band unpacker-fixtures unpacker-parity unpacker-trace unpacker-trace-windows unpacker-trace-blocks unpacker-readable-equivalence unpacker-readable-stats clean

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

unpacker-scan:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_scan tools/entry_unpacker_scan.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c
	@build/entry_unpacker_scan "$(TARGET_EXE)" 0x126C0 0x12880 "$(SCAN_MODE)"

unpacker-replay:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_replay tools/entry_unpacker_replay.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c
	@build/entry_unpacker_replay "$(TARGET_EXE)" "$(REPLAY_OFFSET)" "$(REPLAY_DCAP)" "$(REPLAY_MODE)"

unpacker-fingerprint:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_replay tools/entry_unpacker_replay.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c
	@$(PYTHON) tools/entry_unpacker_fingerprint.py --exe "$(TARGET_EXE)" --replay-bin build/entry_unpacker_replay

unpacker-band:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_replay tools/entry_unpacker_replay.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c
	@$(PYTHON) tools/entry_unpacker_confidence_band.py --exe "$(TARGET_EXE)" --replay-bin build/entry_unpacker_replay --center "$(CENTER_OFFSET)" --radius "$(BAND_RADIUS)" --mode "$(REPLAY_MODE)"

unpacker-fixtures:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_replay tools/entry_unpacker_replay.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c
	@$(PYTHON) tools/check_unpacker_fixtures.py --fixtures config/unpacker_fixtures.json --replay-bin build/entry_unpacker_replay

unpacker-parity:
	@$(PYTHON) tools/check_unpacker_mode_parity.py --fixtures config/unpacker_fixtures.json

unpacker-trace:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_trace tools/entry_unpacker_trace.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c
	@build/entry_unpacker_trace "$(TARGET_EXE)" "$(TRACE_OFFSET)" "$(REPLAY_DCAP)" "$(TRACE_MODE)" "$(TRACE_MAX_EVENTS)" "build/unpacker_trace.csv"

unpacker-trace-windows:
	@$(MAKE) unpacker-trace
	@$(PYTHON) tools/entry_unpacker_trace_windows.py --trace-csv build/unpacker_trace.csv --start "$(TRACE_WINDOW_START)" --end "$(TRACE_WINDOW_END)" --window "$(TRACE_WINDOW_SIZE)" --report-out build/unpacker_trace_windows.md

unpacker-trace-blocks:
	@$(MAKE) unpacker-trace
	@$(PYTHON) tools/entry_unpacker_trace_blocks.py --trace-csv build/unpacker_trace.csv --blocks-json "$(TRACE_BLOCKS_JSON)" --csv-out build/unpacker_trace_blocks.csv --report-out build/unpacker_trace_blocks.md

unpacker-readable-equivalence:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_replay tools/entry_unpacker_replay.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_replay_readable tools/entry_unpacker_replay_readable.c logic/entry_unpacker_readable.c
	@$(PYTHON) tools/check_unpacker_readable_equivalence.py --fixtures config/unpacker_fixtures.json --ref-bin build/entry_unpacker_replay --readable-bin build/entry_unpacker_replay_readable

unpacker-readable-stats:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_replay tools/entry_unpacker_replay.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_replay_readable tools/entry_unpacker_replay_readable.c logic/entry_unpacker_readable.c
	@$(PYTHON) tools/report_unpacker_readable_stats.py --fixtures config/unpacker_fixtures.json --ref-bin build/entry_unpacker_replay --readable-bin build/entry_unpacker_replay_readable --report-out build/unpacker_readable_stats_report.md

clean:
	@rm -rf build/*
	@touch build/.gitkeep
	@echo "Cleaned generated artifacts under build/."

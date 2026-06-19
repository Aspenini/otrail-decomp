PYTHON ?= python3
CONFIG ?= config/functions.json
TARGET_EXE ?= Oregon_The_1990/OREGON.EXE
MATCH_FILTER ?=
BOOTSTRAP_LOAD_SEG ?= 0xA000
REPLAY_OFFSET ?= 0x1274F
REPLAY_DCAP ?= 65536
REPLAY_MODE ?= 1
HANDOFF_TRACE_EVENTS ?= 24
WINDOW_TRACE_EVENTS ?= 2201
UNPACKED_PAYLOAD_REPORT ?= build/unpacked_payload_report.md
ENTRY_TAIL_CANDIDATES_REPORT ?= build/entry_tail_candidates_report.md
ENTRY_TAIL_FAMILY_REPORT ?= build/entry_tail_family_report.md
ENTRY_TAIL_MATERIALIZED_REPORT ?= build/entry_tail_materialized_report.md
ENTRY_TAIL_MATERIALIZED_IMAGE ?= build/entry_tail_materialized.bin
MATERIALIZED_ENTRY_RANK_REPORT ?= build/materialized_entry_candidate_rank.md
MATERIALIZED_ENTRY_CANDIDATE_POOL ?= 400
MATERIALIZED_ENTRY_TOPN ?= 16
CANDIDATE_ROUTINE_PREFIX_REPORT ?= build/candidate_routine_prefix_report.md
CANDIDATE_ROUTINE_PREFIX_FIXTURES ?= config/candidate_routine_prefix_fixtures.json
ENTRY_TAIL_IMAGE_CS ?= 0x0D60
ENTRY_TAIL_IMAGE_BX ?= 0x00F0
UNPACKED_WINDOW_REPORT ?= build/unpacked_window_report.md
UNPACKED_WINDOW_SWEEP_REPORT ?= build/unpacked_window_sweep_report.md
UNPACKED_WINDOW_FIXTURES ?= config/unpacked_window_fixtures.json
UNPACKED_CONTRIBUTOR_CHAIN_REPORT ?= build/unpacked_contributor_chain_report.md
UNPACKED_MOTIF_FAMILY_REPORT ?= build/unpacked_motif_family_report.md
UNPACKED_MOTIF_FIXTURES ?= config/unpacked_motif_fixtures.json
UNPACKED_RUNTIME_FIXTURES ?= config/unpacked_runtime_fixtures.json
UNPACKED_RUNTIME_MAP ?= config/unpacked_runtime_map.json
MOTIF_SPECS ?= 0x070B:0x4,0x0DC3:0x3
CHAIN_START ?= 0x2000
CHAIN_SIZE ?= 0x40
WINDOW_START ?= 0x2000
WINDOW_SIZE ?= 0x40
WINDOW_SWEEP_SPECS ?= 0x0818:0x38,0x0F46:0x50,0x2000:0x40,0x2F00:0x40,0x0600:0x60
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
LOADER_FIXTURES ?= config/entry_loader_fixtures.json
ENTRY_HANDOFF_FIXTURES ?= config/entry_handoff_fixtures.json
ENTRY_HANDOFF_TRACE_FIXTURES ?= config/entry_handoff_trace_fixtures.json
ENTRY_TOKEN_FIXTURES ?= config/entry_token_fixtures.json
ENTRY_TAIL_FIXTURES ?= config/entry_tail_fixtures.json
ENTRY_BOOTSTRAP_FIXTURES ?= config/entry_bootstrap_fixtures.json

MATCH_ARGS := $(if $(MATCH_FILTER),--match "$(MATCH_FILTER)",)

.PHONY: bootstrap materialize materialize-authored promote-baseline verify verify-summary progress loader-fixtures loader-readable-equivalence entry-handoff-fixtures entry-handoff-trace entry-handoff-trace-fixtures entry-token-fixtures entry-tail-fixtures unpacked-payload-report unpacked-window-report unpacked-window-sweep unpacked-window-fixtures unpacked-contributor-chain unpacked-motif-family unpacked-motif-fixtures unpacked-runtime-fixtures unpacked-runtime-map entry-tail-candidates entry-tail-family-sweep entry-tail-materialized-image materialized-entry-rank candidate-routine-prefixes candidate-routine-prefix-fixtures entry-bootstrap-replay entry-bootstrap-fixtures entry-bootstrap-readable-equivalence unpacker-scan unpacker-replay unpacker-fingerprint unpacker-band unpacker-fixtures unpacker-parity unpacker-trace unpacker-trace-windows unpacker-trace-blocks unpacker-readable-equivalence unpacker-readable-stats clean

bootstrap:
	@test -f "$(TARGET_EXE)" || (echo "Missing target executable: $(TARGET_EXE)" && exit 1)
	@test -f "$(CONFIG)" || (echo "Missing config file: $(CONFIG)" && exit 1)
	@echo "Bootstrap checks passed."
	@echo "Next step: add a contiguous target unit and iterate with 'make verify'."

materialize:
	@$(PYTHON) tools/materialize_baseline_candidates.py --config "$(CONFIG)" $(MATCH_ARGS) $(MATERIALIZE_ARGS)
	@$(PYTHON) tools/materialize_authored_byte_arrays.py --config "$(CONFIG)" $(MATCH_ARGS) $(MATERIALIZE_ARGS)

materialize-authored:
	@$(PYTHON) tools/materialize_authored_byte_arrays.py --config "$(CONFIG)" $(MATCH_ARGS) $(MATERIALIZE_ARGS)

promote-baseline:
	@$(PYTHON) tools/promote_baseline_units_to_authored.py --config "$(CONFIG)" $(MATCH_ARGS)

verify:
	@$(PYTHON) tools/materialize_baseline_candidates.py --config "$(CONFIG)" $(MATCH_ARGS) $(MATERIALIZE_ARGS)
	@$(PYTHON) tools/materialize_authored_byte_arrays.py --config "$(CONFIG)" $(MATCH_ARGS) $(MATERIALIZE_ARGS)
	@$(PYTHON) tools/check_match.py --config "$(CONFIG)" $(MATCH_ARGS) $(VERIFY_ARGS)

verify-summary:
	@$(MAKE) verify MATCH_FILTER='$(MATCH_FILTER)' MATERIALIZE_ARGS='$(MATERIALIZE_ARGS) --quiet' VERIFY_ARGS='$(VERIFY_ARGS) --quiet-pass'

progress:
	@$(PYTHON) tools/match_progress.py --config "$(CONFIG)" --runtime-map "$(UNPACKED_RUNTIME_MAP)" --runtime-fixtures "$(UNPACKED_RUNTIME_FIXTURES)"

loader-fixtures:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_loader_fixture tools/entry_loader_fixture.c logic/entry_loader_model.c logic/entry_loader_readable.c
	@$(PYTHON) tools/check_entry_loader_fixtures.py --fixtures "$(LOADER_FIXTURES)" --runner build/entry_loader_fixture

loader-readable-equivalence:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_loader_fixture tools/entry_loader_fixture.c logic/entry_loader_model.c logic/entry_loader_readable.c
	@$(PYTHON) tools/check_entry_loader_readable_equivalence.py --fixtures "$(LOADER_FIXTURES)" --runner build/entry_loader_fixture

entry-handoff-fixtures:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_handoff_fixture tools/entry_handoff_fixture.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c src/unit_0002_entrypoint_next_64.c
	@$(PYTHON) tools/check_entry_handoff_fixtures.py --fixtures "$(ENTRY_HANDOFF_FIXTURES)" --runner build/entry_handoff_fixture

entry-handoff-trace:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_handoff_dump_stream tools/entry_handoff_dump_stream.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c src/unit_0002_entrypoint_next_64.c
	@build/entry_handoff_dump_stream "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" "build/entry_handoff_stream.bin"
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_trace tools/entry_unpacker_trace.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c
	@build/entry_unpacker_trace "build/entry_handoff_stream.bin" 0 "$(REPLAY_DCAP)" "$(REPLAY_MODE)" "$(HANDOFF_TRACE_EVENTS)" "build/entry_handoff_trace.csv"

entry-handoff-trace-fixtures:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_handoff_dump_stream tools/entry_handoff_dump_stream.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c src/unit_0002_entrypoint_next_64.c
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_trace tools/entry_unpacker_trace.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c
	@$(PYTHON) tools/check_entry_handoff_trace.py --fixtures "$(ENTRY_HANDOFF_TRACE_FIXTURES)" --dump-runner build/entry_handoff_dump_stream --trace-runner build/entry_unpacker_trace

entry-token-fixtures:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_token_fixture tools/entry_token_fixture.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c src/unit_0002_entrypoint_next_64.c src/unit_0003_entrypoint_contig_64.c src/unit_0004_entrypoint_contig_64.c src/unit_0005_entrypoint_contig_64.c
	@$(PYTHON) tools/check_entry_token_fixtures.py --fixtures "$(ENTRY_TOKEN_FIXTURES)" --runner build/entry_token_fixture

entry-tail-fixtures:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_tail_fixture tools/entry_tail_fixture.c src/unit_0004_entrypoint_contig_64.c src/unit_0005_entrypoint_contig_64.c src/unit_0006_entrypoint_contig_64.c
	@$(PYTHON) tools/check_entry_tail_fixtures.py --fixtures "$(ENTRY_TAIL_FIXTURES)" --runner build/entry_tail_fixture

unpacked-payload-report:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_bootstrap_replay tools/entry_bootstrap_replay.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c
	@build/entry_bootstrap_replay "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" 1 readable >/dev/null
	@$(PYTHON) tools/report_unpacked_payload.py --payload build/entry_bootstrap_replay_readable_heuristic.bin --report-out "$(UNPACKED_PAYLOAD_REPORT)"

unpacked-window-report:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_handoff_dump_stream tools/entry_handoff_dump_stream.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c src/unit_0002_entrypoint_next_64.c
	@build/entry_handoff_dump_stream "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" "build/entry_handoff_stream.bin"
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_trace tools/entry_unpacker_trace.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c
	@build/entry_unpacker_trace "build/entry_handoff_stream.bin" 0 "$(REPLAY_DCAP)" "$(REPLAY_MODE)" "$(WINDOW_TRACE_EVENTS)" "build/entry_handoff_trace.csv"
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_bootstrap_replay tools/entry_bootstrap_replay.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c
	@build/entry_bootstrap_replay "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" 1 readable >/dev/null
	@$(PYTHON) tools/report_unpacked_window.py --payload build/entry_bootstrap_replay_readable_heuristic.bin --trace build/entry_handoff_trace.csv --start "$(WINDOW_START)" --size "$(WINDOW_SIZE)" --report-out "$(UNPACKED_WINDOW_REPORT)"

unpacked-window-sweep:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_handoff_dump_stream tools/entry_handoff_dump_stream.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c src/unit_0002_entrypoint_next_64.c
	@build/entry_handoff_dump_stream "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" "build/entry_handoff_stream.bin"
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_trace tools/entry_unpacker_trace.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c
	@build/entry_unpacker_trace "build/entry_handoff_stream.bin" 0 "$(REPLAY_DCAP)" "$(REPLAY_MODE)" "$(WINDOW_TRACE_EVENTS)" "build/entry_handoff_trace.csv"
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_bootstrap_replay tools/entry_bootstrap_replay.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c
	@build/entry_bootstrap_replay "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" 1 readable >/dev/null
	@$(PYTHON) tools/report_unpacked_window_sweep.py --payload build/entry_bootstrap_replay_readable_heuristic.bin --trace build/entry_handoff_trace.csv --windows "$(WINDOW_SWEEP_SPECS)" --report-out "$(UNPACKED_WINDOW_SWEEP_REPORT)"

unpacked-window-fixtures:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_handoff_dump_stream tools/entry_handoff_dump_stream.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c src/unit_0002_entrypoint_next_64.c
	@build/entry_handoff_dump_stream "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" "build/entry_handoff_stream.bin"
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_trace tools/entry_unpacker_trace.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c
	@build/entry_unpacker_trace "build/entry_handoff_stream.bin" 0 "$(REPLAY_DCAP)" "$(REPLAY_MODE)" "$(WINDOW_TRACE_EVENTS)" "build/entry_handoff_trace.csv"
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_bootstrap_replay tools/entry_bootstrap_replay.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c
	@build/entry_bootstrap_replay "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" 1 readable >/dev/null
	@$(PYTHON) tools/check_unpacked_window_fixtures.py --fixtures "$(UNPACKED_WINDOW_FIXTURES)" --payload build/entry_bootstrap_replay_readable_heuristic.bin --trace build/entry_handoff_trace.csv

unpacked-contributor-chain:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_handoff_dump_stream tools/entry_handoff_dump_stream.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c src/unit_0002_entrypoint_next_64.c
	@build/entry_handoff_dump_stream "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" "build/entry_handoff_stream.bin"
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_trace tools/entry_unpacker_trace.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c
	@build/entry_unpacker_trace "build/entry_handoff_stream.bin" 0 "$(REPLAY_DCAP)" "$(REPLAY_MODE)" "$(WINDOW_TRACE_EVENTS)" "build/entry_handoff_trace.csv"
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_bootstrap_replay tools/entry_bootstrap_replay.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c
	@build/entry_bootstrap_replay "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" 1 readable >/dev/null
	@$(PYTHON) tools/report_unpacked_contributor_chain.py --payload build/entry_bootstrap_replay_readable_heuristic.bin --trace build/entry_handoff_trace.csv --start "$(CHAIN_START)" --size "$(CHAIN_SIZE)" --report-out "$(UNPACKED_CONTRIBUTOR_CHAIN_REPORT)"

unpacked-motif-family:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_handoff_dump_stream tools/entry_handoff_dump_stream.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c src/unit_0002_entrypoint_next_64.c
	@build/entry_handoff_dump_stream "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" "build/entry_handoff_stream.bin"
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_trace tools/entry_unpacker_trace.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c
	@build/entry_unpacker_trace "build/entry_handoff_stream.bin" 0 "$(REPLAY_DCAP)" "$(REPLAY_MODE)" "$(WINDOW_TRACE_EVENTS)" "build/entry_handoff_trace.csv"
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_bootstrap_replay tools/entry_bootstrap_replay.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c
	@build/entry_bootstrap_replay "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" 1 readable >/dev/null
	@$(PYTHON) tools/report_unpacked_motif_family.py --payload build/entry_bootstrap_replay_readable_heuristic.bin --trace build/entry_handoff_trace.csv --motifs "$(MOTIF_SPECS)" --report-out "$(UNPACKED_MOTIF_FAMILY_REPORT)"

unpacked-motif-fixtures:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_handoff_dump_stream tools/entry_handoff_dump_stream.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c src/unit_0002_entrypoint_next_64.c
	@build/entry_handoff_dump_stream "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" "build/entry_handoff_stream.bin"
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_unpacker_trace tools/entry_unpacker_trace.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c
	@build/entry_unpacker_trace "build/entry_handoff_stream.bin" 0 "$(REPLAY_DCAP)" "$(REPLAY_MODE)" "$(WINDOW_TRACE_EVENTS)" "build/entry_handoff_trace.csv"
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_bootstrap_replay tools/entry_bootstrap_replay.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c
	@build/entry_bootstrap_replay "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" 1 readable >/dev/null
	@$(PYTHON) tools/check_unpacked_motif_fixtures.py --fixtures "$(UNPACKED_MOTIF_FIXTURES)" --payload build/entry_bootstrap_replay_readable_heuristic.bin --trace build/entry_handoff_trace.csv

unpacked-runtime-fixtures:
	@cc -O2 -std=c11 -Wall -Wextra -o build/unpacked_runtime_fixture tools/unpacked_runtime_fixture.c logic/unpacked_runtime_fragments.c logic/unpacked_runtime_map.c
	@$(PYTHON) tools/check_unpacked_runtime_fixtures.py --fixtures "$(UNPACKED_RUNTIME_FIXTURES)" --runner build/unpacked_runtime_fixture
	@$(PYTHON) tools/check_unpacked_runtime_map.py --runtime-map "$(UNPACKED_RUNTIME_MAP)" --runner build/unpacked_runtime_fixture

unpacked-runtime-map:
	@cc -O2 -std=c11 -Wall -Wextra -o build/unpacked_runtime_fixture tools/unpacked_runtime_fixture.c logic/unpacked_runtime_fragments.c logic/unpacked_runtime_map.c
	@$(PYTHON) tools/check_unpacked_runtime_map.py --runtime-map "$(UNPACKED_RUNTIME_MAP)" --runner build/unpacked_runtime_fixture

entry-tail-candidates:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_bootstrap_replay tools/entry_bootstrap_replay.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_tail_search tools/entry_tail_search.c src/unit_0004_entrypoint_contig_64.c src/unit_0005_entrypoint_contig_64.c src/unit_0006_entrypoint_contig_64.c
	@build/entry_bootstrap_replay "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" 1 readable >/dev/null
	@$(PYTHON) tools/report_entry_tail_candidates.py --payload build/entry_bootstrap_replay_readable_heuristic.bin --runner build/entry_tail_search --report-out "$(ENTRY_TAIL_CANDIDATES_REPORT)"

entry-tail-family-sweep:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_bootstrap_replay tools/entry_bootstrap_replay.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_tail_search tools/entry_tail_search.c src/unit_0004_entrypoint_contig_64.c src/unit_0005_entrypoint_contig_64.c src/unit_0006_entrypoint_contig_64.c
	@build/entry_bootstrap_replay "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" 1 readable >/dev/null
	@$(PYTHON) tools/report_entry_tail_candidates.py --payload build/entry_bootstrap_replay_readable_heuristic.bin --runner build/entry_tail_search --report-out "$(ENTRY_TAIL_FAMILY_REPORT)" --max-cs-base 0x2000 --bx-start 0x0000 --bx-end 0x0100 --bx-step 0x10

entry-tail-materialized-image:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_bootstrap_replay tools/entry_bootstrap_replay.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c
	@build/entry_bootstrap_replay "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" 1 readable >/dev/null
	@$(PYTHON) tools/materialize_entry_tail_image.py --payload build/entry_bootstrap_replay_readable_heuristic.bin --cs-base "$(ENTRY_TAIL_IMAGE_CS)" --bx-seed "$(ENTRY_TAIL_IMAGE_BX)" --image-out "$(ENTRY_TAIL_MATERIALIZED_IMAGE)" --report-out "$(ENTRY_TAIL_MATERIALIZED_REPORT)"

materialized-entry-rank:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_bootstrap_replay tools/entry_bootstrap_replay.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_tail_search tools/entry_tail_search.c src/unit_0004_entrypoint_contig_64.c src/unit_0005_entrypoint_contig_64.c src/unit_0006_entrypoint_contig_64.c
	@build/entry_bootstrap_replay "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" 1 readable >/dev/null
	@$(PYTHON) tools/rank_materialized_entry_candidates.py --payload build/entry_bootstrap_replay_readable_heuristic.bin --runner build/entry_tail_search --report-out "$(MATERIALIZED_ENTRY_RANK_REPORT)" --candidate-pool "$(MATERIALIZED_ENTRY_CANDIDATE_POOL)" --topn "$(MATERIALIZED_ENTRY_TOPN)"

candidate-routine-prefixes:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_bootstrap_replay tools/entry_bootstrap_replay.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c
	@build/entry_bootstrap_replay "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" 1 readable >/dev/null
	@$(PYTHON) tools/materialize_entry_tail_image.py --payload build/entry_bootstrap_replay_readable_heuristic.bin --cs-base "$(ENTRY_TAIL_IMAGE_CS)" --bx-seed "$(ENTRY_TAIL_IMAGE_BX)" --image-out "$(ENTRY_TAIL_MATERIALIZED_IMAGE)" --report-out "$(ENTRY_TAIL_MATERIALIZED_REPORT)"
	@$(PYTHON) tools/report_candidate_routine_prefixes.py --payload build/entry_bootstrap_replay_readable_heuristic.bin --materialized "$(ENTRY_TAIL_MATERIALIZED_IMAGE)" --report-out "$(CANDIDATE_ROUTINE_PREFIX_REPORT)" --runtime-map "$(UNPACKED_RUNTIME_MAP)"

candidate-routine-prefix-fixtures: candidate-routine-prefixes
	@$(PYTHON) tools/check_candidate_routine_prefixes.py --fixtures "$(CANDIDATE_ROUTINE_PREFIX_FIXTURES)" --runtime-map "$(UNPACKED_RUNTIME_MAP)"

entry-bootstrap-replay:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_bootstrap_replay tools/entry_bootstrap_replay.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c
	@build/entry_bootstrap_replay "$(TARGET_EXE)" "$(BOOTSTRAP_LOAD_SEG)" "$(REPLAY_MODE)"

entry-bootstrap-fixtures:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_bootstrap_replay tools/entry_bootstrap_replay.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c
	@$(PYTHON) tools/check_entry_bootstrap_fixtures.py --fixtures "$(ENTRY_BOOTSTRAP_FIXTURES)" --runner build/entry_bootstrap_replay

entry-bootstrap-readable-equivalence:
	@cc -O2 -std=c11 -Wall -Wextra -o build/entry_bootstrap_replay tools/entry_bootstrap_replay.c logic/entry_bootstrap.c logic/entry_loader_model.c logic/entry_loader_readable.c logic/entry_unpacker_model.c logic/entry_unpacker_engine.c logic/entry_unpacker_readable.c
	@$(PYTHON) tools/check_entry_bootstrap_readable_equivalence.py --fixtures "$(ENTRY_BOOTSTRAP_FIXTURES)" --runner build/entry_bootstrap_replay

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

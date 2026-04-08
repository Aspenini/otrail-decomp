# AI Decompilation Playbook (Matching -> Readable Source)

This document is a **general workflow guide** for AI agents performing software decompilation projects in the style of well-known community efforts (for example, SM64-like workflows).

It is intentionally project-agnostic and can be used as a baseline in new repositories.

## Mission

Recover software into maintainable source code with high behavioral fidelity, using a two-phase strategy:

1. **Phase 1: Matching (verification-first)**
2. **Phase 2: Readable source (maintenance-first)**

Phase 1 proves correctness. Phase 2 enables long-term development, porting, and modding.

## Guiding Principles

- Prefer reproducibility over speed.
- Preserve original behavior before refactoring.
- Keep every recovered unit testable/verifiable.
- Track progress with objective metrics (function count, bytes, section coverage).
- Avoid irreversible/destructive operations unless explicitly requested.

## Recommended Repository Layout

```text
repo/
  README.md
  config/
    functions.json
  src/
    *.c                  # Phase 1 matched functions or equivalent units
  logic/                 # Phase 2 readable implementations
  tools/
    check_match.py
    match_progress.py
  build/                 # generated artifacts
  Makefile               # or equivalent task runner
```

## Phase 1: Matching Workflow

Goal: recover units that compile back to expected machine code (or otherwise pass strict equivalence checks).

### Per-unit loop

1. Identify a clean unit boundary (function/basic region).
2. Extract target bytes/instructions from the original binary.
3. Create a source stub for that unit in `src/`.
4. Add metadata in `config/functions.json`:
   - logical name
   - original binary path/id
   - address/offset
   - size
   - expected bytes (or canonical checksum/signature)
   - output object mapping
5. Build and compare with `tools/check_match.py`.
6. Record pass/fail and iterate until matched.

### Matching metadata contract

Each configured unit should be independently verifiable. If one unit fails, other units should still be checkable.

## Phase 1 Progress Tracking

Provide both:

- **Configured progress**: matched units/bytes out of known configured targets.
- **Whole-binary progress**: matched bytes versus total executable/text section bytes.

Always report:

- Per-binary coverage
- Grand total coverage across all target binaries

This prevents false confidence from showing 100% of a small configured subset.

## Phase 2: Readable Source Migration

Start only after meaningful Phase 1 confidence.

### Migration rules

- Replace matched stubs with readable, typed, documented code.
- Keep behavior equivalent to the verified baseline.
- Preserve tests/checks to detect regressions.
- Introduce module boundaries and naming conventions.
- Isolate platform-specific code behind adapters.

### Practical strategy

- Migrate by subsystem, not random functions.
- Keep a "golden behavior" harness for key paths.
- Prefer small PRs/changesets with clear before/after validation.

## Validation Strategy

Minimum validation stack:

- Unit-level equivalence checks
- Integration smoke tests
- Runtime sanity checks on representative inputs
- Deterministic regression tests where possible

Optional advanced checks:

- Differential tracing between original and rebuilt paths
- Snapshot/fuzz-based behavior comparisons

## Build and Tooling Recommendations

- Use deterministic compiler/linker flags where feasible.
- Keep build commands scripted (`make`, task runner, or CI pipeline).
- Ensure all verification commands are one-command reproducible.
- Store tool assumptions (compiler version, arch, ABI) in docs.

## Agent Operating Rules

For AI agents working this flow:

- Do not rewrite broad areas without verification targets.
- Do not discard existing unrelated workspace changes.
- Prefer incremental, contiguous recovery for easier reasoning.
- Update tracking metadata immediately after each recovered unit.
- Run full verification after substantive batches.
- Report concise status after every batch:
  - units added
  - match result
  - new coverage values
  - next contiguous target

## Definition of Done

A decompilation effort is complete when:

- Phase 1 verification targets meet project threshold (or full target scope),
- Phase 2 readable source is authoritative for rebuilds,
- documented build pipeline reproduces expected behavior on modern systems.

---

Use this playbook as the default workflow unless project-specific constraints require deviations.

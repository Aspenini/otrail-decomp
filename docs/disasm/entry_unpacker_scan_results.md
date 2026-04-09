# Entry Unpacker Harness Results

Command used:

```bash
make unpacker-scan
```

The scan runs the inferred unpacker model against candidate offsets in
`OREGON.EXE` and reports candidates with meaningful decode activity.

## Top candidates (latest run)

Scan window used by default:

- start: `0x126C0`
- end: `0x12880`

From `build/unpacker_scan_latest.txt`:

- `0x126DC`: src `16` -> dst `470` (ratio `29.375`)
- `0x126E0`: src `11` -> dst `249` (ratio `22.636`)
- `0x126DE`: src `13` -> dst `249` (ratio `19.154`)
- `0x126DA`: src `13` -> dst `228` (ratio `17.538`)
- `0x126D0`: src `28` -> dst `488` (ratio `17.429`)

## Interpretation

- Candidate cluster is tightly centered around `0x126D*`, close to entry setup.
- The refined model now reaches `status=ok` for many nearby starts.
- This strongly suggests the control-flow and token interpretation are closer to
  the original routine than the earlier model.

## Next validation step

Anchor scan starts to known control-flow entry points (`0x1271A`, `0x1274F`) and
compare decoded output signatures against runtime-observed destination memory to
disambiguate "algorithm-plausible" from "true execution path" candidates.

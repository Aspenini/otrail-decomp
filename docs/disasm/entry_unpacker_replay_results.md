# Entry Unpacker Replay Results (Targeted Offsets)

This page summarizes single-offset replay runs using:

```bash
make unpacker-replay REPLAY_OFFSET=<offset>
```

Comparison data source: `build/unpacker_replay_compare.csv`

## Candidate set

Tested offsets around the likely decode entry region:

- `0x12738`, `0x12745`, `0x1274F`, `0x12759`, `0x12764`
- `0x1276D`, `0x12778`, `0x1277E`, `0x12785`, `0x12790`

## Results snapshot

| Offset | Status | Src Used | Dst Written | Ratio | Zero Ratio |
|---|---:|---:|---:|---:|---:|
| `0x12778` | ok | 1199 | 3961 | 3.304 | 0.906 |
| `0x12738` | ok | 1263 | 3941 | 3.120 | 0.888 |
| `0x12745` | ok | 1250 | 3789 | 3.031 | 0.885 |
| `0x12759` | ok | 1230 | 3546 | 2.883 | 0.881 |
| `0x12764` | ok | 1219 | 3420 | 2.806 | 0.877 |
| `0x1276D` | ok | 1210 | 3408 | 2.817 | 0.878 |
| `0x12785` | ok | 1186 | 3348 | 2.823 | 0.889 |
| `0x1274F` | ok | 1240 | 3347 | 2.699 | 0.872 |
| `0x1277E` | ok | 1193 | 3326 | 2.788 | 0.879 |
| `0x12790` | ok | 1175 | 3292 | 2.802 | 0.880 |

## Interpretation

- All tested starts in this tight region produce stable `ok` decodes.
- Decode expansion is consistent (roughly 2.7x to 3.3x).
- The best expansion candidates are centered near `0x12778`.
- High zero-byte ratios suggest output is not plain text payload; it may include
  structured buffers or relocation-initialized regions.

## Current working hypothesis

- True decode entry is likely inside `0x12738`-`0x12790` rather than exactly one
  fixed point.
- `0x1274F` remains architecturally meaningful from disassembly, while `0x12778`
  gives strongest output expansion metrics.

Next step: compare replay outputs against traced destination memory (or known
asset buffers) to identify the canonical runtime entry offset.

## Fingerprint ranking update

An automated fingerprint ranker now combines:

- byte-frequency cosine similarity
- fixed-prefix positional similarity

Outputs:

- `build/unpacker_fingerprint.csv`
- `build/unpacker_fingerprint_report.md`

Current top candidate in the tested set:

- `0x12778` (avg similarity `0.9447`)

## Confidence band pass

Using:

```bash
make unpacker-band CENTER_OFFSET=0x12778 BAND_RADIUS=64 REPLAY_MODE=1
```

Latest band result (`build/unpacker_band_report.md`):

- best offset in band: `0x12778`
- tested range: `0x12738` to `0x127B8`
- top nearby alternatives: `0x12738`, `0x12745`, `0x1273D`

After adding preseeded strict history-window modeling (8KB trailing
window to mirror `+0x2000` behavior), strict mode now converges to the same
confidence-band winner (`0x12778`) in the tested range.

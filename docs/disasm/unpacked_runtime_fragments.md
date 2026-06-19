# Unpacked Runtime Fragments

Scope: first readable post-unpack runtime fragments promoted out of reports and
into checked source code.

## Readable Source

- `logic/unpacked_runtime_fragments.h`
- `logic/unpacked_runtime_fragments.c`
- `logic/unpacked_runtime_map.h`
- `logic/unpacked_runtime_map.c`

## Covered Fragments

1. Seed `0x05FF`
   - exact bytes: `C7 C3 00 FA`
   - readable composition:
     - literal `0xC7`
     - copied pair from `0x0506..0x0508`: `C3 00`
     - literal `0xFA`
2. Seed `0x0DC3`
   - exact bytes: `DB 17 75`
   - readable composition:
     - literal `0xDB`
     - literal `0x17`
     - copied byte from `0x0DA8`: `0x75`
3. Aligned block `0x05F0..0x0610`
   - exact bytes:
     - `00 00 00 00 00 FC F3 BF CA CF A3 54 15 EB 06 C7`
     - `C3 00 FA 0A 00 83 00 00 FA 01 74 10 7F 00 00 F9`
   - readable composition:
     - leading zero carrier
     - literal `0xFC`
     - copied motif tail `F3 BF CA`
     - literal span `CF A3 54 15 EB 06`
     - seed `0x05FF`
     - literal span `0A 00 83 00 00 FA 01 74`
     - copied tail `10 7F 00 00`
     - literal `0xF9`
4. Aligned block `0x0DC0..0x0DD0`
   - exact bytes:
     - `00 00 D6 DB 17 75 00 00 00 FD 1C A3 A5 E3 06 00`
   - readable composition:
     - leading zero carrier
     - literal `0xD6`
     - seed `0x0DC3`
     - zero tail from the same short copy family
     - literal span `FD 1C A3 A5 E3`
     - copied pair `06 00`
5. Root window `0x2000` prefix
   - exact bytes: `30 E4 80 7D F6 4A C7 C3 00 FA DB 17 75 4A C7 C3`
   - readable composition:
     - `30 E4` from the tail of the upstream `0x1230` copy
     - literal span `80 7D F6 4A`
     - seed `0x05FF`
     - seed `0x0DC3`
     - short self-copy tail `4A C7 C3`
6. Full upstream family window `0x05A4..0x06A1`
   - exact size: `253` bytes
   - readable composition:
     - traced template for the wider seed-family source window
     - exact `0x05F0..0x0610` block overlaid from the checked block builder
7. First exact downstream copy `0x06B0..0x07AD`
   - exact size: `253` bytes
   - readable composition:
     - exact long-copy clone of `0x05A4..0x06A1`
8. Direct `0x05FF` family source window `0x05E5..0x06AD`
   - exact size: `200` bytes
   - readable composition:
     - fixed `0x05E5` prefix
     - exact `0x05F0..0x0610` block from the checked block builder
     - traced suffix through `0x06AD`
9. Exact downstream copy `0x0FD0..0x1098`
   - exact size: `200` bytes
   - readable composition:
     - exact long-copy clone of `0x05E5..0x06AD`
10. Later `0x05FF` family source window `0x05F5..0x06D6`
   - exact size: `225` bytes
   - readable composition:
     - tail slice of the checked `0x05E5..0x06AD` source window
     - fixed extension through `0x06D6`
11. Exact downstream copy `0x1B7A..0x1C5B`
   - exact size: `225` bytes
   - readable composition:
     - exact long-copy clone of `0x05F5..0x06D6`
12. Full upstream family window `0x0D1A..0x0E05`
   - exact size: `235` bytes
   - readable composition:
     - traced template for the wider `0x0DC3` seed-family source window
     - exact `0x0DC0..0x0DD0` block overlaid from the checked block builder
13. First exact downstream copy `0x1C5F..0x1D4A`
   - exact size: `235` bytes
   - readable composition:
     - exact long-copy clone of `0x0D1A..0x0E05`
14. Enclosing `0x2000` source window `0x1F6D..0x206C`
   - exact size: `255` bytes
   - readable composition:
     - fixed prefix through `0x1FFF`
     - exact `0x2000..0x2040` root window inserted at offset `0x93`
     - fixed suffix through `0x206C`
15. Exact downstream copy `0x21B6..0x22B5`
   - exact size: `255` bytes
   - readable composition:
     - exact long-copy clone of `0x1F6D..0x206C`
16. Root-derived source slice `0x2003..0x2021`
   - exact size: `30` bytes
   - readable composition:
     - exact slice of the checked `0x2000..0x2040` root window starting at offset `+3`
17. Exact downstream copy `0x207E..0x209C`
   - exact size: `30` bytes
   - readable composition:
     - exact long-copy clone of `0x2003..0x2021`
18. Exact source window `0x110E..0x112E`
   - exact size: `32` bytes
   - readable composition:
     - fixed source window behind the later `0x2E90` clone
     - includes the `0x1120..0x1128` slice later reused by `0x2F35..0x2F3D`
19. Exact downstream clone `0x2E90..0x2EB0`
   - exact size: `32` bytes
   - readable composition:
     - exact clone of `0x110E..0x112E`
     - provides the `FC F9 00 00 00 B3 00` opener reused by `0x2F00`
20. Dense post-root window `0x2F00..0x2F40`
   - exact size: `64` bytes
   - readable composition:
     - opener copied from the checked `0x2E90` clone
     - zero carriers and fixed literals for the middle control-looking span
     - explicit short self-copy for `0x2F24..0x2F28`
     - `0x1EB5` contributor fragment overlaid at `0x2F2A`
     - `0x1120` slice reused from the checked `0x110E` source window
21. Enclosing `0x2F00` source neighborhood `0x2EF0..0x2F40`
   - exact size: `80` bytes
   - readable composition:
     - exact slice of the checked `0x2EC0..0x2F40` enclosing window
     - carries the 16-byte prelude directly into the checked `0x2F00` root
22. Enclosing `0x2F00` source neighborhood `0x2EC0..0x2F40`
   - exact size: `128` bytes
   - readable composition:
     - fixed `0x2EC0` prefix
     - exact `0x2F00..0x2F40` root window inserted at offset `0x40`
23. Dense continuation window `0x2F40..0x2F80`
   - exact size: `64` bytes
   - readable composition:
     - leading `0x2BD6` tail fragment
     - `0x1E1D` contributor fragment overlaid at `0x2F45`
     - `0x2D38` contributor fragment overlaid at `0x2F4E`
     - explicit short self-copy for `0x2F54..0x2F57`
     - `0x2961` contributor fragment overlaid at `0x2F57`
     - trailing `27` bytes copied from the checked `0x2EF0` window
24. Contiguous post-root span `0x2F00..0x2F80`
   - exact size: `128` bytes
   - readable composition:
     - exact concatenation of the checked `0x2F00..0x2F40` and `0x2F40..0x2F80` windows
25. Exact long-copy continuation `0x2F80..0x2FC0`
   - exact size: `64` bytes
   - readable composition:
     - `0x2F80..0x2F87` copied from the checked `0x2EF0` window at its `0x2F0E` overlap
     - six-byte zero carrier from `0x2597`
     - short-copy reuse of the checked `0x2EAB` and `0x2EA4` slices inside `0x2E90`
     - explicit local long self-copy for `0x2FA5..0x2FAC`
     - first 20 bytes of the `0x253E..0x263C` long-copy tail
26. Exact long-copy continuation `0x2FC0..0x3000`
   - exact size: `64` bytes
   - readable composition:
     - direct exact template for the next 64 bytes of the same `0x253E..0x263C` long-copy tail
27. Contiguous post-root span `0x2F00..0x3000`
   - exact size: `256` bytes
   - readable composition:
     - exact concatenation of the checked `0x2F00..0x2F80`, `0x2F80..0x2FC0`, and `0x2FC0..0x3000` windows
28. Sparse long-copy continuation `0x3000..0x3040`
   - exact size: `64` bytes
   - readable composition:
     - zero-filled continuation of the same `0x253E..0x263C` long-copy tail
29. Sparse long-copy continuation `0x3040..0x3080`
   - exact size: `64` bytes
   - readable composition:
     - second zero-filled page of that same long-copy continuation
30. Terminal visible tail `0x3080..0x30AA`
   - exact size: `42` bytes
   - readable composition:
     - sparse final tail of the `0x253E..0x263C` long-copy chain
     - ends with the visible non-zero suffix `0E 00 00 00 00 00 00 00 16 00 00 00 67 00`
31. Contiguous post-root span `0x2F00..0x30AA`
   - exact size: `426` bytes
   - readable composition:
     - exact concatenation of the checked `0x2F00..0x3000`, `0x3000..0x3040`, `0x3040..0x3080`, and `0x3080..0x30AA` windows
32. Source envelope `0x0EFF..0x0FAB`
   - exact size: `172` bytes
   - readable composition:
     - fixed 33-byte prefix from `0x0EFF..0x0F20`
     - checked `0x0F20..0x0F96` neighborhood inserted at offset `0x21`
     - fixed 21-byte suffix from `0x0F96..0x0FAB`
33. Exact envelope clone `0x2A6E..0x2B1A`
   - exact size: `172` bytes
   - readable composition:
     - exact clone of the checked `0x0EFF..0x0FAB` source envelope
34. Structured neighborhood `0x0F20..0x0F96`
   - exact size: `118` bytes
   - readable composition:
     - leading `F1 00 00` short-copy seed from `0x0F0B`
     - literal/bootstrap prefix around the local `FF FA BE` self-copy at `0x0F2D..0x0F32`
     - zero carriers from wrapped and low-address long copies
     - literal `B4 35 FB 0E 49 F1` bridge into the dense `0x0F46` root
     - exact checked `0x0F46..0x0F96` root inserted at offset `0x26`
35. Exact dense root `0x0F46..0x0F96`
   - exact size: `80` bytes
   - readable composition:
     - short-copy ingress from local `0x0ED1` and `0x0F26` slices
     - zero carriers before `FD`, `E6`, and `3F`
     - checked `0x067D..0x0685` slice reused from the canonical `0x05F5` window
     - checked `0x077B..0x0784` slice reused from the canonical `0x06B0` window
     - local short-copy tail fragments from `0x0F42`, `0x0EDB`, and `0x0EBA`
36. Second-level source envelope `0x2A3A..0x2B1A`
   - exact size: `224` bytes
   - readable composition:
     - fixed 52-byte prefix from `0x2A3A..0x2A6E`
     - checked `0x2A6E..0x2B1A` envelope clone inserted at offset `0x34`
37. Second-level exact clone head `0x2C7F..0x2D3F`
   - exact size: `192` bytes
   - readable composition:
     - exact copy of `0x2A3A..0x2AFA`, the checked head of the `0x2A3A` source envelope
38. Exact clone neighborhood `0x2A8F..0x2B05`
   - exact size: `118` bytes
   - readable composition:
     - exact clone of the checked `0x0F20..0x0F96` neighborhood
39. Exact clone root `0x2AB5..0x2B05`
   - exact size: `80` bytes
   - readable composition:
     - exact clone of the checked `0x0F46..0x0F96` root
40. Near-clone root `0x2CFA..0x2D4A`
   - exact size: `80` bytes
   - readable composition:
     - first `69` bytes match the checked `0x0F46` root exactly
     - terminal `11` bytes switch to a local tail variant: `00 00 00 CE 61 A3 00 31 F6 31 00`
41. Full root window `0x2000..0x2040`
   - exact bytes:
     - `30 E4 80 7D F6 4A C7 C3 00 FA DB 17 75 4A C7 C3`
     - `D4 68 72 00 00 F8 00 00 00 42 00 00 00 11 F0 EB`
     - `00 00 63 16 73 00 00 00 00 00 00 00 F7 A5 99 00`
     - `00 00 42 EE E7 F0 F8 7E CD BE C7 C3 00 FA DB CC`
   - readable composition:
     - first 16 bytes from the existing prefix helper
     - literal span `D4 68 72`
     - two zero carriers before `F8`
     - three zero carriers before the `0x03ED` fragment
     - `0x03ED` carrier fragment: `42 00 00 00 11 F0 EB 00 00`
     - literal span `63 16 73`
     - five-byte zero carrier from the `0x1F87` family
     - `0x1DCB` carrier fragment: `00 00 F7 A5 99 00`
     - explicit self-copy of `0x2017..0x201A` into `0x2030..0x2033`
     - literal span `EE E7 F0 F8 7E CD BE`
     - explicit self-copy of `0x2006..0x200B` into `0x203A..0x203F`
     - terminal literal `CC`
42. Exact tail-page clone `0x2552..0x2592`
   - exact size: `64` bytes
   - readable composition:
     - exact clone of the checked `0x2FC0..0x3000` long-copy tail page
43. Exact terminal-tail clone `0x1909..0x1933`
   - exact size: `42` bytes
   - readable composition:
     - exact clone of the checked `0x3080..0x30AA` terminal tail
44. Exact terminal-tail clone `0x2612..0x263C`
   - exact size: `42` bytes
   - readable composition:
     - exact clone of the checked `0x3080..0x30AA` terminal tail
45. Dense source window `0x0950..0x09A0`
   - exact size: `80` bytes
   - readable composition:
     - trace-anchored template for the current highest-scoring unchecked dense sibling
     - provenance report shows 40 literal bytes, 30 short-copy bytes, and 10 long-copy bytes
     - includes the dense `0x0960..0x09A0` tail that exact-matches the downstream `0x2758` clone
46. Exact dense-window clone `0x2748..0x2798`
   - exact size: `80` bytes
   - readable composition:
     - exact clone of the checked `0x0950..0x09A0` source window
47. Dense source window `0x2500..0x2550`
   - exact size: `80` bytes
   - readable composition:
     - trace-anchored template for the next high-scoring unchecked dense sibling
     - provenance report shows 26 literal bytes, 27 short-copy bytes, and 27 long-copy bytes
     - no wrapped-copy events in the checked trace overlap
48. Dense source window `0x2670..0x26B0`
   - exact size: `64` bytes
   - readable composition:
     - trace-anchored template for the next long-copy-heavy dense sibling
     - provenance report shows 19 literal bytes, 2 short-copy bytes, and 43 long-copy bytes
     - no wrapped-copy events in the checked trace overlap
49. Dense source window `0x2DF0..0x2E40`
   - exact size: `80` bytes
   - readable composition:
     - trace-anchored template for the no-wrap source window immediately before the checked `0x2E90/0x2F00` family
     - provenance report shows 6 literal bytes, 20 short-copy bytes, and 38 long-copy bytes for its best 64-byte core
50. Dense source window `0x29A0..0x29E0`
   - exact size: `64` bytes
   - readable composition:
     - trace-anchored template for the no-wrap source window leading toward the checked `0x2A3A/0x0F46` family
     - provenance report shows 22 literal bytes, 1 short-copy byte, and 41 long-copy bytes
51. Dense source window `0x2B20..0x2B70`
   - exact size: `80` bytes
   - readable composition:
     - trace-anchored template for the no-wrap source window between the checked `0x2A3A` and `0x2C00` regions
     - provenance report shows 26 literal bytes, 18 short-copy bytes, and 36 long-copy bytes
52. Dense source window `0x2C00..0x2C40`
   - exact size: `64` bytes
   - readable composition:
     - trace-anchored template for the no-wrap source window continuing the `0x2B20/0x2D50` sibling chain
     - provenance report shows 18 literal bytes, 19 short-copy bytes, and 27 long-copy bytes
53. Dense source window `0x2D50..0x2D90`
   - exact size: `64` bytes
   - readable composition:
     - trace-anchored template for the no-wrap source window before the checked `0x2DF0` region
     - provenance report shows 15 literal bytes, 5 short-copy bytes, and 44 long-copy bytes
54. Dense source window `0x1800..0x1840`
   - exact size: `64` bytes
   - readable composition:
     - trace-anchored template for a no-wrap source window with a long-copy head from `0x09B8`
     - provenance report shows 11 literal bytes, 2 short-copy bytes, and 51 long-copy bytes
55. Dense source window `0x2310..0x2350`
   - exact size: `64` bytes
   - readable composition:
     - trace-anchored template for a no-wrap source window after the checked `0x2000` root-derived family
     - provenance report shows 12 literal bytes, 19 short-copy bytes, and 33 long-copy bytes
56. Dense source window `0x3130..0x3170`
   - exact size: `64` bytes
   - readable composition:
     - trace-anchored template for a no-wrap source window beyond the closed `0x2F00..0x30AA` tail
     - provenance report shows 23 literal bytes, 23 short-copy bytes, and 18 long-copy bytes
57. Contiguous checked span `0x0900..0x0B00`
   - exact size: `512` bytes
   - readable composition:
     - fixture-backed table span for the broader `0x0950` dense family neighborhood
     - used to lock the surrounding carrier bytes while smaller windows continue to get semantic names
58. Contiguous checked span `0x1100..0x1200`
   - exact size: `256` bytes
   - readable composition:
     - fixture-backed table span around the checked `0x110E` source window and its dense descendants
     - preserves the full source page context for later semantic splitting
59. Contiguous checked span `0x1700..0x1900`
   - exact size: `512` bytes
   - readable composition:
     - fixture-backed table span including the checked `0x1800` source window
     - captures the wider long-copy-heavy neighborhood for clone-family classification
60. Contiguous checked span `0x1D50..0x1F00`
   - exact size: `432` bytes
   - readable composition:
     - fixture-backed table span over the dense corridor before the root-derived `0x2000` family
     - keeps this carrier/source mix reproducible while remaining provenance is sorted
61. Contiguous checked span `0x2300..0x2500`
   - exact size: `512` bytes
   - readable composition:
     - fixture-backed table span covering the checked `0x2310` source window and nearby dense material
     - expands the post-root runtime spine toward the `0x2500` sibling
62. Contiguous checked span `0x2800..0x2A00`
   - exact size: `512` bytes
   - readable composition:
     - fixture-backed table span preceding the checked `0x2A3A/0x0F46` family
     - gives the `0x29A0` sibling broader checked context
63. Dense source window `0x0850..0x08D0`
   - exact size: `128` bytes
   - readable composition:
     - trace-anchored template for the dense sibling before the checked `0x0900` span
     - provenance report shows 26 literal bytes, 44 short-copy bytes, and 58 long-copy bytes
64. Exact-clone family source window `0x09E0..0x0A7F`
   - exact size: `159` bytes
   - readable composition:
     - trace-anchored template for the shared 159-byte body later cloned at `0x177C` and `0x1D7F`
     - sits inside the broader checked `0x0900..0x0B00` span
65. Exact-clone family member `0x177C..0x181B`
   - exact size: `159` bytes
   - readable composition:
     - exact clone of the checked `0x09E0..0x0A7F` source window
66. Exact-clone family member `0x1D7F..0x1E1E`
   - exact size: `159` bytes
   - readable composition:
     - exact clone of the checked `0x09E0..0x0A7F` source window
67. Dense source window `0x11C0..0x1240`
   - exact size: `128` bytes
   - readable composition:
     - trace-anchored template for the dense sibling straddling the checked `0x1100` span tail
     - provenance report shows 36 literal bytes, 32 short-copy bytes, and 60 long-copy bytes
68. Late corridor gap span `0x2B70..0x2C00`
   - exact size: `144` bytes
   - readable composition:
     - fixture-backed table span closing the gap between the checked `0x2B20` and `0x2C00` windows
69. Late corridor gap span `0x2C40..0x2C7F`
   - exact size: `63` bytes
   - readable composition:
     - fixture-backed table span closing the gap before the checked `0x2C7F` clone head
70. Late corridor gap span `0x2D90..0x2DF0`
   - exact size: `96` bytes
   - readable composition:
     - fixture-backed table span closing the gap between the checked `0x2D50` and `0x2DF0` windows
71. Late corridor gap span `0x2E40..0x2E90`
   - exact size: `80` bytes
   - readable composition:
     - fixture-backed table span closing the gap before the checked `0x2E90/0x2F00` family
72. Terminal payload span `0x30AA..0x3172`
   - exact size: `200` bytes
   - readable composition:
     - fixture-backed table span for the remaining post-`0x30AA` payload tail
73. Post-`0x05A4` checked gap sweep
   - exact size: `3966` newly checked unique bytes across `28` additional fixture cases
   - readable composition:
     - fixture-backed spans close the gaps at `0x07AD..0x0850`, `0x08D0..0x0900`, `0x0B00..0x0D1A`, `0x0E05..0x0EFF`, `0x0FAB..0x0FD0`, and `0x1098..0x1100`
     - fixture-backed spans close the broad middle gaps at `0x1240..0x1700`, `0x1933..0x1B7A`, and `0x209C..0x21B6`
     - fixture-backed spans close root/clone bridges at `0x1900..0x1909`, `0x1C5B..0x1C5F`, `0x1D4A..0x1D50`, `0x1F00..0x1F6D`, `0x206C..0x207E`, and `0x22B5..0x2300`
     - fixture-backed spans close late tiny bridges at `0x2550..0x2552`, `0x2592..0x2612`, `0x263C..0x2670`, `0x26B0..0x2748`, `0x2798..0x2800`, `0x2A00..0x2A3A`, `0x2B1A..0x2B20`, `0x2D4A..0x2D50`, and `0x2EB0..0x2EC0`
     - all `28` post-`0x05A4` gap/bridge cases now dispatch through a reusable sparse-span builder that zero-fills the span and overlays named nonzero runs
     - after this sweep, the only remaining large unchecked contiguous band is the early `0x0000..0x05A4` payload prefix
74. Early payload prefix sweep `0x0000..0x05A4`
   - exact size: `1444` newly checked unique bytes across `3` additional fixture cases
   - readable composition:
     - fixture-backed prefix pages at `0x0000..0x0200`, `0x0200..0x0400`, and `0x0400..0x05A4`
     - all three prefix spans now have named builders that zero-fill the range and overlay explicit block/byte patches
     - includes leading user-facing string/table material such as `gYou may:\\  1.`
     - mostly zero-rich wrapped-copy carrier/data material; this closes checked payload coverage but still needs semantic classification

## Candidate Routine Prefixes

- `0x1803..0x1818` is fixture-checked as a candidate routine prefix inside the
  broader checked `0x1800..0x1840` window.
- The current descriptor records:
  - `CX = 0x0083`
  - `SI = 0`, `DI = 0`
  - far call target `0xE600:0x0006`
  - saves of `AX`, `BX`, and `DX` to `[BP-0x0C]`, `[BP-0x0A]`, and `[BP-0x08]`
- This is intentionally not promoted to a full function boundary yet because
  the following bytes fall back into sparse/data-like material.
- The broader prefix-family scan is checked separately by
  `make candidate-routine-prefix-fixtures`; it currently pins `8` unpacked
  payload hits and `7` selected materialized-image hits.
- Runtime-map containment is checked for the unpacked-payload hits: the family
  appears inside `window_0950`, `window_09e0`, `window_177c`, `window_1800`,
  `window_1d7f`, and `window_2748` coverage rather than as unmapped bytes.

## Fixture Gate

- `config/unpacked_runtime_fixtures.json`
- `config/unpacked_runtime_map.json`
- `tools/unpacked_runtime_fixture.c`
- `tools/check_unpacked_runtime_fixtures.py`
- `tools/check_unpacked_runtime_map.py`
- Make target:
  - `make unpacked-runtime-fixtures`
  - `make unpacked-runtime-map`
- Current regression count:
  - `104` readable runtime cases
  - `9` named dense `0x0F46` map descriptors checked against the JSON runtime map

This is still not enough to call `0x2000` confirmed executable code, but it is
now a broader checked source spine: two seeds, two aligned seed blocks, three
separate fan-out families (`0x05FF`, `0x0DC3`, and the `0x2000`-root-derived
copy chain), the lifted `0x0EFF/0x0F20/0x0F46` family with exact and near-clone
windows in the `0x2A3A/0x2A6E/0x2A8F/0x2AB5/0x2C7F/0x2CFA` region, the
additional dense `0x2F00` runtime family plus its checked continuation through
`0x30AA`, exact tail clones at `0x2552`, `0x1909`, and `0x2612`, the new
`0x0950` dense sibling plus its `0x2748` clone, dense source windows at `0x1800`,
`0x2310`, `0x2500`, `0x2670`, `0x29A0`, `0x2B20`, `0x2C00`, `0x2D50`, `0x2DF0`,
and `0x3130`, named dense siblings at `0x0850` and `0x11C0`, the checked
`0x09E0/0x177C/0x1D7F` exact-clone family, broader checked contiguous spans at
`0x0900`, `0x1100`, `0x1700`, `0x1D50`, `0x2300`, and `0x2800`, late corridor gap
spans through `0x2F00`, the terminal `0x30AA..0x3172` payload span, the post-`0x05A4`
gap sweep, the early `0x0000..0x05A4` payload prefix sweep, and the full `0x2000`
root block all exist in readable checked source rather than only as provenance
reports. Unique fixture-backed unpacked runtime recovery is now complete; the next
work is semantic classification and replacement of broad table spans with named
source, data, clone, and carrier fragments.

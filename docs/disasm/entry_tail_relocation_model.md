# Entry Tail Relocation Model

Scope: packed entry tail `0x127EC-0x12845`.

## Exact Decode

Re-decoding from `0x127EC` yields a coherent post-unpack relocation tail:

```asm
0x127EC  push cs
0x127ED  pop ds
0x127EE  mov si,0x0158
0x127F1  pop bx
0x127F2  add bx,0x0010
0x127F5  mov dx,bx
0x127F7  xor di,di
0x127F9  lodsb
0x127FA  or al,al
0x127FC  jz  0x12814
0x127FE  mov ah,0x00
0x12800  add di,ax
0x12802  mov ax,di
0x12804  and di,0x000F
0x12807  mov cl,0x04
0x12809  shr ax,cl
0x1280B  add dx,ax
0x1280D  mov es,dx
0x1280F  add [es:di],bx
0x12812  jmp 0x127F9
0x12814  lodsw
0x12815  or ax,ax
0x12817  jnz 0x12821
0x12819  add dx,0x0FFF
0x1281D  mov es,dx
0x1281F  jmp 0x127F9
0x12821  cmp ax,0x0001
0x12824  jnz 0x12800
0x12826  mov ax,bx
0x12828  mov di,[0x0004]
0x1282C  mov si,[0x0006]
0x12830  add si,ax
0x12832  add [0x0002],ax
0x12836  sub ax,0x0010
0x12839  mov ds,ax
0x1283B  mov es,ax
0x1283D  xor bx,bx
0x1283F  cli
0x12840  mov ss,si
0x12842  mov sp,di
0x12844  sti
0x12845  jmp far [cs:bx]
```

## Interpreted Semantics

- The tail is not mixed code/data after `0x127EC`; it is a relocation/fixup loop.
- `CS:0x0158` is the current best source for the relocation stream.
- Non-zero stream bytes advance `DI` and relocate a word at `[ES:DI]` by `BX`.
- Zero bytes trigger `LODSW` control words:
  - `0x0000`: paragraph-wrap / segment bump (`DX += 0x0FFF`)
  - `0x0001`: terminal handoff
  - any other word: low byte is reused as the next run-length increment
- The terminal handoff treats `CS:0x0000` as a small header:
  - `[0x0000]`: far-jump offset
  - `[0x0002]`: far-jump segment word to be relocated by `AX`
  - `[0x0004]`: final `SP`
  - `[0x0006]`: final `SS` relative word, relocated by `AX`

## Why This Matters

- The unpacker output blob is not yet the final executable image by itself.
- The `0x127EC` tail applies a second relocation layer before the final far jump.
- Any post-unpack code recovery that ignores this tail will misidentify the first
  real entrypoint and misread the early image layout.

## Current Workflow

- `make unpacked-payload-report`
  - builds the stable readable bootstrap payload report
- `make entry-tail-candidates`
  - searches segment-aligned `CS` placements under a 64K relocation-window
    model and reports the best current unpacked-entry candidates
- `make entry-tail-family-sweep`
  - expands the search across `BX=0x0000..0x0100` and clusters repeated jump
    targets into low-confidence candidate families
- `make entry-tail-materialized-image`
  - materializes a selected candidate after the exact relocation tail and writes
    `build/entry_tail_materialized.bin` plus a disassembly report
- `make materialized-entry-rank`
  - reruns the exact relocation tail for the strongest byte-heuristic candidates
    and ranks the resulting entry disassembly after penalizing sparse/data-like
    instruction streams
  - also scores small local-start offsets after each materialized entry, since
    the tail can land on a header/prefix just before a plausible instruction run
- `make candidate-routine-prefixes`
  - scans the unpacked payload plus the selected materialized image for the
    short routine-prefix shape now fixture-pinned at `0x1803`
- `make candidate-routine-prefix-fixtures`
  - rebuilds the payload/materialized image and checks the expected prefix-family
    offsets and descriptors from `config/candidate_routine_prefix_fixtures.json`
- `make unpacked-window-report WINDOW_START=... WINDOW_SIZE=...`
  - correlates a chosen unpacked payload window with the real handoff trace so
    candidate windows can be judged by provenance instead of raw byte scores
- `make unpacked-window-sweep`
  - compares the current candidate windows in one pass and ranks them by a
    simple provenance heuristic
- `make entry-tail-fixtures`
  - directly checks the exact lifted `unit_0004` tail bootstrap, the
    `unit_0005` relocation loop, and the exact mixed `unit_0006` slice
    decomposition (xor-tail byte, terminal handoff bytes, and first post-jump
    data prefix)

The post-jump payload/data immediately after the terminal handoff is now
classified past the mixed `unit_0006` slice: `unit_0007` through `unit_0026`
are semantic data lifts with descriptors for their file offset, continuation
source, byte count, byte range, zero/non-zero counts, and printable-byte count.
This does not claim executable code in those slices; it replaces anonymous
authored bytes with named post-jump payload structure.

This search is still low confidence, but it replaces guesswork with a repeatable
model and gives the repo a concrete path from “unpacked bytes” to “candidate
runtime entrypoint.”

## Current Families

The corrected wider family sweep, which now includes the exact `unit_0004`
bootstrap before `unit_0005`, currently surfaces the strongest low-confidence
clusters around:

- `0x0820` / `0x081F` / `0x082C`
- `0x2F00` / `0x2EF0`
- `0x0F46` / `0x0F75`
- broad repeated pages at `0x0800`, `0x0600`, and `0x1000`

These are not yet validated entrypoints. They are the current best-ranked
regions produced by the exact tail model under a wider `CS x BX` search.

The first provenance pass over `0x0818..0x0850` weakens the `0x0820` family as
an immediate lift target:

- `47/56` bytes in that window are zero
- the apparent `E9 05 00`, `E9 EE 00`, and `C3` bytes are surrounded by copied
  zero padding
- the window is assembled by `16` events (`5` literals, `5` short backrefs,
  `6` long backrefs), not by a clean literal code block

So `0x0820` remains a candidate, but the denser `0x2F00` and `0x0F46` windows
are the better next provenance targets.

The first sweep report currently ranks the sampled windows this way:

1. `0x2000..0x2040`
2. `0x2F00..0x2F40`
3. `0x0600..0x0660`
4. `0x0F46..0x0F96`
5. `0x0818..0x0850`

That does not prove `0x2000` is executable code, but it does make it the best
current immediate lift target under the repo's present evidence.

The first materialized-entry ranking pass now weakens the current jump-target
families as immediate semantic lifts.  The best post-relocation samples still
look data-heavy: `0x2F00` begins with `cld; stc` followed by repeated
zero-add instructions and implausible far calls, while the `0x0600` family
often starts with `ret` followed by sparse carrier bytes.  This means the next
entry search should use the materialized-image ranking as a guardrail and keep
looking for a better post-tail basis before promoting any of these families to
real runtime code.

The local-start pass does surface one pattern worth tracking rather than
lifting yet: several candidates near `0x1800` contain a plausible short run at
`0x1803`:

```asm
0x1803  mov cx,0x0083
0x1806  xor si,si
0x1808  xor di,di
0x180A  call 0xE600:0x0006
0x180F  mov [bp-0x0C],ax
0x1812  mov [bp-0x0A],bx
0x1815  mov [bp-0x08],dx
```

That still falls back into sparse/data-like bytes immediately afterward, so it
is not enough to call a function boundary.  It is, however, the first better
sub-entry clue for the next materialized search pass.  The bytes and immediate
descriptor for this prefix are now pinned by the `candidate1803prefix` runtime
fixture so later entry-search work can treat it as a stable clue rather than a
loose report note.

`make candidate-routine-prefixes` generalizes that clue into a repeatable scan.
The first report finds this prefix shape as a repeated family rather than a
single coincidence: the unpacked payload has hits at `0x0963`, `0x09E5`,
`0x0A67`, `0x1781`, `0x1803`, `0x1D84`, `0x1E06`, and `0x275B`; the selected
materialized image repeats the same shape at several corresponding offsets,
with one relocated far-call immediate at `0x0A67`.  The checked fixture now pins
`8` unpacked-payload hits and `7` selected-materialized hits; `0x275B` is the
only payload-only instance in the default materialized image.  The same fixture
also pins runtime-map containment for the unpacked-payload hits: the repeated
prefixes sit in the `dense_0950`, `dense_09e0`, `dense_1800`, and `window_2748`
coverage families rather than in unmapped payload.  Byte grouping confirms the
payload uses one exact `21`-byte prefix body for all `8` hits, while the selected
materialized image keeps `6` exact copies and relocates the far-call offset at
`0x0A67`.

The current `0x2000` working notes are tracked separately in:

- `docs/disasm/unpacked_window_2000_notes.md`
- `build/unpacked_contributor_chain_report.md`
- `build/unpacked_motif_family_report.md`
- `docs/disasm/unpacked_motif_family_notes.md`

That note also pins the dominant current contributor ranges that need to be
resolved before the first post-unpack semantic lift. The newer motif-family
pass sharpens that target list:

- `0x070B` is not a true seed; the exact `C7 C3 00 FA` family starts earlier at
  `0x05FF`
- `0x0DC3` remains a real seed for the `DB 17 75` family

So the most defensible next post-unpack lift targets are now `0x05FF` and
`0x0DC3`, not the derivative `0x070B` copy inside the `0x2000` window.

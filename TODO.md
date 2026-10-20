# TODO — PS2-NANA-decomp

*Last updated: 2026-10-20 (Phase 3 in progress)*

`progress.yaml` is the machine-readable source of truth. This file is the
human-readable working plan.

## Current snapshot

```yaml
phase: 3
phase_name: Decompilation foundation
estimated_total_functions: 2348
identified: 190
clean_reconstructed: 123
binary_matched: 0
remaining_estimate: 2225
```

The function count is still an estimate until the Ghidra boundary export
replaces the rizin-based inventory.

## Completed

### Documentation / repository hygiene

- [x] Dump inspection and personal-dump policy established
- [x] ELF analysis: entry point, sections, `.reginfo`, compiler comment
- [x] MWo3 overlay container documented
- [x] `.nkp` / `.rws` signatures identified
- [x] Memory layout and Game Code layout documented
- [x] crt0 chain identified: `ResetEE → bootstrap → _start → main`
- [x] `main` identified @ `0x0013B8B0`
- [x] Codegen catalog extended with verified branch-likely, delay-slot,
custom-register ABI, allocator and memory-loop idioms
- [x] Initial syscall wrapper map added in `src/common/ps2_syscalls.h`
- [x] Raw-file cleanup completed; all five tracked `_raw.c` files removed
- [x] Progress chart generator implemented and documented
- [x] Progress chart restyled as PNG and wired to regenerate on push
- [x] README refreshed; progress snapshot now image-only
- [x] `symbols/functions.txt` converted to TSV v2
- [x] `symbols/README.md` updated for TSV format

### Core game / startup

- [x] crt0 BSS range fixed to `0x001A7900..0x005A3680`
- [x] `Game__Main` reconstructed @ `0x0013B930`
- [x] `Game__Init` reconstructed @ `0x00141D90`
(correction: `0x00146350` was a stale label — needs reclassification)
- [x] `Mode__GetState` reconstructed
- [x] `Mode__GetStatus` reconstructed
- [x] `State__Enable` reconstructed
- [x] `State__CopyOut` reconstructed
- [x] `Mode__Update` reconstructed
- [x] `Scene__Update` reconstructed @ `0x00146040`
- [x] `AssetOverlay__Registry` reconstructed @ `0x001446E0`
- [x] Pad input family reconstructed (11 functions)
- [x] First overlay (JOBFND2R) parsed and documented
- [x] Boot-time handler installer identified @ `0x00101148`

### Sound / scheduler core

- [x] `Sound__ProcessCommand` reconstructed
- [x] Scheduler symbols renamed per Ghidra:
`Schedule__Update`, `Schedule__FrameTick`, `Schedule__TickA`,
`Schedule__TickB`, `Schedule__WalkSlots`
- [x] Duplicate `src/game/schedule_update.c` removed
- [x] `Schedule__Update` reconstructed @ `0x00177F30`
- [x] `Schedule__FrameTick` reconstructed @ `0x00177F00`
- [x] `Schedule__WalkSlots` reconstructed @ `0x00177140`
- [x] `Schedule__Init` reconstructed @ `0x00177FD0`
- [x] `Schedule__OnJobDone` reconstructed @ `0x00177EB0`
- [x] Scheduler primitives reconstructed:
`Job__ForceFlag`, `Schedule__Kick`, `Job__Poll`
- [x] `Job__Handler11` reconstructed @ `0x00176DF0`
- [x] `Job__Create` reconstructed @ `0x00188910`
- [x] `fcn_00188A98` reconstructed — job class resolver
- [x] `fcn_00189830` reconstructed — deferred-job manager accessor
- [x] `fcn_00192D50` reconstructed — fallback job class registration
- [x] Scheduler dependency block reconstructed:
`fcn_00177320`, `FUN_001773A0`, `FUN_001773D0`, `Task__QueuePending`

### Scheduler allocator / memory helpers

- [x] Scheduler intrusive allocator reconstructed:
`FUN_00177550`, `FUN_00177400`, `FUN_00177480`, `FUN_001774D0`
- [x] Memory helpers reconstructed:
`fcn_00129B30` (`memmove`), `fcn_00129CE8` (`memset`),
`fcn_0012D018` (`strncpy`), `fcn_0012CA60` (`strcpy`)

### Task queue

- [x] Task queue wrappers reconstructed:
`FUN_00177650`, `FUN_001776B0`, `FUN_00177710`, `FUN_00177770`,
`FUN_001777D0`, `FUN_00177830`, `FUN_00177890`, `FUN_00177900`
- [x] `Command__Requeue` reconstructed
- [x] `Sound__DeferBgmA` reconstructed
- [x] `Sound__DeferBgmB` reconstructed

### Stream layer

- [x] Stream stage/notify layer reconstructed:
`Stream__StageConfig`, `Stream__StageCmd`, `Stream__StageResume`,
`Stream__Notify`, `Stream__StageParam`, `Stream__Resume`,
`Stream__Control`
- [x] `Stream__CloseWait` reconstructed
- [x] `Sound__WaitCh0..3` aliases added
- [x] Stream accessor/worker-init cluster reconstructed:
`FUN_001A31A0`, `FUN_001A31D0`, `FUN_001A31D8`, `FUN_001A32A0`,
`FUN_001A32B8`, `FUN_001A3340`
- [x] Stream root open/close cluster reconstructed:
`Stream__Open`, `FUN_001A2E70`, `FUN_001A2EE0`, `FUN_001A2F00`
- [x] Subsystem worker init reconstructed:
`fcn_001A4EB8`, `FUN_001A4F08`, `FUN_001A4F38`

### RPC layer

- [x] RPC request cluster reconstructed:
`Rpc__Call`, `Rpc__IsPending`, `FUN_00112AA8`, `FUN_00112B40`,
`FUN_00112C10`, `FUN_00112CA8`
- [x] RPC helper cluster reconstructed:
`FUN_00112D38`, `FUN_00112F58`, `FUN_00112F98`, `FUN_00113008`,
`FUN_00113088`
- [x] RPC completion dispatcher reconstructed:
`FUN_00112D90`
- [x] RPC allocator/dequeue helpers reconstructed:
`fcn_001121A0`, `fcn_00112170`
- [x] Dispatcher slot-index call fixed to pass `req->f[8]` explicitly
- [x] RPC retry wrapper reconstructed @ `0x00113110`
(`FUN_00113110` — bounded retry around `Rpc__Call`, custom ABI 10 args)
- [x] RPC slot-table lookup reconstructed @ `0x00113200`
(`FUN_00113200` — index-to-slot under global semaphore)
- [x] Media path builder reconstructed @ `0x00141FA0`
(`FUN_00141FA0` — host0/cdrom0/hfs/dvd prefixes, `;1` suffix,
separator normalization; NOTE: stale `Config__Parse` comment in
`symbols/functions.txt` flagged for reclassification)

### Custom ABI / format migration

- [x] Custom ABI support added:
`src/common/ee_custom_abi.h`
- [x] Custom-entry stub added:
`src/game/scheduler_abi.S`
- [x] `Task__QueuePending` split into assembly entry stub + C helper
- [x] `symbols/functions.txt` migrated from loose text to TSV v2

## In progress

- [ ] **Phase 3 — Decompilation foundation**
- [x] Main loop reconstructed
- [x] Sound/scheduler command VM largely reconstructed
- [x] RPC request/helper/dispatcher layer reconstructed
- [x] Stream stage/root/close layer reconstructed
- [x] Scheduler dependency block reconstructed
- [x] ABI-specific stubs/compiler-extension layer added
- [ ] Verify custom ABI layer with the real ps2dev toolchain
- [ ] Trace the indirect `jalr` caller of `FUN_00113270`
- [ ] Classify/reconstruct remaining `Schedule__Init` subsystem helpers

## Next — technical priorities

### 1. Verify custom ABI before building more on it

- [ ] Compile a minimal ps2dev test that includes `src/common/ee_custom_abi.h`.
- [ ] Verify `EE_CALL5`, `EE_CALL6` and `EE_CALL8` register constraints.
- [ ] Verify `src/game/scheduler_abi.S` stack layout for `Task__QueuePending`.
- [ ] If the inline-asm path fails, replace it with checked assembly stubs.
- [ ] After verification, update codegen notes with the exact compiler
behavior and remove “target verification pending” warnings.

### 2. RPC response-record content dispatcher — DONE

- [x] `FUN_00113270` reconstructed @ `0x00113270` (was PARTIAL).

Final facts: 25-entry table @ `0x002FFA90` indexed by `cmd - 2` with only
4 handlers (cmd 2 dual-write cached+uncached, cmd 11 324-byte copy-out,
cmd 12 64-byte context copy-out from `$s1`, cmd 23/25/26 clamped copy);
"128-bit copies" were actually LDL/LDR/SDL/SDR (MIPS-III unaligned
64-bit), not LQ/SQ. Tail: slot release in 32-entry table @ `0x001C4998`
plus `fcn_0010e950` signal hypothesis.

Interactions mapped: record pool `0x005A7280`/1088B via `0x001C4A18`;
memmove `fcn_00129b30`; sema pair `0x001C4A28/0x001C4A2C`; stream context
enters as custom-ABI `$s1` (cmd 12).

- [ ] Trace the indirect `jalr` caller of `FUN_00113270` (likely an RPC
handler table registration) to confirm the `$s1` stream-context source.

### 3. Remaining scheduler / subsystem init helpers

- [ ] Classify and reconstruct `fcn_00192AA0`.
- [ ] Classify and reconstruct `fcn_001A4C98`.
- [ ] Classify and reconstruct `fcn_001A0D30`.
- [ ] Classify and reconstruct `fcn_001A6610`.
- [ ] Continue replacing `extern` declarations in `sound.c` with verified
function bodies one family at a time.

### 4. Tooling and code hygiene

- [ ] Add `tools/symbols_lint.py` to validate `symbols/functions.txt`:

- five TSV columns,
- valid hex addresses,
- known status values,
- duplicate address/name detection.

- [ ] Fix the pre-existing implicit declaration warning for `Bgm__StopAll`.
- [ ] Audit remaining direct calls with more than four arguments for custom
ABI usage.
- [ ] Add an ABI convention note to `AGENTS.md` once ps2dev verification is
complete.

## Pending — blocked by Ghidra/export confirmation

- [ ] Confirm function count and boundaries in Ghidra; replace rizin estimate
`2348` with an exported count.
- [ ] Export reliable function sizes into `symbols/functions.txt`.
- [ ] Verify reconstructed functions against full Ghidra decompiles and
promote to `VERIFIED` only after CFG/call/side-effect checks pass.
- [ ] Add `functions.csv`.
- [ ] Add `globals.txt` and `globals.csv`.

## Pending — blocked by target/runtime verification

- [ ] Build a minimal EE syntax/build test with ps2dev.
- [ ] Add linker layout matching the original memory map.
- [ ] Add function-level comparison against original assembly.
- [ ] Run PCSX2 behavioral checks once a buildable subset exists.

## Pending — blocked by dump content

- [ ] Locate and document `.nkp` dispatch tables.
The currently available analysis dump does not include a complete `.nkp`
file set, so this remains blocked until a fuller local dump is inspected.

## Non-goals for the current phase

- [ ] No byte-identical GCC matching yet.
- [ ] No committed ISO/ELF/assets.
- [ ] No proprietary SDK material.
- [ ] No speculative names for helpers whose Ghidra/user labels are absent.

## Phase 3 exit criteria

Phase 3 can be considered complete when:

- [x] RPC response-record content dispatcher @ `0x00113270` reconstructed.
- [ ] Remaining scheduler subsystem init helpers are classified or
reconstructed.
- [ ] Custom ABI layer is verified with ps2dev.
- [ ] `symbols/functions.txt` has reliable boundaries from Ghidra.
- [ ] No raw Ghidra references remain.
- [ ] Reconstruction state is consistently tracked in `progress.yaml`.

## Suggested next commit

- `abi: verify custom t-register call layer` after ps2dev smoke test, or
- `decomp: trace rpc handler caller` (`FUN_00113270` jalr site) if ABI
verification is deferred.
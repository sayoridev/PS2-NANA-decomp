# TODO — PS2-NANA-decomp

*Last updated: 2026-10-19 (Phase 3 in progress)*

Milestones: [progress.yaml](progress.yaml) is the source of truth; this file is
the human-readable mirror.

## Completed

- [x] Custom ABI support added: `src/common/ee_custom_abi.h`, `src/game/scheduler_abi.S`, symbols TSV v2
- [x] Scheduler dependency block reconstructed: `fcn_00177320`, `FUN_001773A0`, `FUN_001773D0`, `Task__QueuePending`
- [x] RPC allocator/dequeue helpers reconstructed: `fcn_001121A0`, `fcn_00112170`; dispatcher slot-index call fixed
- [x] `FUN_00112D90` reconstructed (RPC completion dispatcher)
- [x] RPC helper cluster reconstructed: `FUN_00112D38`, `FUN_00112F58`, `FUN_00112F98`, `FUN_00113008`, `FUN_00113088`
- [x] RPC request cluster reconstructed: `Rpc__Call`, `Rpc__IsPending`, `FUN_00112AA8`, `FUN_00112B40`, `FUN_00112C10`, `FUN_00112CA8`
- [x] Stream root open/close cluster reconstructed: `Stream__Open`, `FUN_001A2E70`, `FUN_001A2EE0`, `FUN_001A2F00`
- [x] Stream accessor/worker-init cluster reconstructed: `FUN_001A31A0`, `FUN_001A31D0`, `FUN_001A31D8`, `FUN_001A32A0`, `FUN_001A32B8`, `FUN_001A3340`
- [x] `Stream__CloseWait` reconstructed; `Sound__WaitCh0..3` aliases added
- [x] Stream stage/notify layer reconstructed: `Stream__StageConfig`, `Stream__StageCmd`, `Stream__StageResume`, `Stream__Notify`, `Stream__StageParam`, `Stream__Resume`, `Stream__Control`, `Sound__DeferBgmB`, `Command__Requeue`, `Sound__DeferBgmA`
- [x] Progress chart restyled as PNG and wired to regenerate on push
- [x] Subsystem worker init reconstructed: `fcn_001A4EB8`, `FUN_001A4F08`, `FUN_001A4F38`
- [x] Conditional subsystem init/fallback reconstructed: `fcn_0019F788`, `fcn_001A39E8`
- [x] `fcn_00192D50` reconstructed (fallback job class registration)
- [x] Subsystem handle accessors reconstructed: `fcn_0018A9E0`, `FUN_0018A9F0`, `FUN_0018A9F8`
- [x] `fcn_0017CCF0` reconstructed (scheduler subsystem context init)
- [x] Dump inspection and personal-dump policy (never commit the dump)
- [x] ELF analysis: entry, sections, `.reginfo`, compiler comment
- [x] First-pass function inventory with rizin (`aaa` + `afl`)
- [x] `.nkp` / `.rws` signatures identified
- [x] MWo3 overlay container documented
- [x] Documented memory layout and Game Code layout
- [x] Identified crt0 chain: `ResetEE → bootstrap → _start → main`
- [x] Identified `main` @ `0x0013B8B0`
- [x] Identified main-loop address region
- [x] `Game__Main` reconstructed @ `0x0013B930`
- [x] `Game__Init` reconstructed @ `0x00146350`
- [x] First overlay (JOBFND2R) parsed and documented
- [x] Boot-time handler installer identified @ `0x00101148`
- [x] `Mode__GetState` reconstructed
- [x] `Mode__GetStatus` reconstructed
- [x] `State__Enable` reconstructed
- [x] `State__CopyOut` reconstructed
- [x] `Mode__Update` reconstructed
- [x] `Scene__Update` reconstructed @ `0x00146040`
- [x] `AssetOverlay__Registry` reconstructed @ `0x001446E0`
- [x] Pad input family reconstructed (11 functions)
- [x] `Sound__ProcessCommand` reconstructed
- [x] `Schedule__OnJobDone` reconstructed @ `0x00177EB0`
- [x] Scheduler primitives reconstructed: `Job__ForceFlag`, `Schedule__Kick`, `Job__Poll`
- [x] `Schedule__Init` reconstructed @ `0x00177FD0`
- [x] `Schedule__WalkSlots` reconstructed @ `0x00177140`
- [x] Scheduler symbols renamed per Ghidra (`Schedule__Update`, `Schedule__FrameTick`, `Schedule__TickA`, `Schedule__TickB`, `Schedule__WalkSlots`)
- [x] Duplicate `src/game/schedule_update.c` removed
- [x] `Job__Create` reconstructed @ `0x00188910`
- [x] `fcn_00189830` reconstructed (deferred-job manager accessor)
- [x] `fcn_00188A98` reconstructed (job class resolver)
- [x] `Job__Handler11` reconstructed @ `0x00176DF0`
- [x] Task queue wrappers reconstructed: `FUN_00177650`, `FUN_001776B0`, `FUN_00177710`, `FUN_00177770`, `FUN_001777D0`, `FUN_00177830`, `FUN_00177890`, `FUN_00177900`
- [x] Scheduler intrusive allocator reconstructed: `FUN_00177550`, `FUN_00177400`, `FUN_00177480`, `FUN_001774D0`
- [x] Memory helpers reconstructed: `fcn_00129B30` (`memmove`), `fcn_00129CE8` (`memset`)
- [x] `fcn_0012D018` reconstructed (`strncpy` semantics)
- [x] `fcn_0012CA60` reconstructed (`strcpy` semantics)
- [x] Raw-file cleanup completed; all five tracked `_raw.c` references removed after clean replacements were verified
- [x] Progress chart generator implemented and documented (`tools/progress_chart.py`)
- [x] Codegen catalog extended with verified branch-likely, custom-register ABI, delay-slot and memory-loop idioms
- [x] Initial syscall wrapper map added in `src/common/ps2_syscalls.h`

## In progress

- [ ] **Phase 3 — Decompilation foundation**
- [x] crt0 / startup chain reconstructed
- [x] BSS range fixed to `0x001A7900..0x005A3680`
- [x] Main loop reconstructed
- [x] Sound/scheduler command VM largely reconstructed
- [x] Complete remaining scheduler dependencies (`fcn_00177320`, `Task__QueuePending`, immediate subsystem init helpers)
- [x] ABI-specific stubs/compiler-extension layer added for custom `$t0-$t3` calls (target verification pending)

## Pending — blocked by Ghidra/export confirmation

- [ ] Confirm function count and boundaries in Ghidra; replace rizin estimate `2348` with an exported count.
- [ ] Export/update `symbols/functions.txt` from Ghidra with reliable function sizes.
- [ ] Verify `Schedule__Update`, `Schedule__WalkSlots`, `Schedule__Init`, `Job__Create` and allocator helpers against full Ghidra decompile; promote to `VERIFIED` only after CFG/call/side-effect checks pass.
- [ ] Add `functions.csv`, `globals.txt` and `globals.csv` after the Ghidra boundary pass is exported.

## Pending — blocked by dump content

- [ ] Locate and document `.nkp` dispatch tables. The current uploaded analysis dump contains no `.nkp` file, only a minimal subset including `BGM_002.RWS`; complete this after a fuller local dump is available.

## Pending — next analysis targets

- [ ] Reconstruct ABI-specific scheduler initializer `fcn_00177320` after choosing a safe custom-call strategy.
- [ ] Reconstruct `Task__QueuePending` after solving its seven-argument `$a0-$a3/$t0-$t2` convention.
- [ ] Classify `fcn_00192AA0`, `fcn_00192D50`, `fcn_0019F788`, `fcn_001A39E8`, `fcn_00177320`, `fcn_0017CCF0` called by `Schedule__Init`.
- [ ] Classify `fcn_00177EB0` rename confidence against Ghidra project state (currently `Schedule__OnJobDone` hypothesis supported by flag writer/reader pair).
- [ ] Continue replacing remaining `extern` declarations in `sound.c` with verified function bodies one module at a time.

## Next recommended actions

1. Provide or run Ghidra/rizin disassembly for `fcn_00177320` and `Task__QueuePending` so the custom ABI strategy can be finalized safely.
2. Import/inspect a fuller local dump containing `.nkp` assets before attempting NKP dispatch-table analysis.
3. Keep scheduler reconstructions at `RECONSTRUCTED` until the Ghidra decompile comparison is performed.

## Suggested commit sequence

- `decomp: reconstruct scheduler memory helpers` (previous delivered batch)
- `todo: complete raw cleanup and scheduler documentation baseline`
- Then resume normal decomp on ABI-specific scheduler initializers.
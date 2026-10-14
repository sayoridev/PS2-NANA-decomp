# Analysis — Game__MainLoop Body Classification

_Game__MainLoop @0x141F04 iterates ~13 fixed calls. Tracked here as they are
identified. Evidence: rizin disassembly; see symbols/functions.txt._

| # | Address | Name | Status | Notes |
| --- | --- | --- | --- | --- |
| 1 | 0x0014E8E0 | `Time__Tick` | ✅ RECONSTRUCTED | frame counter @0x71FDE8, cap 0x0CDFD7F0 — see src/system/time.c |
| 2 | 0x00142390 | `System__FrameService` | hypothesis | lazy hook install; per-frame callback @0x5A3A78; counters @0x5A3A70/0x5A3A74; calls 0x1314E0 |
| 3 | 0x0012B968 | `Random__Next` | ✅ RECONSTRUCTED | Knuth LCG a=6364136223846793005, c=1 — see src/system/random.c |
| 4 | 0x0013BDE0 | `Mode__Update` | hypothesis | state machine on struct @0x5B2F00; big switch on field+0x130 (cases 0x4D,0x4C,0x48,0x47,0x46,0x2A,0x29,0x28); uses Memory__Memset |
| 5 | 0x00142510 | func_00142510 | pending |  |
| 6 | 0x001424C0 | func_001424C0 | pending |  |
| 7 | 0x001424B0 | func_001424B0 | pending |  |
| 8 | 0x0013B310 | func_0013B310 | pending |  |
| — | 0x001425D0 | (first-iteration only) | pending | one-shot inside loop |
| 9 | 0x00177F30 | func_00177F30 | pending |  |
| 10 | 0x00142570 | func_00142570 | pending |  |
| 11 | 0x00177F00 | func_00177F00 | pending |  |
| 12 | 0x00146040 | func_00146040 | pending |  |
| 13 | 0x00142700 | func_00142700 | pending |  |
| 14 | 0x00134D50 | `Display__EndFrame` | hypothesis | struct @0x5B1AD0; fn-ptr call at struct+0 (a0=0); helpers 0x132680/0x1345E0/0x1347F0/0x132630/0x134790; also called by Game__Main init (0x141D90) |

## Related discoveries (2026-10-14)

- `g_SystemContext` @ **0x001C7FD0** — central system struct pointer.
Known offsets: `+0x3C` exit final callback, `+0x148` exit-handler list head,
`+0xA8` PRNG state. Node layout in exit list: `{next, count, funcs[count]}`.
- `System__Exit` (0x127DE0, was "BootTail"): runs exit-callback lists, then
`j 0x100218` back into crt0 epilogue. External callers: 0x1289D8, 0x128B20,
0x128BA4, 0x12C054, 0x12C0E8, 0x12E99C, 0x12EB60.
- BootInit1 tail: zeroes 0x1C6048/0x1C604C and memsets **0x5AB8C8 × 0x200** —
arena allocator init (0x10-byte slots). `fcn.00129CE8` = memset-like
(`(dst, 0, n)`); `fcn.00129B30` similar memory op. `Memory__ArenaPush`
(0x11A9D8) / `Memory__ArenaIndex` (0x11AA58, masks handle & 0x7FFFFFFF).
- MWo3 loader NOT yet found — candidates: Game__Main init calls
(0x141D90, 0x13BB80).

## Struct map emerging (gp-relative and absolute)

| Address | Hypothesis | Evidence |
| --- | --- | --- |
| 0x005A3A70 | perf counter A | `sw v1, -0x7B00(gp)` +1/frame |
| 0x005A3A74 | perf counter B | `sw v0, -0x7AFC(gp)` +1/frame |
| 0x005A3A78 | per-frame callback ptr | cached result of fcn.00101FA0, `jalr` per frame |
| 0x005A3A80 | boot param slot | crt0 a3 arg |
| 0x005B1AD0 | display context | Display__EndFrame struct; fn-ptr at +0 |
| 0x005B2F00 | game state / mode struct | Mode__Update; fields +0x12C..+0x168, switch on +0x130 |

Decodes flagged as rizin artifacts (verify in Ghidra): `ext s0, sp, 0, 1`,
`xori.b w0, w0, 0xb0`, `mult ac3, ...`, `dpa.w.ph ac0, sp, s3` (likely MMI).

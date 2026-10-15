# Analysis — Game__MainLoop Body Classification

_Game__MainLoop @0x141F04 iterates 14 fixed calls — ALL CLASSIFIED ✅
(2026-10-14). Game__Main reconstructed in src/game/main.c._

| # | Address | Name | Status | Notes |
| --- | --- | --- | --- | --- |
| 1 | 0x0014E8E0 | `Time__Tick` | ✅ RECONSTRUCTED | frame counter @0x71FDE8, cap 0x0CDFD7F0 — see src/system/time.c |
| 2 | 0x00142390 | `System__FrameService` | hypothesis | lazy hook install; per-frame callback @0x5A3A78; counters @0x5A3A70/0x5A3A74; calls 0x1314E0 |
| 3 | 0x0012B968 | `Random__Next` | ✅ RECONSTRUCTED | Knuth LCG a=6364136223846793005, c=1 — see src/system/random.c |
| 4 | 0x0013BDE0 | `Mode__Update` | hypothesis | state machine on struct @0x5B2F00; big switch on field+0x130 (cases 0x4D,0x4C,0x48,0x47,0x46,0x2A,0x29,0x28); uses Memory__Memset |
| 5 | 0x00142510 | `State__UpdatePair` | hypothesis | fcn.0013B960(0,&0x5B3E60) + fcn.0013B960(1,&0x5B3E50) |
| 6 | 0x001424C0 | `Mode__GetStatus` | ✅ RECONSTRUCTED | src/game/mode.c — field @+2, 10 xrefs |
| 7 | 0x001424B0 | `Mode__GetState` | ✅ RECONSTRUCTED | src/game/mode.c — field @+0xA, 32 xrefs |
| 8 | 0x0013B310 | `Draw__SetContext2` | hypothesis | ctx accessors 0x1347F0(2)/0x134790(2,val); value from 0x13B780 |
| — | 0x001425D0 | `System__InitFrameHook` | ✅ RECONSTRUCTED | first-iteration one-shot; installs default hook 0x1A8FE0 |
| 9 | 0x00177F30 | `Schedule__Update` | hypothesis | task scheduler: struct @0x8D3900 (0x4D14 = 64 slots x 0x134 (wrap at 0x40, corrected via Task__Execute)), countdown @base, cb 0x177C80/0x177CCB0 |
| 10 | 0x00142570 | `System__RunFrameHook` | partial | src/system/framehook.c — gate/flag/callback slots |
| 11 | 0x00177F00 | `Schedule__FrameTick` | hypothesis | calls 0x192C08; global frame counter @0x8DF810++ |
| 12 | 0x00146040 | `Scene__Update` | hypothesis | state machine @0x5BB880; jump table @0x1B9E20; counters +0xBC; countdown +0xE0; switch on +4 (11+ cases) |
| 13 | 0x00142700 | `System__DispatchPending` | partial | consumes command id @0x5A2A24; buffers 0x301E90/0x301EA0 |
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

## Scheduler module map

| Address | Hypothesis | Notes |
| --- | --- | --- |
| 0x008D3900 | `Schedule__g_Context` | 0x4D14 bytes = 64 slots x 0x134 (wrap at 0x40, corrected via Task__Execute); timer at base |
| 0x008DF810 | frame counter | +1 per frame (Schedule__FrameTick) |
| 0x00192BC8 / 0x00192C08 | scheduler internals | called by #9 / #11 |
| 0x00177C80 / 0x00177CCB0 | task callback helpers | invoked when timer expires |

## Graphic context accessors

| Address | Hypothesis | Notes |
| --- | --- | --- |
| 0x001347F0 | `Draw__BeginContext(n)` | called in #8 and Display__EndFrame |
| 0x00134790 | `Draw__EndContext(n, val)` | pair of the above |
| 0x00132680 / 0x001345E0 / 0x001347F0 / 0x00132630 / 0x00134790 | display helpers | Display__EndFrame chain |

## Frame hook struct (verified in code)

| Offset | Field | Address |
| --- | --- | --- |
| +0 | gate | 0x5A3A64 |
| +4 | flag | 0x5A3A68 |
| +8 | callback | 0x5A3A6C |

Default handler: **0x001A8FE0**. Reconstructed in src/system/framehook.c.

## Mode state structs

- g_ModeStateA @0x5B3E60: 8 x u16. Getters: +2 (Mode__GetStatus),
+0xA (Mode__GetState). State__CopyOut exports all 16 bytes.
- g_ModeStateB @0x5B3E50: sibling block, same updater family.

## Game__Init (@0x141D90) — subsystem map

Boot order so far:

1. `System__EarlyInit` (0x141B50): low-level inits (0x111EE0/0x116BD0/
0x113A40/0x11D5E0/0x11DDB8), config parse via 0x141FA0 (buffers
0x301C00/0x301C20), RNG seeded with first Random__Next().
2. `Game__Init` (0x141D90): Memory__InitRegions, Schedule__Init,
Draw__BootInit, Display__SetMode* (512x448 NTSC)...
3. `Stream__Init` (0x1B96B0): stream manager context @0xB78000
(0x760 B; 6 handle slots init -1; ready flag 0x71F8F7).

Scratch/message region: **0x30xxxx–0x31xxxx** (config buffers, string pool).
Msg dispatch: 13-case jump table @0x34CCD0.

| Callee | Hypothesis | Evidence |
| --- | --- | --- |
| 0x00134DD0 | Memory__InitRegions | args mirror heap-setup syscall pattern |
| 0x00177FD0 | Schedule__Init | memset 0x4D14 = scheduler context size |
| 0x00134AE0/0x134B20/0x134B60 | Display__SetMode* | float consts 512.0/448.0/256.0/224.0 (NTSC) |
| 0x001B96B0 | module loading? | far-away function, tail of init — CHECK NEXT |

GS register usage: `0x12001000` = GS CSR, bit 13 = FIELD (Display__GetField).
Mode state struct size confirmed: **0x180 bytes per index**, base 0x5B2F00
(`State__Update` computes base + index*0x180). Enabled flags @0x5A3A98.

## IOP module loading (in progress)

Module path table @0x301D80–0x301E74 (10 entries, contiguous). Loader
location pending: `axt @ 0x301D80` next. "Error could not reboot iop"
@0x301DA0 suggests IOP reboot (LoadExecPS2-style) before module loading.

## Game asset & overlay registry (found @0x142660 — FINAL 2026-10-14)

**CORRECTION:** 0x142660 is NOT the IOP .irx loader. It is the **unified
asset & overlay registry** — mapping game modes and assets to their data.

The table at **0x1C9AA0** has **three formats**:

**Format A (0x1C9AA0, 12 bytes):** `{flag, name_ptr, nkp_ptr}` — **asset NKP files**

```javascript
{0, "interior",    0x35E1A0}   → NKP file (magic 4E4B501A)
{0, "shiftbg",     0x39AB90}   → NKP file (magic 4E4B501A)
{1, "misc_bg",     0x00000000} → no file (flag=1, runtime generated)
{1, "clothes2",    0x00000000} → no file (flag=1, runtime generated)
{0, "story/allsc", 0x39B380}   → NKP file (magic 4E4B501A)
{0, "work",        0x3A9B10}   → ? (pending verify)
{0, "epi_txt",     0x3AA300}   → ? (pending verify)
{0, 0, 0}                           // terminator
```

**Format B (0x1C9B00, 16 bytes):** `{name, alt_name, overlay_code_ptr, ...}` — **executable overlays**

```javascript
{"Title",   "MyRoom",  0x001B04F0, ...}
{...,       0x00B82E80, "TRAIN",   ...}   // 0xB82E80 = overlay region
{0x001C1A80, ...,       "WORKING", 0x00B83140}
```

Overlay code pointers confirm MWo3 load address 0x00B7F800:

- 0x00B82E80 = 0xB7F800 + 0x4E80
- 0x00B83140 = 0xB7F800 + 0x5340
- 0x00B882F0 = 0xB7F800 + 0xA2F0
- 0x00B82FB0 = 0xB7F800 + 0x57B0
- 0x00B814B0 = 0xB7F800 + 0x3CB0

**NKP magic discovered:** `4E 4B 50 1A` ("NKP" + 0x1A) at 0x35E1A0, 0x39AB90,
0x39B380. The 0x1A byte is likely a version or header-size field.

**IOP .irx loading remains unfound** — likely kernel-side during boot.

## Ghidra cross-validation (2026-10-14) — Game__Main VERIFIED

Ghidra 12.1.2 decompile of the main loop matches the hand reconstruction
call-for-call. Corrections established by the decompiler:

- The one-shot pre-loop init is **0x142640(arg=1)**, called before the loop.
The in-loop call to 0x1425D0 seen in rizin output is at 0x141F54 and is
UNREACHABLE (dead code) — removed by the decompiler with warning
"Removing unreachable block (ram,0x00141f54)".
- Execution order in loop tail: System__RunFrameHook **before**
Schedule__FrameTick.
- Function prologue at 0x141EE0: `addiu sp,sp,-0x20; sd ra,0x10(sp)`
(bytes 10 00 BF FF at 0x141EE4 were initially undefined in Ghidra).

Lesson recorded in AGENTS.md: rizin hypotheses must be cross-checked with
the Ghidra decompiler before being marked VERIFIED.

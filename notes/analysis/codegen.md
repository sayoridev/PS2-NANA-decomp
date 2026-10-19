# Analysis — Codegen Idioms (Metrowerks CodeWarrior 2.4.1.01, R5900)

_Catalog of observed code-generation patterns. Add entries with disassembly
evidence (address + excerpt). Everything here is structural observation only._

## 1. Kernel syscall wrappers (dominant pattern, hundreds of instances)

```javascript
addiu v1, zero, N
syscall
jr    ra
nop
```

- Plain wrappers: 2 instructions of body; some are `-N` (remove/unregister
counterparts) — e.g. syscall 0x56 / -0x56 .. 0x59 / -0x59 around 0x1183A8.
- Dispatcher variant @0x1183A8: validates an ID in `[0x0D, 0x30)` before
forwarding to the syscall-0x56 wrapper.
- Boot-critical IDs: `0x3C` (heap/stack setup, returns sp in v0),
`0x3D` (secondary setup), `0x64` (called by crt0 with a0=0).
- **Stub table region**: 0x10EBA0–0x10ECDC hosts wrappers for IDs
0x64–0x76 (46+ callers) — a central kernel-API surface.

## 2. Prologue / epilogue

- Prologue: `addiu sp, sp, -N` then `sq $sX, off($sp)` 128-bit saves
(observed in SHOP2R.BIN @file+0x80 and main-ELF functions).
- Epilogue: `ld ra, off(sp)` … `jr ra` with `addiu sp, sp, N` in the
delay slot; saves/restores are `sd`/`ld` pairs for 64-bit GPRs.

## 3. Tail calls

- `j target; nop` replaces `jal` + `jr` chains (e.g. Startup__Entry
region → 0x11A9B0; crt0 tail → 0x127DE0). In C: a plain call in tail
position.

## 4. Data access

- Globals via `lui`/`addiu` absolute pairs (gp anchor 0x005AB270 also used).
- Overlay code references main-ELF globals and vice versa — cross-image
pointers exist, relocated at MWo3 load time.

## 4b. System / COP0 idioms (verified)

- COP0 Status read: `mfc0 v0, $12, 0` — rizin prints rd as a GPR name
(`mfc0 v1, t4, 0` = Status, since $12 = t4). Bit 16 = BEV.
- BEV pair: `System__IsBEVSet` (test+sltu) / `System__WaitBEVClear`
(poll loop) — 112 + 98 xrefs, the two hottest functions.
- Uncached access via KSEG1: `ptr | 0x20000000` (or with lui v0, 0x2000)
before lw/sw — DMA/hardware register blocks (e.g. 0x5A4xxx region).

## 5. Suspect decodes (verify in Ghidra)

Known rizin artifacts seen repeatedly (likely MMI opcodes misdecoded):

- `ext s0, sp, 0, 1` — appears at function heads (stack-alignment prologue?)
- `xori.b w0, w0, 0xb0` — trailing constant-return idiom area
- `mult ac3, rs, rt` — in Random__MulStep (64x64 multiply body)
- `dpa.w.ph ac0, sp, s3` — DSP-suffix decode, implausible on R5900
- `ext zero, v0, 0, 1` in the crt0 BSS-clear loop (0x00100160 area) — an
`ext` writing to $zero is implausible for CW; likely rizin misdecode.
Check raw bytes before relying on it.
- `v3mulu zero, zero, zero` at Startup__Entry head — likewise suspicious.

## 6. Branch-likely opcodes

- Opcode `0x14`/`0x15` (`beql`/`bnel`) appears in scheduler code, e.g.
`0x00188B88` (`FUN_00188A98`) and `0x00189838` (`fcn_00189830`).
- The instruction in the delay slot executes only when the branch is taken.
- Do not model these as plain `beq`/`bne` without preserving delay-slot
semantics.

## 7. Custom multi-register calls

- `0x00177320` and `0x00177960` pass extra parameters in `$t0-$t3` instead
of the normal o32 stack spill area.
- Example: `Task__QueuePending` receives `p5`, callback and callback argument
in `$t0-$t2`.
- Standard C prototypes are not sufficient to reproduce these calls exactly;
ABI-specific stubs or verified compiler extensions are required.

## 8. Delay-slot argument/state setup

- `Job__Handler11` stores `g_JobState = 3` in the delay slot before
`Schedule__Kick(6)`, so the state is visible before the callee body.
- The allocator wrapper at `0x001774D0` loads the move/copy length from the
inherited third argument; do not “clean up” the call into a two-argument
form.

## 9. Intrusive allocator and queue offsets

- Scheduler allocator nodes use fixed offsets:
`value +0x00`, `prev +0x04`, `next +0x08`, `payload/marker +0x0C`.
- Task slots use base `0x008D3900`, stride `0x134`, command `+0x04`, payload
words `+0x110/+0x114`.

## 10. SWAR string/memory loops

- `0x0012CA60` implements `strcpy` with zero-word detection.
- `0x00129B30` implements `memmove` with overlap-aware reverse copy.
- `0x00129CE8` implements byte-wise `memset`.
- The vectorized control flow is compiler-specific; C reconstructions preserve
observable behavior, not instruction-for-instruction shape.

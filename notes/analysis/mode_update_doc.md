# Mode__Update — decompile analysis (2026-10-15)

Ghidra R5900 decompile of 0x0013BDE0 (FULL, no gaps). Verbatim C committed
in src/game/mode_update_raw.c. Key extracted facts:

## Loop structure

- Iterates TWO state blocks: base 0x5B2F00, stride 0x180 (matches manual
analysis and State__Enable's index<2 check).

## State block field map (from decompile offsets)

| Offset | Field | Evidence |
| --- | --- | --- |
| +0x100 | rampAltMode | `if (*(int*)(+0x100) == 0)` gates ramp advance |
| +0x104 | rampACounter | incremented each frame while < max |
| +0x108 | rampAMax | loop bound |
| +0x10C | rampAType | 1 = set const (\\|1 odd), 2 = linear (\\|1), 3 = two-phase |
| +0x112/0x114 | rampA end/start (u16) | interpolation endpoints |
| +0x118..+0x128 | rampB same layout |  |
| +0x12A | lastKey | reset when key changes |
| +0x12C | buttons (u16, ACTIVE LOW inverted via ^0xffff) |  |
| +0x12E | modeState | from Pad__ReadStatus(slot,0) |
| +0x12F | subState | derived via Pad__TestCombo(slot,0,1/2,0) |
| +0x130 | command | switch: 0x28,0x29,0x2A,0x46,0x47,0x48,0x4C,0x4D,0,else |
| +0x134 | cmdCounter | ++ when modeState==7 |
| +0x13E/+0x13F | outA/outB | ramp outputs, sent via Pad__SetOutputs(slot,0,&outA) |
| +0x14A/+0x14B | analogX/Y | deadzone vs 0x50/0xB0; high-nibble==7 = analog-flagged key |
| +0x164 | buttonsPrev | prev frame |
| +0x166 | idleCounter | auto-repeat: counts frames with no press |
| +0x168 | buttonsPressed | cur & ~prev |
| +0x16A | buttonsRepeat | = pressed; on idle>delay && idle%interval==0 -> full cur |
| +0x16C | buttonsReleased | ~cur & prev |

## Button word flags built from analog check

- high nibble == 7 (analog key) + stick out of deadzone -> sets 0x8000/0x2000
(X axis) / 0x1000/0x4000 (Y axis) in the +0x12C word.

## gp globals discovered

- gp+0xFFFF8438 = 0x005A34A8 : pad auto-repeat delay (compare)
- gp+0xFFFF843C = 0x005A34AC : pad auto-repeat interval (modulo)

## Pad driver family (0x11Bxxx) — all take (slot, ???) first args

| Addr | Role (hypothesis) |
| --- | --- |
| 0x11B500 | Pad__ReadStatus — returns mode/state byte |
| 0x11B668 | Pad__TestButton — 1/0 (released test; also returns 2) |
| 0x11BE78 | Pad__TestButton2 |
| 0x11BE18 | Pad__TestButton3 — gates 0x48 -> 0x4C |
| 0x11BA60 | Pad__TestCombo(,,1,3) |
| 0x11B928 | Pad__TestAnalog(,,n,mask) — returns count/mask result |
| 0x11BBE0 | Pad__Query(,,buf) |
| 0x11B488 | Pad__Check(,,buf) |
| 0x11BB18 | Pad__SetOutputs(,,u8*2) — ramp outputs (rumble fade hypothesis) |

## Command state machine semantics (hypotheses to verify)

- 0x4D: on release decrement; at 0 -> 99 (menu item down-through?)
- 0x4C: on release increment
- 0x48: TestButton3 ? -> 0x4C : 99 (confirm/cancel split)
- 0x47: decrement then increment (both)
- 0x46: two-phase with 6-byte work buffer @ +0x138, loops via 0x47
- 0x2A: decrement to 0
- 0x29: TestCombo(1,3) gated increment
- 0x28: TestAnalog(4,-1) -> +1 to 0x29 chain, else 99
- 0 (idle): reads subState (1/2 read, 7 = commit -> 0x46, 4 -> 0x28)
- else: cmdCounter++ while modeState==7; ramp engine when modeState 2/6

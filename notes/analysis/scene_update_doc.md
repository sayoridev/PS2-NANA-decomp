# Scene__Update — decompile analysis (2026-10-15)

Ghidra R5900 decompile of 0x00146040 (FULL). Verbatim C in
src/game/scene_update_raw.c.

## State machine (variable @0x5BB884, per-frame from main loop)

| State | Phase | Detail |
| --- | --- | --- |
| 1 | fade-in A | item counters (ctx+0xBC+4i) vs targets g_FadeTable[+0x08]; -> 2 |
| 2,3 | fade-in B | targets [+0x10]; all done && global time -> 4 |
| 4,5,6 | hold | counters loop [+0x14..+0x18]; time loaded from g_SceneTiming |
| 7 | fade-out + overlay | **Module__LoadRequest(1)**; per-item volume ramp on parallel array +0x200 (floor -0x80, slope 0x80/n, subcounter +0xD4) |
| 8,9 | fade-out B | targets [+0x28]; -> 10 |
| 10 | reset | counters = [+0x28] values; -> 0 |
| 0 | idle |  |

## Key structures

- g_SceneContext @0x5BB880: +0x18 item id, +0xBC.. item counters,
+0x884 state, +0x888 flag, +0x960 countdown, +0x968 time counter,
+0x964 scene index (into g_SceneTiming 0x10-byte entries)
- g_FadeCounts @0x1C9E20 (per mode idx: item count)
- g_FadeTable @0x1C9E40: per-mode array, 0x2C-byte entries
[+0x08 fadeA target][+0x10 fadeB][+0x14/+0x18 loop pair][+0x20/+0x28 fadeC]
- g_SceneTiming @0x1CA164: per-scene 0x10-byte entries (+0 max, +8 hold)
- g_SceneItemModes @0x5BB830 (0xC bytes per mode; 0 = simple, else ramp)
- g_DmaPacketBuf @0x5BB8A8: packet fields built per item (0x100, flags,
param from *(0x5A3680 + 0x48)) -> Draw__SubmitPacket(ctx, id, buf)

## Function graph

- Draw/DMA: Display__BeginContext(8) -> per item: Draw__SetItemCount(id,
count-1) + Draw__SubmitPacket -> optional extra item -> Display__EndContext
- FUN_001347f0/00134790 CONFIRMED as context begin/end (pair, param 8)

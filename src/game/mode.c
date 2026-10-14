/*
 * PS2-NANA-decomp — game mode state accessors
 * STATUS: RECONSTRUCTED
 * ORIGINAL: SLPM_659.14 @ 0x001424B0 / 0x001424C0 / 0x001424D0
 *
 * Game state = two structs near 0x5B3E50/0x5B3E60 (see mainloop.md).
 * Struct A @0x5B3E60 is 8 x u16 (16 bytes) — copied out wholesale by
 * State__CopyOut. Getters read single u16 fields (+2 and +0xA).
 */
#include "../common/ps2.h"
#include <string.h>

typedef struct {
    u16 field[8];      /* 0x5B3E60 .. 0x5B3E70 */
} Mode__StateBlock;

Mode__StateBlock g_ModeStateA;   /* @0x005B3E60 — updated via fcn.0013B960(0,...) */
Mode__StateBlock g_ModeStateB;   /* @0x005B3E50 — updated via fcn.0013B960(1,...) */

u32 g_StateEnabled[2];   /* @0x005A3A98 (gp-0x7BB8) — checked by State__UpdatePair */

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x001424C0 — 10 callers
 * PURPOSE: read state field @ +2. */
u16 Mode__GetStatus(void)
{
    return g_ModeStateA.field[1];
}

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x001424B0 — 32 callers (most-used getter in the game)
 * PURPOSE: read state field @ +0xA. */
u16 Mode__GetState(void)
{
    return g_ModeStateA.field[5];
}

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x0013BB80 — called by Game__Main with arg 0
 * PURPOSE: enable one of the two mode state blocks (flag slot per index). */
void State__Enable(s32 index)
{
    if (index >= 0 && index < 2) {
        g_StateEnabled[index] = 1;
    }
}

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x001424D0 — 7 callers
 * PURPOSE: copy the whole 16-byte state block out to the caller. */
void State__CopyOut(u16 *dst)
{
    memcpy(dst, &g_ModeStateA, sizeof(g_ModeStateA));
}

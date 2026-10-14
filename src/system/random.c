/*
 * PS2-NANA-decomp — PRNG (Knuth LCG)
 * STATUS: RECONSTRUCTED
 * ORIGINAL: SLPM_659.14 @ 0x0012B968 (third call of Game__MainLoop, 44 xrefs)
 *
 * Disasm (rizin):
 *   s0 = *(u32*)0x001C7FD0               ; g_SystemContext
 *   a0 = ld  0xa8(s0)                    ; 64-bit state
 *   v0 = fcn.00123C18(a0, 0x5851F42D4C957F2D)
 *   v1 = v0 + 1
 *   sd  v1, 0xa8(s0)                     ; state' = a*X + 1
 *   return (s32)((u32)(state' >> 32) & 0x7FFFFFFF)
 * Multiplier = 6364136223846793005 = Knuth MMIX LCG constant. c = 1.
 */
#include "../common/ps2.h"

#define RANDOM__MULT 0x5851F42D4C957F2DULL   /* Knuth MMIX multiplier */

extern u64 Random__MulStep(u64 x, u64 mult); /* @0x00123C18 — 64x64 multiply helper */

typedef struct {
    u8  _pad[0xA8];
    u64 rngState;       /* +0xA8 */
} System__Context;

extern System__Context *g_SystemContext;    /* @0x001C7FD0 */

s32 Random__Next(void)
{
    u64 *state = &g_SystemContext->rngState;

    *state = Random__MulStep(*state, RANDOM__MULT) + 1;   /* X' = aX + 1 */
    return (s32)(((u32)(*state >> 32)) & 0x7FFFFFFFu);
}

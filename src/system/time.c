/*
 * PS2-NANA-decomp — frame tick counter
 * STATUS: RECONSTRUCTED
 * ORIGINAL: SLPM_659.14 @ 0x0014E8E0 (first call of Game__MainLoop)
 *
 * Disasm (rizin):
 *   lw   a0, 0x18(base=0x71FDD0)      ; load counter
 *   slt  v0, a0, 0x0CDFD7F0
 *   bnez -> increment & store
 *   (capped: no store)
 * Counter lives at 0x0071FDE8 = field 0x18 of a struct based at 0x0071FDD0.
 */
#include "../common/ps2.h"

#define TIME__TICK_CAP 0x0CDFD7F0u

typedef struct {
    u8  _pad[0x18];
    u32 counter;        /* +0x18 -> 0x0071FDE8 */
} Time__TickContext;

Time__TickContext Time__g_Context;      /* @0x0071FDD0 */

s32 Time__Tick(void)
{
    if (Time__g_Context.counter < TIME__TICK_CAP) {
        Time__g_Context.counter++;
    }
    return (s32)Time__g_Context.counter;
}

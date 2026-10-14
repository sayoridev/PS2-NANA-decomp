/*
 * PS2-NANA-decomp — GS/display status
 * STATUS: RECONSTRUCTED
 * ORIGINAL: SLPM_659.14 @ 0x001010B8
 *
 * Reads GS Control/Status Register (EE memory-mapped at 0x12001000),
 * bit 13 = FIELD (current interlace output field). 7 callers.
 */
#include "../common/ps2.h"

#define GS_CSR  (*(volatile u64 *)0x12001000u)
#define GS_CSR_FIELD_BIT 13u

typedef struct {
    s16  mode;         /* +0x00: 1 = interlaced (field valid) */
    u8   _pad[6];
    u32  field_0x8;    /* +0x08: selects CSR read path */
} Display__Context;

extern Display__Context *Display__GetContext(void);  /* @0x001003C0 */
extern u64 Gs__ReadCsrA(Display__Context *);         /* @0x0010EDB0 — direct path */
extern u64 Gs__ReadCsrB(Display__Context *);         /* @0x0010EE40 — cached path */

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x001010B8
 * PURPOSE: return the current GS output field (1 when not interlaced). */
s32 Display__GetField(void)
{
    Display__Context *ctx = Display__GetContext();
    u64 csr = (ctx->field_0x8 != 0) ? Gs__ReadCsrB(ctx) : Gs__ReadCsrA(ctx);

    if (ctx->mode != 1) {
        return 1;
    }
    return (s32)((csr >> GS_CSR_FIELD_BIT) & 1u);
}

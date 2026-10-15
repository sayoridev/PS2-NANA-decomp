/*
 * PS2-NANA-decomp — pad driver (PsIIlibpad wrapper over Sony libpad)
 * STATUS: RECONSTRUCTED (decompile-verified, Ghidra R5900)
 * ORIGINAL: SLPM_659.14 — family at 0x0011B2D8..0x0011BE78
 *
 * Architecture (verified):
 *   IOP DMAs pad state into a double-buffered 2x128B record per (slot,sub).
 *   EE reads via Pad__GetStateRaw; cache is invalidated over the DMA range.
 *   Reads are guarded by the BEV bootstrap primitives. Outputs (actuators:
 *   6-byte DS2 motor payload) go through a transfer context + Pad__Transfer.
 */
#include "../common/ps2.h"

/* ---- context tables ---- */
#define PAD_CTX_BASE      0x005ABB50   /* entry = slot*0x70 + sub*0x1C */
#define PAD_XFER_PTRS     0x005ABB54   /* per-pad transfer context pointer */
#define PAD_FLAG_PRESENT  0x005ABB60   /* +0x10 from base: pad present flag */

/* record (128B, selected by read/write index compare) */
#define REC_ACTUATOR_RDY  0x70         /* pad supports actuators */
#define REC_DATA_RDY      0x71         /* new data available this frame */

extern s32  System__IsBEVSet(void);       /* @0x00118A30 */
extern s64  System__WaitBEVClear(void);   /* @0x001189E0 */
extern void Cache__InvalidateRange(void *start, void *end); /* @0x0010F3D8 */
extern s32  Pad__TransferTrigger(s32 slot, s32 sub);        /* @0x0011AF20 */

typedef struct {                 /* transfer context (IOP-bound command) */
    s32 cmd;                     /* +0x04: 1 = set outputs */
    s32 len;                     /* +0x08: payload length */
    u8  payload[6];              /* +0x0C: DS2 actuator data */
} Pad__TransferCtx;

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x0011B2D8 — central hub (7 callers across the pad family)
 * PURPOSE: fetch current pad record; selects DMA buffer by index compare;
 *          invalidates dcache over the DMA region. With out==NULL returns
 *          the raw record pointer instead of copying. */
int Pad__GetStateRaw(int slot, int sub, void *out)
{
    int waited = System__WaitBEVClear();
    int ctx    = *(int *)(PAD_CTX_BASE + sub * 0x1c + slot * 0x70);
    int bufSel;

    Cache__InvalidateRange((void *)ctx, (void *)(ctx + 0xff));

    bufSel = (*(int *)(ctx + 0x58) < *(int *)(ctx + 0xd8)) ? 0x80 : 0;
    if (out != NULL) {
        /* copy 128-byte record (aligned/unaligned variants in original) */
        u8 *dst = out, *src = (u8 *)(ctx + bufSel);
        for (int i = 0; i < 128; i++) dst[i] = src[i];
    }
    if (waited == 1) {
        System__IsBEVSet();
    }
    return ctx + bufSel;
}

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x0011B500
 * PURPOSE: per-frame pad status for a mode slot.
 *          Returns 'c' (99) when pad absent — 99 in the Mode__Update
 *          state machine means "no input device". */
char Pad__ReadStatus(int slot, int sub)
{
    u8  rec[128];
    char status;

    if (*(int *)(PAD_FLAG_PRESENT + sub * 0x1c + slot * 0x70) == 0) {
        status = 'c';
    } else {
        Pad__GetStateRaw(slot, sub, rec);
        status = rec[0];                 /* cStack_20 */
        if (status == 6 && rec[1] == 2) {/* cStack_1f */
            status = 5;
        }
    }
    return status;
}

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x0011B668
 * PURPOSE: data-ready flag (record[+0x71]); 0 when pad absent. */
u8 Pad__TestButton(int slot, int sub)
{
    int rec;

    if (*(int *)(PAD_FLAG_PRESENT + sub * 0x1c + slot * 0x70) == 0) {
        return 0;
    }
    rec = Pad__GetStateRaw(slot, sub, NULL);
    return *(u8 *)(rec + REC_DATA_RDY);
}

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x0011BB18
 * PURPOSE: DS2 actuator output (rumble). Builds a 6-byte motor payload in
 *          the pad transfer context and triggers the IOP transfer.
 *          Called by the Mode__Update ramp engine with 2 interpolated bytes. */
s32 Pad__SetOutputs(int slot, int sub, u8 *out6)
{
    Pad__TransferCtx *xfer;
    u8  rec[128];
    s32 ok = 0;

    Pad__GetStateRaw(slot, sub, rec);
    if (rec[REC_ACTUATOR_RDY] == 1) {
        xfer = *(Pad__TransferCtx **)(PAD_XFER_PTRS + sub * 0x1c + slot * 0x70);
        for (int i = 0; i < 6; i++) xfer->payload[i] = out6[i];
        xfer->cmd = 1;
        xfer->len = 6;
        ok = (Pad__TransferTrigger(slot, sub) == 1);
    }
    return ok;
}

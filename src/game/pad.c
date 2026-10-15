/*
 * PS2-NANA-decomp — pad driver (PsIIlibpad wrapper over Sony libpad)
 * STATUS: RECONSTRUCTED (decompile-verified, Ghidra R5900)
 * ORIGINAL: SLPM_659.14 — family at 0x0011AF20..0x0011BE78
 *
 * I/O architecture (fully verified):
 *   INPUT  (IOP -> EE): pad state DMA'd into a double-buffered 2x128B record
 *           per (slot,sub). EE reads via Pad__GetStateRaw; dcache INVALIDATED
 *           over the DMA region (stale-data discard). Freshness by read/write
 *           index compare. Reads guarded by the BEV bootstrap primitives.
 *   OUTPUT (EE -> IOP): actuator commands in a 2-slot x 0x20B ring;
 *           dcache WRITEBACK, then sceSifSetDma; in-flight DMA detected via
 *           sceSifDmaStat. RPC-style pad commands (TestCombo/Query) go through
 *           Rpc__Call to the IOP pad server.
 *   Button word: 18 bits total (0x3FFFF = none pressed, active-low raw);
 *           standard buttons = low 12 bits (0xFFF); analog events use the
 *           upper bits (see Mode__Update flag packing).
 */
#include "../common/ps2.h"

/* ---- context tables ---- */
#define PAD_CTX_BASE      0x005ABB50   /* entry = slot*0x70 + sub*0x1C */
#define PAD_XFER_PTRS     0x005ABB54   /* per-pad EE->IOP transfer context   */
#define PAD_IOP_DST       0x005ABB58   /* IOP-side DMA destination           */
#define PAD_DMA_BUSY      0x005ABB5C   /* in-flight DMA id (0 = free)        */
#define PAD_FLAG_PRESENT  0x005ABB60   /* pad present flag                   */
#define PAD_RPC_PORT      0x005ABB00   /* SifRpc client struct               */
#define PAD_RPC_CMDBUF    0x005ABD40   /* 0x80-byte RPC command block        */
#define PAD_RPC_RESULT    (*(volatile s32 *)0x005ABD54)

/* record fields (128-byte DMA record, current buffer) */
#define REC_ACTUATOR_RDY  0x70         /* pad supports actuators             */
#define REC_DATA_RDY      0x71         /* new data available this frame      */

#define PAD_BTNS_ALL_RELEASED  0x3ffff
#define PAD_BTNS_STANDARD_MASK 0xfff

extern s64  System__WaitBEVClear(void);   /* @0x001189E0 */
extern s32  System__IsBEVSet(void);       /* @0x00118A30 */
extern void Cache__InvalidateRange(void *start, void *end); /* @0x0010F3D8 */
extern void Cache__FlushRange(void *start, void *end);      /* @0x0010F218 */
extern s64  Rpc__Call(s32 port, s32 fn, s32, s32 sndBuf, s32 sndSize,
                      s32 rcvBuf, s32 rcvSize, s32);        /* @0x00112868 */
extern s32  fcn_0011b5b0(s32 slot, s32 sub, s32);           /* @0x0011B5B0: rpc wake/signal */
extern u64  Pad__GetButtonsRaw(s32 slot);                   /* @0x0011BCB8: raw 18-bit word */
extern s32  Pad__GetButtonsMasked(s32 slot, s32 sub, s32 mask); /* @0x0011BD68 */
extern void Memory__Copy(void *dst, void *src, s32 n);      /* @0x00129B30 */
extern s64  sceSifDmaStat(void);                            /* ps2sdk SIF DMA */
extern s64  sceSifSetDma(void *array, s32 count);
extern void Thread__Delay(s32 ticks);                       /* @0x00111608 */

/* ================= INPUT ================= */

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x0011B2D8 — central hub (7 callers). NULL out = return record ptr.
 * PURPOSE: select the fresh DMA buffer by index compare, invalidate dcache. */
int Pad__GetStateRaw(int slot, int sub, void *out)
{
    int waited = System__WaitBEVClear();
    int ctx    = *(int *)(PAD_CTX_BASE + sub * 0x1c + slot * 0x70);
    int bufSel;

    Cache__InvalidateRange((void *)ctx, (void *)(ctx + 0xff));
    bufSel = (*(int *)(ctx + 0x58) < *(int *)(ctx + 0xd8)) ? 0x80 : 0;
    if (out != NULL) {
        u8 *dst = out, *src = (u8 *)(ctx + bufSel);
        for (int i = 0; i < 128; i++) dst[i] = src[i];
    }
    if (waited == 1) {
        System__IsBEVSet();
    }
    return ctx + bufSel;
}

/* STATUS: RECONSTRUCTED  ADDRESS: 0x0011B500
 * 'c'(99) = pad absent — 99 in Mode__Update means "no input device". */
char Pad__ReadStatus(int slot, int sub)
{
    u8 rec[128];
    char status;

    if (*(int *)(PAD_FLAG_PRESENT + sub * 0x1c + slot * 0x70) == 0) {
        status = 'c';
    } else {
        Pad__GetStateRaw(slot, sub, rec);
        status = rec[0];
        if (status == 6 && rec[1] == 2) status = 5;
    }
    return status;
}

/* STATUS: RECONSTRUCTED  ADDRESS: 0x0011B668 — record[+0x71] data-ready flag */
u8 Pad__TestButton(int slot, int sub)
{
    int rec;
    if (*(int *)(PAD_FLAG_PRESENT + sub * 0x1c + slot * 0x70) == 0) return 0;
    rec = Pad__GetStateRaw(slot, sub, NULL);
    return *(u8 *)(rec + REC_DATA_RDY);
}

/* STATUS: RECONSTRUCTED  ADDRESS: 0x0011BE78 — standard-buttons mask query */
undefined8 Pad__TestButton2(undefined8 slot, int sub)
{
    if (*(int *)(sub * 0x1c + (int)slot * 0x70 + PAD_FLAG_PRESENT) == 0) return 0;
    return Pad__GetButtonsMasked((s32)slot, sub, PAD_BTNS_STANDARD_MASK);
}

/* STATUS: RECONSTRUCTED  ADDRESS: 0x0011BE18
 * TRUE when ALL buttons released (raw active-low word == 0x3FFFF).
 * Used by Mode__Update state 0x48 as a wait-for-release gate. */
bool Pad__TestButton3(undefined8 slot, int sub)
{
    if (*(int *)(sub * 0x1c + (int)slot * 0x70 + PAD_FLAG_PRESENT) == 0) return false;
    return Pad__GetButtonsRaw((s32)slot) == PAD_BTNS_ALL_RELEASED;
}

/* STATUS: RECONSTRUCTED  ADDRESS: 0x0011B928 — analog accessor
 * mode 1: analog button id (byte>>4; 0xF3 = no event)
 * mode 2/4: u16 axis value from record axis table (param_4 = axis index;
 *           -1 returns the axis-event byte)
 * mode 3: raw record byte */
uint Pad__TestAnalog(undefined8 slot, int sub, int mode, uint sel)
{
    u8   rec[80];
    u16  axis[10];
    char c4c, c3f, c3e;
    u8   b4b, b48, b47;

    if (*(int *)(sub * 0x1c + (int)slot * 0x70 + PAD_FLAG_PRESENT) != 0) {
        Pad__GetStateRaw((int)slot, sub, rec);
        if (c3e != 1 || c3f == 2) return 0;
        switch (mode) {
        case 2:
            if (c4c != 1) { sel = b47; return axis[sel]; }
            break;
        case 1:
            if (b4b != 0xf3) return b4b >> 4;
            break;
        case 3:
            if (c4c != 1) return b47;
            break;
        case 4:
            if (c4c != 1) {
                if (sel == 0xffffffffu) return b48;
                if ((int)sel < (int)b48) return axis[sel];
            }
            break;
        }
    }
    return 0;
}

/* STATUS: RECONSTRUCTED  ADDRESS: 0x0011B488 — copy record slice to caller */
undefined4 Pad__Check(undefined8 slot, undefined8 sub, undefined8 dst)
{
    u8 rec[96];
    undefined4 ret = 0;

    if (*(int *)((int)sub * 0x1c + (int)slot * 0x70 + PAD_FLAG_PRESENT) == 0) {
        ret = 0;
    } else {
        Pad__GetStateRaw((int)slot, (int)sub, rec);
        Memory__Copy((void *)dst, rec, 0);   /* size field recovered at cleanup */
    }
    return ret;
}

/* ================= RPC COMMANDS (IOP pad server) ================= */

/* STATUS: RECONSTRUCTED  ADDRESS: 0x0011BA60 — RPC cmd 6 */
int Pad__TestCombo(undefined8 slot, undefined8 sub, int p3, int p4)
{
    *(s32 *)PAD_RPC_CMDBUF       = 6;
    *(s32 *)(PAD_RPC_CMDBUF + 4) = (s32)slot;
    *(s32 *)(PAD_RPC_CMDBUF + 8) = (s32)sub;
    *(s32 *)(PAD_RPC_CMDBUF + 0xc) = p3;
    *(s32 *)(PAD_RPC_CMDBUF + 0x10) = p4;
    if (Rpc__Call(PAD_RPC_PORT, 1, 0, PAD_RPC_CMDBUF, 0x80,
                  PAD_RPC_CMDBUF, 0x80, 0) < 0) {
        return 0;
    }
    if (PAD_RPC_RESULT == 1) {
        fcn_0011b5b0((s32)slot, (s32)sub, 2);
    }
    return PAD_RPC_RESULT;
}

/* STATUS: RECONSTRUCTED  ADDRESS: 0x0011BBE0 — RPC cmd 8, 6-byte payload */
int Pad__Query(undefined8 slot, undefined8 sub, int payloadSrc)
{
    int ret;

    *(s32 *)PAD_RPC_CMDBUF       = 8;
    *(s32 *)(PAD_RPC_CMDBUF + 4) = (s32)slot;
    *(s32 *)(PAD_RPC_CMDBUF + 8) = (s32)sub;
    for (int i = 0; i < 6; i++)
        *(u8 *)(PAD_RPC_CMDBUF + 0xc + i) = *(u8 *)(payloadSrc + i);
    if (Rpc__Call(PAD_RPC_PORT, 1, 0, PAD_RPC_CMDBUF, 0x80,
                  PAD_RPC_CMDBUF, 0x80, 0) < 0) {
        return 0;
    }
    ret = PAD_RPC_RESULT;
    if (PAD_RPC_RESULT == 1) {
        fcn_0011b5b0((s32)slot, (s32)sub, 2);
        ret = PAD_RPC_RESULT;
    }
    return ret;
}

/* ================= OUTPUT (rumble) ================= */

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x0011AF20 — EE->IOP DMA of the actuator command ring.
 * Ring: 2 x 0x20-byte slots at the transfer context; IOP dst = ctx[+4-ish]
 * slot alternated by (*ringIdx + 1) & 1. Returns 1 when DMA accepted. */
undefined4 Pad__TransferTrigger(int slot, int sub)
{
    int   entry = sub * 0x1c + slot * 0x70;
    uint *ring  = *(uint **)(entry + PAD_XFER_PTRS);
    int   iopDst, idx;
    u32   newIdx;
    s64   dmaId;

    if (*(int *)(entry + PAD_DMA_BUSY) == 0 ||
        (dmaId = sceSifDmaStat(), dmaId < 0)) {
        idx     = *(int *)(entry + PAD_IOP_DST);
        newIdx  = *ring + 1;
        *ring   = newIdx;
        Cache__FlushRange(ring, (u8 *)ring + 0x1f);   /* writeback before DMA  */
        {
            struct { void *src; int dst; int size; int attr; } d =
                { ring, idx + ((newIdx + 1) & 1) * 0x20, 0x20, 0 };
            dmaId = sceSifSetDma(&d, 1);
        }
        if (dmaId != 0) {
            *(int *)(entry + PAD_DMA_BUSY) = (int)dmaId;
            return 1;
        }
    }
    if (*(s32 *)0x001C6064 != 0) {        /* retry-delay enabled             */
        Thread__Delay(0x2ffe80);
    }
    return 0;
}

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x0011BB18 — DS2 actuator output (rumble).
 * Payload: 6-byte motor command; ramp engine feeds 2 interpolated bytes. */
s32 Pad__SetOutputs(int slot, int sub, u8 *out6)
{
    uint *xfer = *(uint **)(PAD_XFER_PTRS + sub * 0x1c + slot * 0x70);
    u8    rec[128];
    s32   ok = 0;

    Pad__GetStateRaw(slot, sub, rec);
    if (rec[REC_ACTUATOR_RDY] == 1) {
        u8 *payload = (u8 *)xfer + 0x0C;
        for (int i = 0; i < 6; i++) payload[i] = out6[i];
        *(s32 *)((u8 *)xfer + 4) = 1;     /* cmd = set outputs                */
        *(s32 *)((u8 *)xfer + 8) = 6;     /* len = 6                          */
        ok = (Pad__TransferTrigger(slot, sub) == 1);
    }
    return ok;
}

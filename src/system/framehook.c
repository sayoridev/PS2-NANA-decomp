/*
 * PS2-NANA-decomp — per-frame hook system
 * STATUS: RECONSTRUCTED (RunFrameHook PARTIAL — control-flow detail)
 * ORIGINAL: SLPM_659.14 — RunFrameHook @0x00142570, InitFrameHook @0x001425D0
 *
 * Three gp-relative slots (see mainloop.md "Struct map"):
 *   gate 0x5A3A64, flag 0x5A3A68, callback 0x5A3A6C. Default handler 0x001A8FE0.
 */
#include "../common/ps2.h"

typedef void (*System__HookFn)(void);

typedef struct {
    u32           gate;        /* 0x5A3A64 */
    u32           flag;        /* 0x5A3A68 */
    System__HookFn callback;   /* 0x5A3A6C */
} System__FrameHook;

System__FrameHook g_FrameHook;
#define SYSTEM__DEFAULT_HOOK  ((System__HookFn)0x001A8FE0)

extern void fcn_00146A20(void);   /* @0x00146A20 — pending classification */
extern void fcn_001010B8(s32);    /* @0x001010B8 — pending classification */

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x001425D0 — invoked once, on the first main-loop iteration
 * PURPOSE: run any registered hook, finish subsystem setup, install the
 *          default frame hook. */
void System__InitFrameHook(void)
{
    if (g_FrameHook.callback != 0 && g_FrameHook.flag != 0) {
        g_FrameHook.callback();
    }
    fcn_00146A20();
    fcn_001010B8(0);
    g_FrameHook.flag = 0;
    g_FrameHook.callback = SYSTEM__DEFAULT_HOOK;
}

/* STATUS: PARTIAL
 * ADDRESS: 0x00142570
 * PURPOSE: ensure a hook is installed (default otherwise) and invoke it
 *          when the gate slot is non-zero. Exact branch layout of the
 *          "install default then retry" loop pending Ghidra confirmation. */
void System__RunFrameHook(void)
{
    if (g_FrameHook.callback == 0) {
        g_FrameHook.callback = SYSTEM__DEFAULT_HOOK;
    }
    if (g_FrameHook.gate != 0) {
        g_FrameHook.callback();
    }
}

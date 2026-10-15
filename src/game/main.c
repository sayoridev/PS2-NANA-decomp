/*
 * PS2-NANA-decomp — game entry point and main loop
 * STATUS: RECONSTRUCTED (call sequence VERIFIED against Ghidra decompile)
 * ORIGINAL: SLPM_659.14 @ 0x00141EE0 (Game__Main), loop @ 0x00141F04
 *
 * Verified call-by-call against Ghidra 12.1.2 decompiler output
 * (2026-10-14). Corrections from rizin-only analysis:
 *   - real one-shot pre-loop init is 0x142640(1); the in-loop call to
 *     0x1425D0 that rizin showed is DEAD CODE (unreachable, s0 always 0)
 *   - System__RunFrameHook executes BEFORE Schedule__FrameTick
 */
#include "../common/ps2.h"

extern void Game__Init(void);           /* @0x00141D90 */
extern void State__Enable(s32);         /* @0x0013BB80 */
extern void fcn_00142640(s32);          /* @0x00142640 — one-shot pre-loop init (name TBD) */

extern s32  Time__Tick(void);           /* src/system/time.c */
extern void System__FrameService(void); /* @0x00142390 */
extern s32  Random__Next(void);         /* src/system/random.c */
extern void Mode__Update(void);         /* @0x0013BDE0 */
extern void State__UpdatePair(void);    /* @0x00142510 */
extern u16  Mode__GetStatus(void);      /* src/game/mode.c */
extern u16  Mode__GetState(void);       /* src/game/mode.c */
extern void Draw__SetContext2(void);    /* @0x0013B310 */
extern void Sound__Update(void);        /* @0x00177F30 — sound command processor */
extern void System__RunFrameHook(void); /* src/system/framehook.c */
extern void Sound__FrameTick(void);     /* @0x00177F00 */
extern void Scene__Update(void);        /* @0x00146040 */
extern void System__DispatchPending(void); /* @0x00142700 (PARTIAL) */
extern void Display__EndFrame(void);    /* @0x00134D50 */

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x00141EE0
 * PURPOSE: top-level game entry — init, then the infinite frame loop.
 *          Never returns.
 * NOTE: dead code at 0x141F54 (jal System__InitFrameHook @0x1425D0) exists
 *       in the binary but is unreachable — not reproduced here. */
void Game__Main(void)
{
    Game__Init();
    State__Enable(0);
    fcn_00142640(1);            /* one-shot pre-loop init */

    for (;;) {
        Time__Tick();
        System__FrameService();
        Random__Next();
        Mode__Update();
        State__UpdatePair();
        Mode__GetStatus();
        Mode__GetState();
        Draw__SetContext2();
        Schedule__Update();
        System__RunFrameHook();
        Schedule__FrameTick();
        Scene__Update();
        System__DispatchPending();
        Display__EndFrame();
    }
}

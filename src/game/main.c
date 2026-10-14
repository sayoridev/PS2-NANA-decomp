/*
 * PS2-NANA-decomp — game entry point and main loop
 * STATUS: RECONSTRUCTED
 * ORIGINAL: SLPM_659.14 @ 0x00141EE0 (Game__Main), loop @ 0x00141F04
 *
 * Full loop body verified call-by-call against rizin disassembly.
 * Branch layout of the first-iteration check reconstructed semantically
 * (exact skip target pending Ghidra confirmation).
 */
#include "../common/ps2.h"

extern void Game__InitA(void);          /* @0x00141D90 — pending classification */
extern void fcn_0013BB80(s32);          /* @0x0013BB80 — pending classification */

extern s32  Time__Tick(void);           /* src/system/time.c */
extern void System__FrameService(void); /* @0x00142390 */
extern s32  Random__Next(void);         /* src/system/random.c */
extern void Mode__Update(void);         /* @0x0013BDE0 */
extern void State__UpdatePair(void);    /* @0x00142510 */
extern u16  Mode__GetStatus(void);      /* src/game/mode.c */
extern u16  Mode__GetState(void);       /* src/game/mode.c */
extern void Draw__SetContext2(void);    /* @0x0013B310 */
extern void Schedule__Update(void);     /* @0x00177F30 */
extern void System__InitFrameHook(void);/* src/system/framehook.c — first iteration only */
extern void Schedule__FrameTick(void);  /* @0x00177F00 */
extern void System__RunFrameHook(void); /* src/system/framehook.c */
extern void Scene__Update(void);        /* @0x00146040 */
extern void System__DispatchPending(void); /* @0x00142700 (PARTIAL) */
extern void Display__EndFrame(void);    /* @0x00134D50 */

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x00141EE0
 * PURPOSE: top-level game entry — init, then the infinite frame loop.
 *          Never returns; System__Exit is reached only via external callers. */
void Game__Main(void)
{
    Game__InitA();
    fcn_0013BB80(0);

    s32 firstIteration = 1;
    for (;;) {
        Time__Tick();               /* frame counter */
        System__FrameService();     /* per-frame hook + perf counters */
        Random__Next();             /* advance Knuth LCG once per frame */
        Mode__Update();             /* game mode state machine */
        State__UpdatePair();        /* update the two mode state structs */
        Mode__GetStatus();          /* (result unused — side effects only?) */
        Mode__GetState();           /* (result unused — side effects only?) */
        Draw__SetContext2();        /* graphic context state 2 */
        Schedule__Update();         /* task scheduler + countdown */
        if (firstIteration) {
            System__InitFrameHook();
        }
        Schedule__FrameTick();      /* scheduler counter */
        System__RunFrameHook();     /* gated per-frame hook */
        Scene__Update();            /* scene/script state machine */
        System__DispatchPending();  /* consume pending command id */
        Display__EndFrame();        /* end-of-frame / flip */
        firstIteration = 0;
    }
}

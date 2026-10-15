/*
 * PS2-NANA-decomp — task scheduler
 * STATUS: RECONSTRUCTED (decompile-verified, Ghidra R5900)
 * ORIGINAL: SLPM_659.14 @ 0x00177F00/0x00177F30
 *
 * Model (verified):
 *   Context at 0x008D3900: countdown timer @ +0x00, slot array @ +0x04,
 *   59 slots of 0x134 bytes. Current slot index @ 0x008D8608; frame
 *   counter @ 0x008D8610 (incremented per frame by Schedule__FrameTick).
 *   When the countdown expires and the current slot has a callback,
 *   Task__Execute runs it.
 */
#include "../common/ps2.h"

extern void Schedule__TickA(void);      /* @0x00192BC8 */
extern void Schedule__TickB(void);      /* @0x0017CCB0 */
extern void Schedule__WalkSlots(void);  /* @0x00177140 */
extern void Task__Execute(void);        /* @0x00177C80 */
extern void fcn_00189468(void);         /* @0x00189468 — subsystem update, TBD */

#define SCHED_COUNTDOWN   (*(volatile s32 *)0x008D3900)
#define SCHED_SLOT(idx)   (*(volatile s32 *)(0x008D3904 + (idx) * 0x134))
#define SCHED_CUR_SLOT    (*(volatile s32 *)0x008D8608)
#define SCHED_FRAME_COUNT (*(volatile s32 *)0x008D8610)

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x00177F30 — main-loop call #9
 * PURPOSE: tick the scheduler: run countdown, subsystem ticks, slot walk,
 *          execute current task when the timer expires. */
void Schedule__Update(void)
{
    if (SCHED_COUNTDOWN > 0) {
        SCHED_COUNTDOWN--;
        if (SCHED_COUNTDOWN < 1) {
            SCHED_COUNTDOWN = 0;
        }
    }
    Schedule__TickA();
    fcn_00189468();
    Schedule__WalkSlots();
    if (SCHED_COUNTDOWN == 0 && SCHED_SLOT(SCHED_CUR_SLOT) != 0) {
        Task__Execute();
    }
    Schedule__TickB();
}

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x00177F00 — main-loop call #11 */
void Schedule__FrameTick(void)
{
    Schedule__TickA();          /* actually 0x192C08 in original (sibling) */
    SCHED_FRAME_COUNT++;
}

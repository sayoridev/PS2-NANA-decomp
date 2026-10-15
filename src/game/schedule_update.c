/*
 * PS2-NANA-decomp — sound/scheduler per-frame update
 * STATUS: RECONSTRUCTED (cleaned from raw Ghidra output; disassembly re-check pending)
 * ORIGINAL: SLPM_659.14 @ 0x00177F30, size 0x98
 *
 * Main-loop call #9. Counts down the scheduler gate at 0x008D3900, runs the
 * sound-update bracket and job poll, then executes one queued sound command
 * when the countdown reaches zero.
 *
 * Queue layout recovered from raw decompile and symbol map:
 *   base  0x008D3900 — countdown word
 *   +0x04 active flag for slot 0
 *   slot stride 0x134
 *   execution cursor 0x008D8608
 *   producer cursor  0x008D8604
 *
 * Unknown helpers are intentionally left unnamed.
 */
#include "../common/ps2.h"

typedef struct {
    s32 delay;                 /* +0x00: scheduler countdown */
    s32 active;                /* +0x04: slot-active flag */
    u8  _pad[0x134 - 8];       /* remaining slot state, layout TBD */
} Schedule__TaskSlot;

#define SCHEDULE__TASK_SLOTS  ((Schedule__TaskSlot *)0x008D3900u)
#define SCHEDULE__EXEC_CURSOR (*(s32 *)0x008D8608u)

extern void Sound__UpdateBegin(void);       /* @0x00192BC8 */
extern void Job__PollAll(void);             /* @0x00189468 */
extern void fcn_00177140(void);             /* @0x00177140 — pending classification */
extern void Sound__ProcessCommand(void);    /* @0x00177C80 */
extern void fcn_0017CCB0(void);             /* @0x0017CCB0 — pending classification */

void Schedule__Update(void)
{
    if (SCHEDULE__TASK_SLOTS[0].delay > 0) {
        s32 remaining = SCHEDULE__TASK_SLOTS[0].delay - 1;

        SCHEDULE__TASK_SLOTS[0].delay = remaining;
        if (remaining < 1) {
            SCHEDULE__TASK_SLOTS[0].delay = 0;
        }
    }

    Sound__UpdateBegin();
    Job__PollAll();
    fcn_00177140();

    if (SCHEDULE__TASK_SLOTS[0].delay == 0 &&
        SCHEDULE__TASK_SLOTS[SCHEDULE__EXEC_CURSOR].active != 0) {
        Sound__ProcessCommand();
    }

    fcn_0017CCB0();
}

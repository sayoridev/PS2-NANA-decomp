/*
 * PS2-NANA-decomp — per-frame mode state + pad input processor
 * STATUS: RECONSTRUCTED (decompile-verified, Ghidra R5900)
 * ORIGINAL: SLPM_659.14 @ 0x0013BDE0 — main-loop call #4
 *
 * Processes both mode state blocks (0x180 bytes each at 0x5B2F00/0x5B3080):
 * pad read, edge/repeat button logic, analog deadzone, command state
 * machine, 2-channel interpolation (ramp) engine feeding the DS2 actuators
 * (rumble). Cleanup of the verbatim decompile — see git history for raw.
 */
#include "../common/ps2.h"
#include "pad.h"

/* ---- external services ---- */
extern void Memory__Memset(void *dst, s32 val, s32 n);  /* @0x00129CE8 */
extern s64  Pad__TestButton2(int slot, int);            /* @0x0011BE78 */
extern s64  Pad__TestButton3(int slot, int);            /* @0x0011BE18 */
extern s64  Pad__TestCombo(int slot, int, int, s64);    /* @0x0011BA60 */
extern s64  Pad__TestAnalog(int slot, int, int, s64);   /* @0x0011B928 */
extern s64  Pad__Query(int slot, int, void *);          /* @0x0011BBE0 */
extern s64  Pad__Check(int slot, int, void *);          /* @0x0011B488 */

extern u16 g_PadRepeatDelay;      /* @0x005A34A8 */
extern u16 g_PadRepeatInterval;   /* @0x005A34AC */

/* ---- mode state block ---- */
typedef struct {
    s32  _unk0[0x40];
    s32  rampAltMode;    /* +0x100: when !=0, ramp counters hold            */
    s32  rampACounter;   /* +0x104 */
    s32  rampAMax;       /* +0x108 */
    s32  rampAType;      /* +0x10C: 1=hold, 2=linear, 3=two-phase           */
    u16  _pad110;        /* +0x110 */
    u16  rampAEnd;       /* +0x112 */
    u16  rampAStart;     /* +0x114 */
    s32  rampBCounter;   /* +0x118 */
    s32  rampBMax;       /* +0x11C */
    s32  rampBType;      /* +0x120 */
    u16  _pad124;        /* +0x124 */
    u16  rampBEnd;       /* +0x126 */
    u16  rampBStart;     /* +0x128 */
    char lastKey;        /* +0x12A */
    u8   _pad12b;        /* +0x12B */
    u16  buttons;        /* +0x12C: current frame, active-low INVERTED      */
    char modeState;      /* +0x12E: Pad__ReadStatus result                  */
    char subState;       /* +0x12F */
    s32  command;        /* +0x130: nav command (0x28..0x4D, 99=no pad)     */
    s32  cmdCounter;     /* +0x134 */
    u8   work[6];        /* +0x138 */
    u8   outA;           /* +0x13E: ramp A output (actuator byte)           */
    u8   outB;           /* +0x13F: ramp B output                           */
    u8   _pad140[4];     /* +0x140 */
    u8   scratch[8];     /* +0x144: zeroed each frame                       */
    u8   _pad14c[2];
    u8   analogX;        /* +0x14A: deadzone vs 0x50 / 0xB0                 */
    u8   analogY;        /* +0x14B */
    u8   _pad14e[0x16];
    u16  buttonsPrev;    /* +0x164 */
    s16  idleCounter;    /* +0x166 */
    u16  buttonsPressed; /* +0x168: cur & ~prev                             */
    u16  buttonsRepeat;  /* +0x16A: pressed, or full set on repeat timeout  */
    u16  buttonsReleased;/* +0x16C: ~cur & prev                             */
    /* sizeof == 0x180 */
} Mode__StateBlock;

Mode__StateBlock g_ModeState[2];   /* @0x005B2F00, @0x005B3080 */

/* nav command values */
#define CMD_NONE   0
#define CMD_28     0x28
#define CMD_29     0x29
#define CMD_2A     0x2A
#define CMD_46     0x46
#define CMD_47     0x47
#define CMD_48     0x48
#define CMD_4C     0x4C
#define CMD_4D     0x4D
#define CMD_NOPAD  99     /* 'c' — Pad__ReadStatus for absent pad          */

/* ramp output curve kinds */
#define RAMP_HOLD   1
#define RAMP_LINEAR 2
#define RAMP_TWOPHASE 3

/* advance one ramp channel; original interleaves A(+0x104..) and B(+0x118..) */
static void Mode__RampChannel(u8 *out, s16 *counterFields, s32 altModeHold,
                              u16 start, u16 end, s32 type, s32 max, s32 cur);

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x0013BDE0
 * PURPOSE: per-frame input/state processing for both mode slots. */
void Mode__Update(void)
{
    Mode__StateBlock *st = g_ModeState;

    for (int slot = 0; slot < 2; slot++, st++) {
        u16 prev;

        Memory__Memset(st->scratch, 0, 8);
        Memory__Memset(&st->outA, 0, 6);
        st->buttonsPressed = 0;
        st->buttonsRepeat = 0;
        st->buttonsReleased = 0;
        prev = st->buttonsPrev;
        st->buttonsPrev = 0;
        st->modeState = Pad__ReadStatus(slot, 0);
        if (st->modeState == 0) {
            st->command = 0;
            st->cmdCounter = 0;
        }

        switch (st->command) {
        case CMD_4D:
            if (Pad__TestButton(slot, 0) == 1) st->command--;
            if (Pad__TestButton(slot, 0) == 0) st->command = CMD_NOPAD;
            break;
        case CMD_4C:
            if (Pad__TestButton2(slot, 0) == 1) st->command++;
            break;
        case CMD_48:
            if (Pad__TestButton3(slot, 0) == 1) st->command = CMD_4C;
            else                              st->command = CMD_NOPAD;
            break;
        case CMD_47:
        case_cmd_47:
            if (Pad__TestButton(slot, 0) == 1) st->command--;
            if (Pad__TestButton(slot, 0) == 0) st->command++;
            break;
        case CMD_46:
            Memory__Memset(st->work, 0xff, 6);
            st->work[0] = 0;
            st->work[1] = 1;
            if (Pad__TestButton(slot, 0) != 2) {
                if (Pad__Query(slot, 0, st->work) == 1) {
                    Memory__Memset(st->work, 0, 6);
                    st->command++;
                    goto case_cmd_47;
                }
            }
            break;
        case CMD_2A:
            if (Pad__TestButton(slot, 0) == 1) st->command--;
            if (Pad__TestButton(slot, 0) == 0) st->command = 0;
            break;
        case CMD_29:
        case_cmd_29:
            if (Pad__TestCombo(slot, 0, 1, 3) == 1) st->command++;
            break;
        case CMD_28:
            if (Pad__TestAnalog(slot, 0, 4, -1) != 0) {
                st->command++;
                goto case_cmd_29;
            }
            st->command = CMD_NOPAD;
            break;
        case CMD_NONE:
            if (st->modeState == 6 || st->modeState == 2) {
                st->subState = Pad__TestAnalog(slot, 0, 1, 0);
                if (st->subState != 0) {
                    s64 v = Pad__TestAnalog(slot, 0, 2, 0);
                    if (v > 0) st->subState = (char)v;
                    if      (st->subState == 7) st->command = CMD_46;
                    else if (st->subState == 4) st->command = CMD_28;
                    else                        st->command = CMD_NOPAD;
                }
            }
            break;
        default:
            if (st->modeState == 7) st->cmdCounter++;
            if ((st->modeState == 6 || st->modeState == 2) &&
                Pad__Check(slot, 0, st->scratch) != 0) {
                if (st->subState == 7) {
                    /* ramp engine: interpolate both channels, then push out */
                    Memory__Memset(&st->outA, 0, 6);
                    s32 maxA = *(s32 *)&st->rampAMax;
                    s32 curA = st->rampACounter;
                    if (curA < maxA) {
                        if (st->rampAltMode == 0) {
                            /* type 3: two-phase, type 2: linear, type 1: hold */
                            s32 half = maxA >> 1;
                            switch (st->rampAType) {
                            case RAMP_TWOPHASE:
                                if (maxA < 0) half = (maxA + 1) >> 1;
                                if (curA < half) {
                                    st->outA = (u8)((char)st->rampAStart +
                                        (char)((curA * ((int)st->rampAEnd - (int)st->rampAStart)) / half));
                                } else {
                                    s32 h2 = half;
                                    if (maxA < 0) h2 = (maxA + 1) >> 1;
                                    st->outA = (u8)(((char)st->rampAEnd +
                                        (char)(((int)st->rampAStart - (int)st->rampAEnd) * (curA - h2) / h2)) | 1);
                                }
                                break;
                            case RAMP_LINEAR:
                                st->outA = (u8)(((char)st->rampAStart +
                                    (char)((curA * ((int)st->rampAEnd - (int)st->rampAStart)) / maxA)) | 1);
                                break;
                            case RAMP_HOLD:
                                st->outA = (u8)(st->rampAEnd | 1);
                                break;
                            }
                        }
                        st->rampACounter++;
                    } else {
                        st->rampAType = 0;
                    }
                    /* channel B — same pattern on +0x118.. (verbatim in raw) */
                    s32 maxB = *(s32 *)&st->rampBMax;
                    s32 curB = st->rampBCounter;
                    if (curB < maxB) {
                        if (st->rampAltMode == 0) {
                            s32 half = maxB >> 1;
                            switch (st->rampBType) {
                            case RAMP_TWOPHASE:
                                if (maxB < 0) half = (maxB + 1) >> 1;
                                if (curB < half) {
                                    st->outB = (u8)((char)st->rampBStart +
                                        (char)((curB * ((int)st->rampBEnd - (int)st->rampBStart)) / half));
                                } else {
                                    s32 h2 = half;
                                    if (maxB < 0) h2 = (maxB + 1) >> 1;
                                    st->outB = (u8)((char)st->rampBEnd +
                                        (char)(((int)st->rampBStart - (int)st->rampBEnd) * (curB - h2) / h2));
                                }
                                break;
                            case RAMP_LINEAR:
                                st->outB = (u8)((char)st->rampBStart +
                                    (char)((curB * ((int)st->rampBEnd - (int)st->rampBStart)) / maxB));
                                break;
                            case RAMP_HOLD:
                                st->outB = (u8)((char)st->rampBStart);
                                break;
                            }
                        }
                        st->rampBCounter++;
                    } else {
                        st->rampBType = 0;
                    }
                    Pad__SetOutputs(slot, 0, &st->outA);
                } else {
                    /* reset all ramp state when not in commit state */
                    st->rampACounter = 0; st->rampAMax = 0; st->rampAType = 0;
                    *(u16 *)&st->_pad110 = 0; st->rampAEnd = 0; st->rampAStart = 0;
                    st->rampBCounter = 0; st->rampBMax = 0; st->rampBType = 0;
                    *(u16 *)&st->_pad124 = 0; st->rampBEnd = 0; st->rampBStart = 0;
                }
                /* button word: pack + invert + analog deadzone flags */
                u8 hi = *(u8 *)((u8 *)st + 0x146);
                st->buttons = (u16)((hi << 8) | *(u8 *)((u8 *)st + 0x147)) ^ 0xffff;
                u8 key = *(u8 *)((u8 *)st + 0x145);
                *(u8 *)((u8 *)st + 0x12b) = key;
                if (st->lastKey != 0 && key != st->lastKey) {
                    st->lastKey = 0;
                    st->command = 0;
                }
                st->lastKey = (char)key;
                if ((key >> 4) == 7 && (st->buttons & 0xf000) == 0) {
                    if (st->analogX < 0x50)       st->buttons |= 0x8000;
                    else if (st->analogX > 0xb0)  st->buttons |= 0x2000;
                    if (st->analogY < 0x50)       st->buttons |= 0x1000;
                    else if (st->analogY > 0xb0)  st->buttons |= 0x4000;
                }
                st->buttonsPressed  = st->buttons & (prev ^ st->buttons);
                st->buttonsReleased = (prev ^ st->buttons) & ~st->buttons;
                st->buttonsRepeat   = st->buttonsPressed;
                if (st->buttonsPressed == 0) {
                    st->idleCounter = (s16)(st->idleCounter + 1);
                    if (g_PadRepeatDelay < st->idleCounter &&
                        (st->idleCounter % g_PadRepeatInterval) == 0) {
                        st->buttonsRepeat = st->buttons;
                    }
                } else {
                    st->idleCounter = 0;
                }
                st->buttonsPrev = st->buttons;
            }
            break;
        }
    }
}

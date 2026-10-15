/*
 * PS2-NANA-decomp — pad driver public interface
 * STATUS: RECONSTRUCTED
 */
#ifndef PAD_H
#define PAD_H

#include "../common/ps2.h"

s32 Pad__GetStateRaw(int slot, int sub, void *out); /* NULL out = return record ptr */
char Pad__ReadStatus(int slot, int sub);
u8   Pad__TestButton(int slot, int sub);
s32  Pad__SetOutputs(int slot, int sub, u8 *out6);

/* pad status enum (Pad__ReadStatus results) */
#define PAD_STATE_NOPAD  99   /* 'c' — pad absent                          */
#define PAD_STATE_5      5    /* remapped from (6,2)                       */
#define PAD_STATE_6      6    /* intermediate (read in progress)           */
#define PAD_STATE_7      7    /* button held                               */

#endif

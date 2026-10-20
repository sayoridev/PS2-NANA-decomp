/*
 * PS2-NANA-decomp — scene transition controller
 * STATUS: RECONSTRUCTED (decompile-verified, Ghidra R5900)
 * ORIGINAL: SLPM_659.14 @ 0x00146040 — main-loop call #12
 *
 * Drives scene fades: fade-in (2 phases) / hold / fade-out (2 phases) with
 * per-item volume ramp on a parallel array (+0x200, floor -0x80, slope
 * 0x80/n) and DMA packet scheduling into g_DmaPacketBuf. State 7 calls
 * Module__LoadRequest(1) — overlays are loaded mid-transition.
 * Cleanup of the verbatim decompile — see git history for raw.
 */
#include "../common/ps2.h"

/* ---- display/DMA family ---- */
extern void *Display__BeginContext(int);            /* @0x001347F0 */
extern void Display__EndContext(void);             /* @0x00134790 */
extern void Draw__SetItemCount(int id, int count); /* @0x00139470 */
extern void Draw__SubmitPacket(void *ctx, int id, void *packet); /* @0x00138700 */
extern void Module__LoadRequest(int cmd);          /* @0x00142630 */

/* ---- fade tables ---- */
typedef struct {              /* 0x2C-byte per-mode fade entry @ 0x001C9E40 */
    s32 _unk[2];              /* +0x00 */
    s32 fadeA;                /* +0x08 */
    s32 _u0c;                 /* +0x0C */
    s32 fadeB;                /* +0x10 */
    s32 loopVal;              /* +0x14 */
    s32 loopMax;              /* +0x18 */
    s32 _u1c;                 /* +0x1C */
    s32 fadeC;                /* +0x20 */
    s32 _u24;                 /* +0x24 */
    s32 fadeOut;              /* +0x28 */
} Scene__FadeEntry;

extern s32          g_FadeCounts[];   /* @0x001C9E20 per-mode item count */
extern Scene__FadeEntry g_FadeTable[];/* @0x001C9E40 */
extern s32          g_SceneTiming[];  /* @0x001CA164 per-scene 0x10B: +0 max, +8 hold */
extern s32          g_SceneItemModes[];/* @0x005BB830 0xC B per mode (0 = simple) */

/* ---- scene context globals (struct base @ 0x005BB880) ---- */
#define SCENE_ITEM(i)     (*(s32 *)(0x005BB880 + 0x18 + (i) * 4))
#define SCENE_COUNTERS    ((s32 *)0x005BB880 + 0xbc)   /* per-item s32 @ +0xBC+4i */
#define SCENE_STATE       (*(volatile s32 *)0x005BB884)
#define SCENE_FLAG        (*(volatile s32 *)0x005BB888)
#define SCENE_COUNTDOWN   (*(volatile s32 *)0x005BB960)
#define SCENE_TIME        (*(volatile s32 *)0x005BB968)
#define SCENE_TIME_LIMIT  (*(volatile s32 *)0x005BB964)  /* index into g_SceneTiming */
#define SCENE_SUSPEND     (*(volatile s32 *)0x005BB97C)
#define SCENE_MODE_IDX    (*(volatile s32 *)0x005BB880)
#define SCENE_ITEM_VOLUME(i) (*(s32 *)(0x005BB880 + 200 + (i) * 4))
#define SCENE_ITEM_SUBCNT(i) (*(s32 *)(0x005BB880 + 0xd4 + (i) * 4))
#define SCENE_MODE_FLAGS(mode, i) (*(s32 *)(0x005BB830 + (mode) * 0xc + (i) * 4))

/* fade slope constants */
#define FADE_FLOOR   (-0x80)
#define FADE_STEP(n) (0x80 / (n))

enum {
    SCENE_IDLE = 0,
    SCENE_FADE_IN_A = 1,
    SCENE_FADE_IN_B = 2, SCENE_FADE_IN_B2 = 3,
    SCENE_HOLD = 4, SCENE_HOLD2 = 5, SCENE_HOLD3 = 6,
    SCENE_FADE_OUT = 7,
    SCENE_FADE_OUT_B = 8, SCENE_FADE_OUT_B2 = 9,
    SCENE_RESET = 10
};

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x00146040
 * PURPOSE: advance the scene transition state machine once per frame and
 *          schedule per-item DMA packets. */
void Scene__Update(void)
{
    int i, done;
    int mode = SCENE_MODE_IDX;

    if (SCENE_STATE == 0) return;

    /* per-item counters ++ */
    for (i = 0; i < g_FadeCounts[mode]; i++) {
        ((volatile s32 *)0x005BB880)[(0xbc/4) + i]++;
    }
    /* global countdown / time */
    if (--SCENE_COUNTDOWN < 1) SCENE_COUNTDOWN = 0;
    if (SCENE_COUNTDOWN < 1)   SCENE_TIME++;

    switch (SCENE_STATE) {
    case SCENE_FADE_IN_A:
        done = 0;
        for (i = 0; i < g_FadeCounts[mode]; i++) {
            if (g_FadeTable[mode*3].fadeA <= SCENE_COUNTERS[i]) {   /* table idx math per raw */
                SCENE_STATE = SCENE_FADE_IN_B;
                SCENE_COUNTERS[i] = g_FadeTable[mode*3].fadeA;
            }
            SCENE_COUNTERS[200/4 + i] = 0;   /* volume sub-array reset, per raw */
        }
        if (g_SceneTiming[SCENE_TIME_LIMIT*4] <= SCENE_TIME)
            SCENE_TIME = g_SceneTiming[SCENE_TIME_LIMIT*4];
        break;

    case SCENE_FADE_IN_B:
    case SCENE_FADE_IN_B2:
        SCENE_STATE = SCENE_FADE_IN_B2;
        done = 0;
        for (i = 0; i < g_FadeCounts[mode]; i++) {
            if (g_FadeTable[mode*3].fadeB <= SCENE_COUNTERS[i]) {
                SCENE_COUNTERS[i] = g_FadeTable[mode*3].fadeB;
                done++;
            }
        }
        {
            int lim = g_SceneTiming[SCENE_TIME_LIMIT*4];
            int reached = (lim <= SCENE_TIME);
            if (reached) SCENE_TIME = lim;
            if (g_FadeCounts[mode] <= done && reached) SCENE_STATE = SCENE_HOLD;
        }
        break;

    case SCENE_HOLD:
    case SCENE_HOLD2:
    case SCENE_HOLD3:
        SCENE_STATE = SCENE_HOLD2;
        done = 0;
        for (i = 0; i < g_FadeCounts[mode]; i++) {
            if (g_FadeTable[mode*3].loopMax <= SCENE_COUNTERS[i]) {
                done++;
                SCENE_COUNTERS[i] = g_FadeTable[mode*3].loopVal;
            }
        }
        if (g_FadeCounts[mode] <= done) SCENE_STATE = SCENE_HOLD3;
        SCENE_TIME = g_SceneTiming[SCENE_TIME_LIMIT*4];
        break;

    case SCENE_FADE_OUT:
        Module__LoadRequest(1);                 /* overlay load mid-transition */
        SCENE_STATE = SCENE_FADE_OUT;
        done = 0;
        for (i = 0; i < g_FadeCounts[mode]; i++) {
            if (SCENE_MODE_FLAGS(mode, i) == 0) {
                if (g_FadeTable[mode*3].fadeOut <= SCENE_COUNTERS[i]) {
                    SCENE_COUNTERS[i] = g_FadeTable[mode*3].fadeOut;
                    done++;
                }
            } else {
                if (g_FadeTable[mode*3].loopMax <= SCENE_COUNTERS[i])
                    SCENE_COUNTERS[i] = g_FadeTable[mode*3].loopVal;
                if (g_FadeTable[mode*3].fadeOut < 1) {
                    SCENE_ITEM_VOLUME(i) = FADE_FLOOR;
                } else {
                    SCENE_ITEM_VOLUME(i) -= FADE_STEP(g_FadeTable[mode*3].fadeOut);
                    if (SCENE_ITEM_VOLUME(i) < FADE_FLOOR)
                        SCENE_ITEM_VOLUME(i) = FADE_FLOOR;
                }
                SCENE_ITEM_SUBCNT(i)++;
                if (g_FadeTable[mode*3].fadeOut <= SCENE_ITEM_SUBCNT(i)) done++;
            }
        }
        if (g_FadeCounts[mode] <= done) SCENE_STATE = SCENE_FADE_OUT_B;
        if (g_SceneTiming[SCENE_TIME_LIMIT*4 + 2] <= SCENE_TIME)
            SCENE_TIME = g_SceneTiming[SCENE_TIME_LIMIT*4 + 2];
        break;

    case SCENE_FADE_OUT_B:
    case SCENE_FADE_OUT_B2:
        SCENE_STATE = SCENE_FADE_OUT_B2;
        done = 0;
        for (i = 0; i < g_FadeCounts[mode]; i++) {
            if (SCENE_MODE_FLAGS(mode, i) == 0) {
                if (g_FadeTable[mode*3].fadeOut <= SCENE_COUNTERS[i]) {
                    SCENE_COUNTERS[i] = g_FadeTable[mode*3].fadeOut;
                    done++;
                }
            } else {
                if (g_FadeTable[mode*3].loopMax <= SCENE_COUNTERS[i])
                    SCENE_COUNTERS[i] = g_FadeTable[mode*3].loopVal;
                if (g_FadeTable[mode*3].fadeOut < 1) {
                    SCENE_ITEM_VOLUME(i) = FADE_FLOOR;
                } else {
                    SCENE_ITEM_VOLUME(i) -= FADE_STEP(g_FadeTable[mode*3].fadeOut);
                    if (SCENE_ITEM_VOLUME(i) < FADE_FLOOR)
                        SCENE_ITEM_VOLUME(i) = FADE_FLOOR;
                }
                SCENE_ITEM_SUBCNT(i)++;
                if (g_FadeTable[mode*3].fadeOut <= SCENE_ITEM_SUBCNT(i)) done++;
            }
        }
        {
            int lim = g_SceneTiming[SCENE_TIME_LIMIT*4 + 2];
            int reached = (lim <= SCENE_TIME);
            if (reached) SCENE_TIME = lim;
            if (g_FadeCounts[mode] <= done && reached) {
                SCENE_FLAG = 0;
                SCENE_STATE = SCENE_RESET + 0;  /* 10 */
            }
        }
        break;

    case SCENE_RESET:
        for (i = 0; i < g_FadeCounts[mode]; i++) {
            SCENE_COUNTERS[i] = g_FadeTable[mode*3].fadeOut;
        }
        SCENE_STATE = SCENE_IDLE;
        SCENE_TIME = g_SceneTiming[SCENE_TIME_LIMIT*4 + 2];
        /* fallthrough */
    case SCENE_IDLE:
        SCENE_FLAG = 0;
        break;
    }

    /* DMA packet scheduling */
    if (SCENE_SUSPEND == 0) {
        void *ctx = Display__BeginContext(8);
        if (SCENE_FLAG == 1) {
            for (i = 0; i < g_FadeCounts[mode]; i++) {
                volatile u32 *pkt = (volatile u32 *)0x005BB8A8;
                pkt[0x5c/4] = 0;           /* 0x5BC04 region fields, per raw */
                pkt[0x18/4] = 4;
                pkt[0x60/4] = 0;
                pkt[0x64/4] = 0;
                pkt[0x68/4] = SCENE_COUNTERS[200/4 + i];
                pkt[0x14/4] = 0x100;
                pkt[0x08/4] = 0x100;
                pkt[0x0c/4] = 0x100;
                pkt[0x10/4] = 0x100;
                Draw__SetItemCount(SCENE_ITEM(i), SCENE_COUNTERS[i] - 1);
                *(volatile u32 *)0x005BB8A8 = *(u32 *)(*(u32 *)0x005A3680 + 0x48);
                Draw__SubmitPacket(ctx, SCENE_ITEM(i), (void *)0x005BB8A8);
            }
            if (SCENE_COUNTDOWN != 0 && SCENE_COUNTDOWN < 1) {
                /* unreachable guard kept from raw structure */
            }
            if (*(s32 *)0x005BB970 != 0 && SCENE_COUNTDOWN < 1) {
                *(volatile u32 *)(0x005BB8A8 + 0x18) = 0;
                Draw__SetItemCount(*(s32 *)0x005BB970, SCENE_TIME - 1);
                Draw__SubmitPacket(ctx, *(s32 *)0x005BB970, (void *)0x005BB8A8);
            }
        }
        Display__EndContext();
    }
}

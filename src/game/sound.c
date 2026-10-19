/*
 * PS2-NANA-decomp — sound system (command VM + stream channels + async jobs)
 * STATUS: RECONSTRUCTED (decompile-verified, Ghidra R5900)
 * ORIGINAL: SLPM_659.14 — dispatcher @0x00177C80, tick @0x00177F30
 *
 * The "scheduler" IS the sound system: a 64-slot x 0x134-byte pending-command
 * queue processed one command per tick. Commands 1-10/0xC-0x13 implement
 * BGM play (30-entry table -> RWAB/BGM_*.RWS), voice play (25 banks of 500
 * lines -> RWAV/V*.RWS, 12,311 lines total), a second SE bank (gp table),
 * stream channel management, and a deferred-callback job executor.
 * State struct: 0x008D8624..0x008D86E0. Stream channels: 0x8D86D0[4].
 */
#include "../common/ps2.h"

/* ---- RPC / timing / stream primitives ---- */
extern void Schedule__TickA(void);              /* @0x00192BC8: g_SoundUpdating=1, Rwa__BeginUpdate */
extern void Sound__UpdateEnd(void);             /* @0x00192C08: Rwa__EndUpdate, flag=0, frame++ */
extern void Job__PollAll(void);                 /* @0x00189468: walk job registry, fire vmethod+0x3C */
extern void Sound__WaitCh0(int);                /* @0x00177710 */
extern void Sound__WaitCh1(int);                /* @0x001776B0 */
extern void Sound__WaitCh2(s32);                /* @0x00177710 variant ch2  */
extern void Sound__WaitCh3(s32);                /* @0x001776B0 variant ch3  */
extern s32  Stream__Open(s32 rwa, int,int,int,int); /* @0x001A2E00 */
extern void Stream__CloseWait(s32 h, int, int); /* @0x0018F600 */
extern void Stream__SendConfig(s32 h, void *cfg0x24); /* @0x001A3080 */
extern void Stream__SendCmd(s32 h, void *env);  /* @0x001A2AE8 */
extern s64  Rpc__Call(s32 port, s32 fn, s32, s32, s32, s32, s32, s32); /* @0x00112868 */
extern void Memory__Memset(void *, s32, s32);   /* @0x00129CE8 */
extern void Memory__Copy(void *, void *, s32);  /* @0x00129B30 */

/* ---- sound state ---- */
typedef struct {                    /* @0x008D8624..0x008D86E0 */
    s32  rwaRoot;                   /* +0x00: RWA driver root handle          */
    s32  bgmIndex;                  /* +0x04 -> 0x8D8628                      */
    u32  bgmState;                  /* +0x08: 1 play / 6,0x11 modes / 0xE idle */
    s32  bgmArg;                    /* +0x0C                                  */
    s32  bgmDesc;                   /* +0x10: from g_BgmTable[idx]            */
    s32  bgmCounter;                /* +0x14                                  */
    f32  bgmVolume;                 /* +0x18: 1.0f typical                    */
    /* voice block +0x34 (0x8D8658) */
    s32  voiceId;                   /* +0x34                                  */
    u32  voiceMode;                 /* +0x38: 1 play / 4 loop                 */
    s32  voiceBank;                 /* +0x3C                                  */
    s32  voiceLocal;                /* +0x40                                  */
    u32  voiceFlag;                 /* +0x44                                  */
    f32  voiceVolume;               /* +0x48                                  */
    /* bank2 (SE) block +0x58 (0x8D867C) */
    s32  seId; u32 seMode; u32 seFlag; s32 seLocal; s32 seBank;
    s32  seArg; f32 seVolume; s32  seSub0;
    s32  seSub1; u32 seSub2;
    s32  streamCh[4];               /* +0xAC (0x8D86D0): ch0/1 BGM, ch2 voice, ch3 SE */
    s32  _pad;
} Sound__State;
extern Sound__State g_Sound;

extern s32  g_BgmTable[][3];        /* @0x001F5E5C: 30 x {name,path,param}    */
extern s32  g_VoiceBankTable[][2];  /* @0x001F6070: 25 x {minId,maxId}        */
extern s32  g_SeBankTable[][2];     /* @0x005A3388 (gp+0xFFFF8118): 1 range   */
extern s32  g_BgmDescTable[];       /* @0x001F5E58 (legacy alias)             */

extern s32  (*g_JobCb)(s32, s32);   /* @0x007D1FC4                            */
extern s32  g_JobCbArg;             /* @0x007D1FC8                            */

/* ---- job executor (type 0x11) ---- */
extern s32  WaitSema(s32);                      /* @0x0010E960 */
extern s32  SignalSema(s32);                    /* @0x0010E940 */
void Sound__DeferBgmB(s32, s32);
void Command__Requeue(s32, s32);
void Sound__DeferBgmA(s32, s32);
void Stream__Resume(s32, int);
void Stream__Control(s32, int);
extern s32  fcn_00190CD0(s32);                  /* @0x00190CD0 — object probe */
extern void fcn_00190C40(s32, s32, s32);        /* @0x00190C40 — object init */
extern s32  fcn_00191300(s32, s32, s32, s32, s32); /* @0x00191300 — ABI-sensitive registration, fifth arg in t0 */
extern s32  fcn_001A4DA0(void);                 /* @0x001A4DA0 — subsystem probe */
extern s32  fcn_001A6730(void);                 /* @0x001A6730 — subsystem probe */
extern s32  fcn_001A73F0(void);                 /* @0x001A73F0 — subsystem probe */
extern s32  fcn_001A2F00(void);                 /* @0x001A2F00 — subsystem probe */
extern void fcn_001A74A8(void);                 /* @0x001A74A8 — subsystem teardown */
extern void fcn_001A66F0(void);                 /* @0x001A66F0 — subsystem teardown */
extern void fcn_001A4D60(void);                 /* @0x001A4D60 — subsystem teardown */
extern s32  fcn_00190AF8(s32, s32);             /* @0x00190AF8 — job class registration */
extern s32  fcn_0018DB90(s32, s32);             /* @0x0018DB90 — job class registration */
extern s32  fcn_00199DD8(s32, s32);             /* @0x00199DD8 — job class registration */
extern void fcn_001A4C98(s32, s32, s32, s32);   /* @0x001A4C98 — subsystem init */
extern s32  GetThreadId(void);                  /* @0x0010E810 */
extern void SleepThread(void);                  /* @0x0010E840 */
extern void fcn_001A2080(s32, s32, s32, s32, s32, s32, s32, s32); /* @0x001A2080 — custom ABI */
extern s32  fcn_001A0D30(s32, s32, s32);        /* @0x001A0D30 — object factory */
extern s32  fcn_001A6610(s32, s32, s32, s32);   /* @0x001A6610 — record factory */
extern void fcn_00177320(void);                 /* @0x00177320 — name pending */
extern void fcn_00192AA0(s32, s32, s32, s32, s32, s32, s32, s32); /* @0x00192AA0 */
extern s32  g_CurrentJob;                       /* @0x007D1FC0 */
extern u32  g_JobState;                         /* @0x007D20D8 */
extern u32  g_JobFrames;                        /* @0x007D20DC */

typedef struct Schedule__Job Schedule__Job;
typedef struct Schedule__JobVtable Schedule__JobVtable;

struct Schedule__Job {
    s32 _pad[0x50 / 4];
    Schedule__JobVtable *vtable;                /* +0x50 */
};

struct Schedule__JobVtable {
    void (*_reserved[11])(void);                /* +0x00..+0x28 */
    void (*method_2C)(Schedule__Job *);         /* +0x2C */
    void (*method_30)(Schedule__Job *, s32, s32); /* +0x30 */
    void (*method_34)(void);                    /* +0x34 */
    void (*method_38)(Schedule__Job *, s32, s32, s32); /* +0x38 */
    s32  (*method_3C)(Schedule__Job *, s32);    /* +0x3C */
};

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00177EC0
 * PURPOSE: read the deferred-job completion flag. */
s32 Job__ForceFlag(void)
{
    return (*(volatile s32 *)0x008E860C);
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00177ED0
 * PURPOSE: set the scheduler countdown and clear the completion flag. */
void Schedule__Kick(int countdown)
{
    (*(volatile s32 *)0x008D3900) = countdown;
    (*(volatile s32 *)0x008E860C) = 0;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00177EF0
 * PURPOSE: read the scheduler countdown; 0 means idle. */
s32 Job__Poll(void)
{
    return (*(volatile s32 *)0x008D3900);
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x0018A9E0, size 0x08
 * PURPOSE: return the scheduler subsystem handle singleton. */
s32 fcn_0018A9E0(void)
{
    return (*(volatile s32 *)0x008DD380);
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x0018A9F0, size 0x08 */
s32 FUN_0018A9F0(s32 handle)
{
    return (*(volatile s32 *)(u32)(handle + 0x64));
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x0018A9F8, size 0x08 */
void FUN_0018A9F8(s32 handle, s32 value)
{
    (*(volatile s32 *)(u32)(handle + 0x68)) = value;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x0019F788, size 0x70
 * PURPOSE: initialize/register the object at 0x008DC98. The final
 *          registration call uses a custom fifth argument in $t0, so the
 *          C call is ABI-sensitive and remains structural. */
s32 fcn_0019F788(void)
{
    if (fcn_00190CD0(0x008DC98) != 0) {
        fcn_00190C40(0x008DC98, 0x0035DD90, 0);
        return fcn_00191300(0x008DC98,
                            0x00206B50,
                            6,
                            0x00207A50,
                            9);
    }
    return 0;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x001A39E8, size 0x6C
 * PURPOSE: conditional fallback/teardown sequence for scheduler subsystems. */
void fcn_001A39E8(void)
{
    if (fcn_001A4DA0() != 0) {
        if (fcn_001A6730() != 0) {
            if (fcn_001A73F0() != 0) {
                if (fcn_001A2F00() == 0) {
                    fcn_001A74A8();
                    fcn_001A66F0();
                }
            }
        }
    }
    fcn_001A4D60();
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x00192D50, size 0x7C
 * PURPOSE: register the three fallback job classes used by the scheduler. */
s32 fcn_00192D50(void)
{
    if (fcn_00190AF8(0x001F8350, 3) != 1) {
        return 0;
    }
    if (fcn_0018DB90(0x001F8348, 1) != 1) {
        return 0;
    }
    if (fcn_00199DD8(0x001F8360, 1) != 1) {
        return 0;
    }
    return 1;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x001A4EB8, size 0x50
 * PURPOSE: register the current subsystem handle, start its worker with a
 *          thread-aware custom-ABI call, then sleep the calling thread. */
void fcn_001A4EB8(s32 handle)
{
    s32 thread_id;

    (*(volatile s32 *)0x005A3580) = handle;
    thread_id = GetThreadId();

    /* fcn_001A2080 uses a custom eight-register ABI. */
    fcn_001A2080(0, 0, 0, 48, 0, thread_id, (*(volatile s32 *)0x005A3580), 0);

    SleepThread();
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x001A4F08, size 0x2C */
void FUN_001A4F08(s32 base, s32 src)
{
    volatile s32 *out = (volatile s32 *)(u32)(base + 0x40);
    const volatile u16 *in16 = (const volatile u16 *)(u32)src;

    out[0x196] = (*(volatile s32 *)(u32)src);
    ((volatile u16 *)out)[0x32E] = in16[2];
    ((volatile u16 *)out)[0x32F] = in16[3];
    out[0x19A] = (*(volatile s32 *)(u32)(src + 8));
    out[0x19B] = (*(volatile s32 *)(u32)(src + 12));
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x001A4F38, size 0x2C */
void FUN_001A4F38(s32 base, s32 dst)
{
    volatile s32 *in = (volatile s32 *)(u32)(base + 0x40);
    volatile u16 *out16 = (volatile u16 *)(u32)dst;

    (*(volatile s32 *)(u32)dst) = in[0x196];
    out16[2] = ((const volatile u16 *)in)[0x32E];
    out16[3] = ((const volatile u16 *)in)[0x32F];
    (*(volatile s32 *)(u32)(dst + 8)) = in[0x19A];
    (*(volatile s32 *)(u32)(dst + 12)) = in[0x19B];
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x0017CCF0, size 0x1D8
 * PURPOSE: initialize the 1620-byte scheduler subsystem context at 0x008D8620
 *          and its twelve 4-byte-spaced records. */
void fcn_0017CCF0(void)
{
    volatile s32 *ctx = (volatile s32 *)0x008D8620u;
    s32 handle;
    s32 object;
    s32 i;
    s32 step;

    fcn_00129CE8(0x008D8620, 0, 1620);

    fcn_001A4C98(0, 6, 0, 0);
    handle = fcn_0018A9E0();
    (*(volatile s32 *)0x008D8624) = handle;
    fcn_001A4EB8(handle);

    object = fcn_001A0D30(4, 0, 0);
    (*(volatile s32 *)0x008D8620) = object;
    (*(volatile s32 *)(u32)(handle + 0x1418)) = object;

    for (i = 0; i < 12; i++) {
        volatile s32 *entry = ctx + i;
        s32 j;

        for (j = 0; j < 16; j++) {
            entry[0x2C + j] = 0;
        }

        entry[0x61] = fcn_001A6610(entry[1], 0, 0, 0);
        entry[0x73] = -1;
        entry[0x85] = -1;
    }

    step = 0;
    for (;;) {
        volatile s32 *block = ctx + step;
        s32 j;

        for (j = 0; j < 8; j++) {
            block[0x9D + j] = 0;
            block[0x101 + j] = -1;
        }

        step += 8;
        if (step >= 92) {
            break;
        }
    }

    {
        volatile s32 *tail = ctx + step;
        s32 j;

        for (j = 0; j < 2; j++) {
            tail[0x9D + j] = 0;
            tail[0x101 + j] = -1;
        }
    }

    ctx[2] = -1;    /* +0x008 */
    ctx[11] = -1;   /* +0x02C */
    ctx[12] = -1;   /* +0x030 */
    ctx[14] = -1;   /* +0x038 */
    ctx[15] = -1;   /* +0x03C */
    ctx[16] = -1;   /* +0x040 */
    ctx[17] = -1;   /* +0x044 */
    ctx[20] = -1;   /* +0x050 */
    ctx[21] = -1;   /* +0x054 */
    ctx[23] = -1;   /* +0x05C */
    ctx[24] = -1;   /* +0x060 */
    ctx[25] = -1;   /* +0x064 */
    ctx[26] = -1;   /* +0x068 */
    ctx[34] = -1;   /* +0x088 */
    ctx[35] = -1;   /* +0x08C */
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00177FD0
 * PURPOSE: initialize scheduler state, allocator anchors and linked-list head,
 *          then initialize the surrounding scheduler/RWA subsystems. */
void Schedule__Init(void)
{
    s32 init_args[4];
    s32 aligned_ctx;
    s32 extended_ctx;
    s32 result;
    volatile s32 *list_head;
    volatile s32 *node;

    Memory__Memset((void *)0x008D3900, 0, 0x4D14);

    aligned_ctx = (s32)((0x007D3900u + 15u) & ~15u); /* 0x007D3910 */
    extended_ctx = aligned_ctx + 0x100000;

    (*(volatile s32 *)0x005A38B0) = extended_ctx;
    (*(volatile s32 *)0x005A38A8) = aligned_ctx;
    (*(volatile s32 *)0x005A38AC) = aligned_ctx;

    (*(volatile s32 *)(u32)(aligned_ctx + 0x04)) = 0;
    (*(volatile s32 *)(u32)(aligned_ctx + 0x08)) = 0;
    (*(volatile s32 *)(u32)(aligned_ctx + 0x0C)) = 0;
    (*(volatile s32 *)(u32)aligned_ctx) = 0;

    init_args[0] = 0x00177550;
    init_args[1] = 0x00177400;
    init_args[2] = 0x001774D0;
    init_args[3] = 0x00177480;

    fcn_00192AA0((s32)init_args,
                 0,
                 0x00177400,
                 0x00177550,
                 init_args[0],
                 init_args[1],
                 init_args[2],
                 init_args[3]);

    (*(volatile s32 *)0x005A3494) = fcn_00192D50();

    result = fcn_0019F788();
    if (result != 0) {
        fcn_001A39E8();
    }

    fcn_00177320();
    fcn_0017CCF0();

    list_head = (volatile s32 *)0x005A38AC;
    node = (volatile s32 *)(u32)*list_head;
    while (node != 0 && ((volatile s32 *)(u32)node)[1] != 0) {
        node = (volatile s32 *)(u32)((volatile s32 *)(u32)node)[1];
    }
    (void)node;
}

/* ================= command implementations ================= */

/* cmd 1/2 — Bgm__Play mode A (0xE/1) / mode B (0x11/6) */
undefined4 Bgm__Play(s32 idx, s32 arg, int modeB)
{
    if (Job__Poll() == 0) {
        Bgm__StopAll();
        g_Sound.bgmIndex  = idx;
        g_Sound.bgmState  = modeB ? 0x11 : 0xE;
        g_Sound.bgmArg    = arg;
        g_Sound.bgmCounter = 0;
        g_Sound.bgmDesc   = g_BgmDescTable[idx * 3];
        if (g_Sound.bgmDesc != 0) g_Sound.bgmState = modeB ? 6 : 1;
        g_Sound.bgmVolume = 1.0f;
    } else {
        if (modeB) Sound__DeferBgmB(idx, arg); else Sound__DeferBgmA(idx, arg);
    }
    return 1;
}

/* cmd 3 — stop BGM channels 0/1, reset state */
void Bgm__StopAll(void)
{
    for (int ch = 0; ch < 2; ch++) {
        s32 h = g_Sound.streamCh[ch];
        if (h != 0) {
            if (Job__Poll() == 0) { Stream__Control(h, 1); Schedule__Kick(6); }
            else { if (ch == 0) Sound__WaitCh0(0); else Sound__WaitCh1(0); }
        }
        if (g_Sound.streamCh[ch] != 0) {
            if (Job__Poll() == 0) {
                Stream__CloseWait(g_Sound.streamCh[ch], 0, 0);
                g_Sound.streamCh[ch] = 0;
                Schedule__Kick(6);
            } else { if (ch == 0) Sound__WaitCh1(0); else Sound__WaitCh0(0); }
        }
    }
    g_Sound.bgmState = 0;
    g_Sound.bgmIndex = -1;
    g_Sound.bgmDesc  = 0;   /* 0x8D864C..0x8D8650 reset per raw */
    g_Sound.bgmCounter = 0;
}

/* cmd 4/5 — Voice__Play once/loop: resolve global id -> {bank, local} */
undefined4 Voice__Play(s32 voiceId, int loop)
{
    int bank = -1;
    if (Job__Poll() != 0) { (loop ? Sound__WaitCh2 : Sound__WaitCh2)(voiceId); return 1; }
    if (g_Sound.streamCh[2] != 0) {
        if (Job__Poll() == 0) { Stream__Control(g_Sound.streamCh[2], 1); Schedule__Kick(6); }
        else { Sound__WaitCh2(2); }
    }
    g_Sound.seSub2 = -1;
    for (int i = 0; i < 25; i++) {
        if (g_VoiceBankTable[i][0] <= voiceId && voiceId <= g_VoiceBankTable[i][1]) {
            bank = i;
            g_Sound.voiceLocal = voiceId - g_VoiceBankTable[i][0];
            g_Sound.voiceBank  = i;
            break;
        }
    }
    if (bank < 0) {
        g_Sound.voiceId  = -1; g_Sound.voiceFlag = 0;
        g_Sound.seSub0 = -1;   g_Sound.seSub1 = -1;
        return 0;
    }
    g_Sound.voiceFlag  = loop ? 4 : 1;
    g_Sound.voiceVolume = 1.0f;
    g_Sound.voiceId    = voiceId;
    return 1;
}

/* cmd 6 — Voice__Stop */
void Voice__Stop(void)
{
    if (g_Sound.streamCh[2] != 0) {
        if (Job__Poll() == 0) { Stream__Control(g_Sound.streamCh[2], 1); Schedule__Kick(6); }
        else Sound__WaitCh2(2);
    }
    g_Sound.voiceFlag = 0;
    g_Sound.voiceId   = -1;
    g_Sound.seSub0 = -1; g_Sound.seSub1 = -1;
}

/* cmd 7/8 — SE bank (gp table) play once/loop */
undefined4 SeBank__Play(s32 id, s32 arg, int loop)
{
    int bank = -1;
    if (Job__Poll() != 0) { (loop ? Sound__WaitCh3 : Sound__WaitCh3)(id); return 1; }
    for (int i = 0; i < 1; i++) {   /* single-range table */
        if (g_SeBankTable[i][0] <= id && id <= g_SeBankTable[i][1]) {
            bank = i;
            g_Sound.seLocal = id - g_SeBankTable[i][0];
            g_Sound.seBank  = i;
            break;
        }
    }
    if (bank < 0) return 0;
    g_Sound.seArg     = arg;
    g_Sound.seFlag    = loop ? 4 : 1;
    g_Sound.seVolume  = 1.0f;
    g_Sound.seSub0    = 0;
    g_Sound.seId      = id;
    return 1;
}

/* cmd 9 — SE bank stop-all (full two-phase) */
void SeBank__StopAll(void)
{
    if (g_Sound.streamCh[3] != 0) {
        if (g_Sound.seFlag != 0 && g_Sound.streamCh[3] != 0) {
            if (Job__Poll() == 0) { Stream__Control(g_Sound.streamCh[3], 1); Schedule__Kick(6); }
            else Sound__WaitCh3(3);
        }
        if (g_Sound.streamCh[3] != 0) {
            if (Job__Poll() == 0) {
                Stream__CloseWait(g_Sound.streamCh[3], 0, 0);
                g_Sound.streamCh[3] = 0;
                Schedule__Kick(6);
            } else Sound__WaitCh3(3);
        }
        g_Sound.seBank = -1;
    }
    g_Sound.seFlag = 0;
    g_Sound.seId   = -1;
    g_Sound.seSub1 = -1;
    g_Sound.seSub2 = -1;
    g_Sound.seSub0 = 0;
    g_Sound.seVolume = 0;
}

/* cmd 10 — deferred callback job */
undefined4 Job__Start(s32 p1, s32 p2, s32 p3, s32 p4, s32 p5, s32 cb, s32 cbArg)
{
    extern void JobDesc__Set(s32, s32, s32);        /* @0x00141FA0 */
    extern void Task__QueuePending(s32,s32,s32,s32,s32,s32,s32); /* @0x00177960 */
    extern s32  g_CurrentJob;                       /* @0x007D1FC0 */
    extern u32  g_JobState;                         /* @0x007D20D8 */
    extern u32  g_JobFrames;                        /* @0x007D20DC */
    s32 job;

    if (Job__Poll() == 0 || Job__ForceFlag() != 0) {  /* @0x00177EC0 */
        JobDesc__Set(0x7d1fd4, p1, p2);
        job = Job__Create(0x7d1fd4, 0x11, 0x176df0, 0);
        g_CurrentJob = job;
        if (job != 0) {
            Job__CallVmethod(job, p3, 1);             /* vtable +0x38 @0x7D1FD4 area */
            g_JobCb            = (void *)cb;
            g_JobCbArg         = cbArg;
            *(s32 *)0x007D1FCC = p5;
            *(s32 *)0x007D1FD0 = p4;
            *(s32 *)0x007D20D4 = p3;
            g_JobState = 1;
            g_JobFrames = 0;
            Schedule__Kick(6);
        }
    } else {
        Task__QueuePending(p1, p2, p3, p4, p5, cb, cbArg);
    }
    return 1;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x00129B30, size 0x1B8
 * PURPOSE: memory move with overlap handling. Semantically equivalent to
 *          memmove(dst, src, n), returning dst. */
s32 fcn_00129B30(s32 dst, s32 src, s32 n)
{
    u8 *out = (u8 *)(u32)dst;
    const u8 *in = (const u8 *)(u32)src;

    if (n <= 0) {
        return dst;
    }

    if (src < dst && dst < src + n) {
        out += n;
        in += n;
        while (n-- > 0) {
            *--out = *--in;
        }
    } else {
        while (n-- > 0) {
            *out++ = *in++;
        }
    }
    return dst;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x00129CE8, size 0xC0
 * PURPOSE: byte fill. Semantically equivalent to memset(dst, value, n),
 *          returning dst. */
s32 fcn_00129CE8(s32 dst, s32 value, s32 n)
{
    u8 *out = (u8 *)(u32)dst;
    u8 fill = (u8)value;

    while (n-- > 0) {
        *out++ = fill;
    }
    return dst;
}

/* Intrusive 16-byte-aligned allocator used by the scheduler task queue.
 * Node layout:
 *   +0x00 value/tag
 *   +0x04 prev
 *   +0x08 next
 *   +0x0C payload pointer; zero marks a free node
 */
#define ALLOC__HEAD (*(volatile s32 *)0x005A38AC)
#define ALLOC__NODE(n) ((volatile s32 *)(u32)(n))
#define ALLOC__ALIGN16(x) (((x) + 15) & ~15)

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00177550, size 0xF8 */
s32 FUN_00177550(s32 size, s32 value)
{
    s32 node = ALLOC__HEAD;
    s32 payload;
    s32 split;

    while (node != 0) {
        if (ALLOC__NODE(node)[3] == 0) {
            payload = ALLOC__ALIGN16(node + 0x10);
            split = ALLOC__ALIGN16(payload + size);

            if (ALLOC__NODE(node)[2] >= split + 0x20) {
                s32 old_next = ALLOC__NODE(node)[2];

                ALLOC__NODE(node)[2] = split;

                ALLOC__NODE(split)[0] = 0;
                ALLOC__NODE(split)[1] = node;
                ALLOC__NODE(split)[2] = old_next;
                ALLOC__NODE(split)[3] = 0;

                ALLOC__NODE(node)[2] = split;
                ALLOC__NODE(node)[3] = payload;
                ALLOC__NODE(node)[0] = value;
                return payload;
            }

            if (ALLOC__NODE(node)[2] >= split) {
                ALLOC__NODE(node)[2] = payload;
                ALLOC__NODE(node)[3] = payload;
                ALLOC__NODE(node)[0] = value;
                return payload;
            }
        }
        node = ALLOC__NODE(node)[2];
    }

    node = 0x005A38AC;
    payload = ALLOC__ALIGN16(node + 0x10);
    split = ALLOC__ALIGN16(payload + size);

    ALLOC__NODE(node)[2] = payload;
    ALLOC__NODE(node)[3] = payload;
    ALLOC__NODE(node)[0] = value;

    ALLOC__NODE(split)[0] = 0;
    ALLOC__NODE(split)[1] = node;
    ALLOC__NODE(split)[2] = 0;
    ALLOC__NODE(split)[3] = 0;

    ALLOC__HEAD = split;
    return payload;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00177400, size 0x74 */
void FUN_00177400(s32 payload)
{
    s32 node = ALLOC__HEAD;

    while (node != 0) {
        s32 next;

        if (ALLOC__NODE(node)[3] == payload) {
            ALLOC__NODE(node)[3] = 0;
            ALLOC__HEAD = node;
            node = ALLOC__NODE(node)[1];
            continue;
        }

        next = ALLOC__NODE(node)[2];
        if (next != 0 &&
            ALLOC__NODE(next)[3] == 0 &&
            ALLOC__NODE(node)[3] == 0) {
            s32 next_next = ALLOC__NODE(next)[2];

            if (next_next != 0) {
                ALLOC__NODE(node)[2] = next_next;
                ALLOC__NODE(next_next)[1] = node;
            } else {
                ALLOC__HEAD = node;
            }
        }

        node = ALLOC__NODE(node)[1];
    }
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00177480, size 0x4C */
s32 FUN_00177480(s32 size, s32 value)
{
    s32 payload = FUN_00177550(size, value);

    fcn_00129CE8(payload, 0, size);
    return payload;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x001774D0, size 0x7C */
/* value is also passed to the copy helper as its length. */
s32 FUN_001774D0(s32 payload, s32 size, s32 value)
{
    s32 new_payload;

    if (size == 0) {
        if (payload == 0) {
            return 0;
        }
        FUN_00177400(payload);
        return 0;
    }

    new_payload = FUN_00177550(size, value);
    if (payload != 0) {
        fcn_00129B30(new_payload, payload, value);
        FUN_00177400(payload);
    }
    return new_payload;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x0012CA60, size 0x118
 * PURPOSE: optimized string copy. Semantically equivalent to strcpy(dst, src),
 *          returning dst. */
s32 fcn_0012CA60(s32 dst, s32 src)
{
    char *out = (char *)(u32)dst;
    const char *in = (const char *)(u32)src;

    while ((*out++ = *in++) != '\0') {
    }
    return dst;
}

/* Queue a 308-byte task slot with two payload words. */
static void Task__QueueSlot2(s32 arg1, s32 arg2, s32 cmd)
{
    s32 slot = (*(volatile s32 *)0x008E8604);
    s32 next = slot + 1;

    if (next < 64) {
        (*(volatile s32 *)0x008E8604) = next;
        (*(volatile s32 *)(u32)(0x008D3900 + next * 0x134 + 0x04)) = cmd;
        (*(volatile s32 *)(u32)(0x008D3900 + next * 0x134 + 0x110)) = arg1;
        (*(volatile s32 *)(u32)(0x008D3900 + next * 0x134 + 0x114)) = arg2;
    } else {
        (*(volatile s32 *)0x008D8604) = 0;
    }
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x001A7B18, size 0x40
 * PURPOSE: stage a stream-channel config payload. */
void Stream__StageConfig(s32 slot, s32 cfg)
{
    s32 flags = (*(volatile s32 *)(u32)slot);

    flags = (flags | 0x04) & ~0x08;
    (*(volatile s32 *)(u32)slot) = flags;
    (*(volatile s32 *)(u32)(slot + 0x08)) = cfg;
    (*(volatile s32 *)(u32)(slot + 0x10)) = 0x3F800000;
    (*(volatile s32 *)(u32)(slot + 0x04)) = 1;
    (*(volatile s32 *)(u32)(slot + 0x14)) = 0;
    (*(volatile s32 *)(u32)(slot + 0x0C)) = 0;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x001A7B58, size 0x18 */
void Stream__StageCmd(s32 slot, s32 env)
{
    s32 flags = (*(volatile s32 *)(u32)slot);

    (*(volatile s32 *)(u32)(slot + 0x0C)) = env;
    (*(volatile s32 *)(u32)slot) = flags | 0x08;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x001A7B70, size 0x18 */
void Stream__StageResume(s32 slot, s32 arg)
{
    s32 flags = (*(volatile s32 *)(u32)slot);

    (*(volatile s32 *)(u32)(slot + 0x14)) = arg;
    (*(volatile s32 *)(u32)slot) = flags | 0x40;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x001A7B88, size 0x18 */
void Stream__Notify(s32 slot)
{
    s32 flags = (*(volatile s32 *)(u32)slot);

    (*(volatile s32 *)(u32)(slot + 0x04)) = 1;
    (*(volatile s32 *)(u32)slot) = flags | 0x80;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x001A7BA0, size 0x30
 * NOTE: original receives float_bits in $f4/$t4; keep that ABI caveat in
 * mind before using this from custom-convention call sites. */
void Stream__StageParam(s32 slot, s32 src, s32 float_bits)
{
    s32 flags = (*(volatile s32 *)(u32)slot);

    if (((*(volatile s32 *)(u32)src) & 0x01) == 0) {
        (*(volatile s32 *)(u32)(slot + 0x10)) = float_bits;
        (*(volatile s32 *)(u32)slot) = flags | 0x20;
    }
}

/* Queue a 308-byte task slot. The producer cursor is 0x008E8604; slots start
 * at 0x008D3900 with stride 0x134. cmd is stored at slot+4, arg at slot+0x110. */
static void Task__QueueSlot(s32 arg, s32 cmd)
{
    s32 slot = (*(volatile s32 *)0x008E8604);
    s32 next = slot + 1;

    if (next < 64) {
        (*(volatile s32 *)0x008E8604) = next;
        (*(volatile s32 *)(u32)(0x008D3900 + next * 0x134 + 0x04)) = cmd;
        (*(volatile s32 *)(u32)(0x008D3900 + next * 0x134 + 0x110)) = arg;
    } else {
        (*(volatile s32 *)0x008D8604) = 0;
    }
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00177830, size 0x60 */
void FUN_00177830(s32 arg1, s32 arg2)
{
    Task__QueueSlot2(arg1, arg2, 14);
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00177890, size 0x70 */
void FUN_00177890(s32 arg1, s32 arg2)
{
    s32 slot = (*(volatile s32 *)0x008E8604);
    s32 next = slot + 1;

    if (next < 64) {
        s32 slot_addr;

        (*(volatile s32 *)0x008E8604) = next;
        slot_addr = 0x008D3900 + next * 0x134;
        (*(volatile s32 *)(u32)(slot_addr + 0x04)) = 13;
        (*(volatile s32 *)(u32)(slot_addr + 0x110)) = arg2;
        fcn_0012CA60(slot_addr + 0x08, arg1);
    } else {
        (*(volatile s32 *)0x008D8604) = 0;
    }
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00177900, size 0x58 */
void FUN_00177900(s32 arg)
{
    Task__QueueSlot(arg, 12);
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00177650, size 0x60 */
void FUN_00177650(s32 arg)
{
    Task__QueueSlot(arg, 19);
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x001776B0, size 0x60 */
void FUN_001776B0(s32 arg)
{
    Task__QueueSlot(arg, 18);
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00177710, size 0x60 */
void FUN_00177710(s32 arg)
{
    Task__QueueSlot(arg, 17);
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00177770, size 0x60 */
void FUN_00177770(s32 arg)
{
    Task__QueueSlot(arg, 16);
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x001777D0, size 0x60 */
void FUN_001777D0(s32 arg)
{
    Task__QueueSlot(arg, 15);
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00176DF0, size 0x58
 * PURPOSE: deferred-callback job handler for vmethod operation 5. */
void Job__Handler11(s32 job, s32 arg, s32 unused, s32 op)
{
    void (*callback)(s32, s32);

    (void)job;
    (void)unused;

    if (op != 5) {
        return;
    }

    callback = (void (*)(s32, s32))(u32)(*(volatile s32 *)0x007D1FC4);
    callback(arg, *(volatile s32 *)0x007D1FC8);

    (*(volatile s32 *)0x007D20D8) = 3;
    Schedule__Kick(6);
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x0012D018, size 0x1BC
 * PURPOSE: bounded string copy with zero padding. Semantically equivalent to
 *          strncpy(dst, src, n), returning dst. */
s32 fcn_0012D018(s32 dst, s32 src, s32 n)
{
    char *out = (char *)(u32)dst;
    const char *in = (const char *)(u32)src;
    s32 remaining = n;

    while (remaining > 0 && *in != '\0') {
        *out++ = *in++;
        remaining--;
    }
    while (remaining > 0) {
        *out++ = '\0';
        remaining--;
    }
    return dst;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00188A98, size 0x138
 * PURPOSE: resolve a job descriptor prefix ("name:...") to a registered job
 *          class. Returns the fallback class when no prefix match exists.
 *
 * Opaque structures:
 *   registration manager @0x008D91B8: strlen @+0xF4, compare @+0xF0
 *   job-class list head   @0x008DD298
 *   class node: next @+0x00, name @+0x0C
 */
s32 fcn_00188A98(s32 desc)
{
    char prefix[256];
    s32 manager = 0x008DD298;
    s32 length;
    s32 node;
    s32 i;

    if ((*(volatile s32 *)(u32)(manager + 0x08)) == 0) {
        s32 register_cb = (*(volatile s32 *)(u32)(manager + 0x14));

        if (register_cb != 0) {
            ((void (*)(s32))(u32)register_cb)(6);
        }
        return 0;
    }

    length = ((s32 (*)(void))
              (*(volatile s32 *)0x008D91B8 + 0xF4))();
    if (length == 0) {
        return (*(volatile s32 *)0x005A3494);
    }

    for (i = 0; i < length; i++) {
        if ((*(volatile char *)(u32)(desc + i)) == ':') {
            s32 prefix_len = i + 1;
            s32 cmp_result;

            fcn_0012D018((s32)prefix, desc, prefix_len);
            prefix[prefix_len] = '\0';

            node = (*(volatile s32 *)0x008DD298);
            while (node != 0) {
                s32 (*compare)(s32, s32);

                compare = *(s32 (**)(s32, s32))
                    (u32)((*(volatile s32 *)0x008D91B8) + 0xF0);
                cmp_result = compare((s32)prefix,
                                     (*(volatile s32 *)(u32)(node + 0x0C)));
                if (cmp_result == 0) {
                    return node;
                }
                node = (*(volatile s32 *)(u32)node);
            }
            return 0;
        }
    }

    return (*(volatile s32 *)0x005A3494);
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00189830, size 0x08
 * PURPOSE: return the deferred-job manager singleton. */
s32 fcn_00189830(void)
{
    return 0x008DD298;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00188910, size 0x188
 * PURPOSE: find the last reusable deferred-job slot for the requested job
 *          class and initialize it. Returns the job handle, or 0 on failure.
 *
 * Opaque layout used below:
 *   job class: active_count @ +0x04, get_slot @ +0x14, init @ +0x28
 *   job:       field_30 @ +0x30, field_34 @ +0x34, status @ +0x38,
 *              field_40 @ +0x40, handler @ +0x48, arg @ +0x4C
 */
s32 Job__Create(s32 desc, int type, s32 handler, int arg)
{
    s32 class_handle = fcn_00188A98(desc);
    s32 job = 0;
    s32 sema_id;
    s32 signal_result;
    s32 init_state;
    s32 init_result;
    s32 i;

    if (class_handle == 0) {
        return 0;
    }

    sema_id = (*(volatile s32 *)0x005A3908);
    WaitSema(sema_id);

    for (i = 0;
         i < (*(volatile s32 *)(u32)(class_handle + 0x04));
         i++) {
        s32 (*get_slot)(s32, s32);
        s32 candidate;

        get_slot = *(s32 (**)(s32, s32))(u32)(class_handle + 0x14);
        candidate = get_slot(class_handle, i);
        if ((*(volatile s32 *)(u32)(candidate + 0x38)) == 5) {
            job = candidate;
        }
    }

    signal_result = SignalSema(sema_id);

    if (job == 0) {
        if ((type & 0x10) != 0) {
            s32 manager = fcn_00189830();
            s32 create_cb = (*(volatile s32 *)0x008E2D2C);

            if (create_cb == 0) {
                return 0;
            }

            (*(volatile s32 *)(u32)(manager + 0x10)) = 8;
            ((void (*)(void))(u32)create_cb)();
        }
        return 0;
    }

    if (signal_result != 0) {
        init_state = 2;
    } else {
        init_state = (*(volatile s32 *)(u32)(job + 0x30));
    }

    if (init_state != 0) {
        (*(volatile s32 *)(u32)(job + 0x48)) = handler;
        (*(volatile s32 *)(u32)(job + 0x40)) = 2;
        (*(volatile s32 *)(u32)(job + 0x4C)) = arg;
    }

    if ((*(volatile s32 *)(u32)(class_handle + 0x10)) != 0) {
        (*(volatile s32 *)(u32)(job + 0x48)) = handler;
        (*(volatile s32 *)(u32)(job + 0x40)) = 2;
        (*(volatile s32 *)(u32)(job + 0x4C)) = arg;
        type |= 0x10;
    }

    init_result = ((s32 (*)(s32, s32, s32, s32))
                   (*(volatile s32 *)(u32)(class_handle + 0x28)))(
        class_handle, job, desc, type);

    if (init_result != 1) {
        (*(volatile s32 *)(u32)(job + 0x38)) = 1;
        job = 0;
    }

    if (job != 0) {
        (*(volatile s32 *)(u32)(job + 0x34)) = 2;
    }

    return job;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode)
 * ADDRESS: 0x00177EB0
 * PURPOSE: raise the deferred-job completion flag read by Job__ForceFlag. */
void Schedule__OnJobDone(void)
{
    (*(volatile s32 *)0x008E860C) = 1;
}

/* STATUS: RECONSTRUCTED (manual MIPS decode; Ghidra decompile comparison pending)
 * ADDRESS: 0x00177140
 * PURPOSE: advance the deferred-callback job state machine once per frame.
 *          The global context starts at 0x007D1FC0; the current job vtable is
 *          read through job+0x50. */
void Schedule__WalkSlots(void)
{
    Schedule__Job *job = (Schedule__Job *)(u32)g_CurrentJob;
    s32 local_out;
    s32 result;

    if (job == 0) {
        return;
    }

    g_JobFrames++;
    result = job->vtable->method_3C(job, 0);

    switch ((s32)g_JobState) {
    case 1:
        if (result != 2) {
            return;
        }
        if (Job__Poll() != 0) {
            return;
        }

        job->vtable->method_30(job,
                               *(s32 *)0x007D1FCC,
                               *(s32 *)0x007D1FD0);
        g_JobState = 2;
        Schedule__Kick(6);
        break;

    case 2:
        break;

    case 3:
        if (Job__Poll() != 0) {
            return;
        }

        job->vtable->method_2C(job);
        g_CurrentJob = 0;
        g_JobCb = 0;
        g_JobCbArg = 0;
        *(s32 *)0x007D1FCC = 0;
        *(s32 *)0x007D1FD0 = 0;
        g_JobState = 0;
        Schedule__Kick(6);
        Schedule__OnJobDone();
        break;

    case 4:
        if (g_JobFrames < 30) {
            return;
        }
        if (Job__Poll() != 0) {
            return;
        }

        job->vtable->method_2C(job);
        g_JobFrames = 0;
        g_JobState = 5;
        break;

    case 5:
        if (g_JobFrames < 30) {
            return;
        }
        if (Job__Poll() != 0) {
            return;
        }

        g_CurrentJob = Job__Create(0x007D1FD4,
                                   0x11,
                                   0x00176DF0,
                                   0);
        Schedule__Kick(6);

        job = (Schedule__Job *)(u32)g_CurrentJob;
        if (job == 0) {
            return;
        }

        job->vtable->method_38(job,
                               (s32)&local_out,
                               *(s32 *)0x007D20D4,
                               1);
        g_JobFrames = 0;
        g_JobState = 1;
        break;

    default:
        break;
    }
}

/* cmd 0C-0x13 — stream channel management (channel idx in arg) */
void Stream__OpenChannel(int ch)
{
    if (g_Sound.streamCh[ch] == 0) {
        if (Job__Poll() == 0) {
            g_Sound.streamCh[ch] = Stream__Open(g_Sound.rwaRoot, 0, 0, 0, 0);
            Schedule__Kick(6);
        } else Sound__DeferOpen(ch);                  /* @0x00177900 */
    }
}

void Stream__Configure(s32 slot, int ch)
{
    if (g_Sound.streamCh[ch] != 0 && Job__Poll() == 0) {
        struct { s32 a; s32 flag1; s32 z1; s32 z2; s32 f8; s32 z3; s32 f4; s32 z4; s32 one; } cfg = {0};
        cfg.a = slot; cfg.flag1 = 1; cfg.one = 1; cfg.f8 = 8; cfg.f4 = 4;
        Stream__SendConfig(g_Sound.streamCh[ch], &cfg);
        Schedule__Kick(6);
    }
}

void Stream__SetParam(s32 slot, int ch)
{
    if (g_Sound.streamCh[ch] != 0 && Job__Poll() == 0) {
        struct { s32 v; s32 t3; s32 z; s32 t2; } inner = { slot, 3, 0, 2 };
        struct { void *p; s32 count; } env = { &inner, 1 };
        Stream__SendCmd(g_Sound.streamCh[ch], &env);
        Schedule__Kick(6);
    }
}

void Stream__Pause(int ch)   { if (g_Sound.streamCh[ch] && Job__Poll() == 0) { Stream__Control(g_Sound.streamCh[ch], 3); Schedule__Kick(6); } }
void Stream__FadeStop(int ch){ if (g_Sound.streamCh[ch] && Job__Poll() == 0) { Sound__FadePrep(ch); Stream__Control(g_Sound.streamCh[ch], 2); Schedule__Kick(6); } } /* @0x16FB0 prep */
void Stream__StopCh(int ch)  { if (g_Sound.streamCh[ch] && Job__Poll() == 0) { Stream__Control(g_Sound.streamCh[ch], 1); Schedule__Kick(6); } }
void Stream__CloseCh(int ch) { if (g_Sound.streamCh[ch] && Job__Poll() == 0) { Stream__CloseWait(g_Sound.streamCh[ch], 0, 0); g_Sound.streamCh[ch] = 0; Schedule__Kick(6); } }
void Stream__ResumeCh(int ch){ if (g_Sound.streamCh[ch] && Job__Poll() == 0) { Stream__Resume(g_Sound.streamCh[ch], 1); Schedule__Kick(6); } }

/* ================= dispatcher ================= */

#define SOUND_CMD_QUEUE   ((volatile s32 *)0x008D3904)
#define SOUND_EXEC_SLOT   (*(volatile s32 *)0x008D8608)  /* consumer cursor */
#define SOUND_WRITE_SLOT  (*(volatile s32 *)0x008D8604)  /* producer cursor (requeue) */
#define SOUND_COUNTDOWN   (*(volatile s32 *)0x008D3900)
extern void Schedule__TickB(void);              /* @0x0017CCB0 — tail update */

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x00177BC0, size 0x60 */
void Sound__DeferBgmB(s32 arg1, s32 arg2)
{
    Task__QueueSlot2(arg1, arg2, 2);
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x00177C20, size 0x58 */
void Command__Requeue(s32 arg1, s32 arg2)
{
    Task__QueueSlot2(arg1, arg2, 1);
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: alias of Command__Requeue */
void Sound__DeferBgmA(s32 arg1, s32 arg2)
{
    Command__Requeue(arg1, arg2);
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x001A3060, size 0x20 */
void Stream__Resume(s32 handle, int unused)
{
    (void)unused;
    Stream__StageResume(handle + 0x2C0, 1);
}

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-19)
 * ADDRESS: 0x001A3180, size 0x20 */
void Stream__Control(s32 handle, int op)
{
    (void)handle;
    (void)op;
    Stream__Notify(handle + 0x2C0);
}

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x00177C80 — executes one queued command per tick. */
void Sound__ProcessCommand(void)
{
    int i = SOUND_EXEC_SLOT * 0x134;
    bool done = false;

    switch (*(volatile s32 *)(i + 0x8d3904)) {
    case 1:  Bgm__Play(*(s32 *)(i + 0x8d3a08), *(s32 *)(i + 0x8d3a0c), 0); done = true; break;
    case 2:  Bgm__Play(*(s32 *)(i + 0x8d3a08), *(s32 *)(i + 0x8d3a0c), 1); done = true; break;
    case 3:  Bgm__StopAll(); done = true; break;
    case 4:  Voice__Play(*(s32 *)(i + 0x8d3a30), 0); done = true; break;
    case 5:  Voice__Play(*(s32 *)(i + 0x8d3a30), 1); done = true; break;
    case 6:  Voice__Stop(); done = true; break;
    case 7:  SeBank__Play(*(s32 *)(i + 0x8d3a34), *(s32 *)(i + 0x8d3a0c), 0); done = true; break;
    case 8:  SeBank__Play(*(s32 *)(i + 0x8d3a34), *(s32 *)(i + 0x8d3a0c), 1); done = true; break;
    case 9:  SeBank__StopAll(); done = true; break;
    case 10: Job__Start(*(s32 *)(i + 0x8d3908), *(s32 *)(i + 0x8d3a18), *(s32 *)(i + 0x8d3a1c),
                        *(s32 *)(i + 0x8d3a20), *(s32 *)(i + 0x8d3a24), *(s32 *)(i + 0x8d3a28),
                        *(s32 *)(i + 0x8d3a2c)); done = true; break;
    case 0xC: Stream__OpenChannel(*(s32 *)(i + 0x8d3a10)); done = true; break;
    case 0xD: Stream__Configure(*(s32 *)(i + 0x8d3908), *(s32 *)(i + 0x8d3a10)); done = true; break;
    case 0xE: Stream__SetParam(*(s32 *)(i + 0x8d3a10), *(s32 *)(i + 0x8d3a14)); done = true; break;
    case 0xF: Stream__Pause(*(s32 *)(i + 0x8d3a10)); done = true; break;
    case 0x10: Stream__FadeStop(*(s32 *)(i + 0x8d3a10)); done = true; break;
    case 0x11: Stream__StopCh(*(s32 *)(i + 0x8d3a10)); done = true; break;
    case 0x12: Stream__CloseCh(*(s32 *)(i + 0x8d3a10)); done = true; break;
    case 0x13: Stream__ResumeCh(*(s32 *)(i + 0x8d3a10)); break;   /* no done-flag in raw */
    }
    if (done) {
        *(volatile s32 *)(i + 0x8d3904) = 0;
        SOUND_EXEC_SLOT++;
        if (SOUND_EXEC_SLOT > 0x3f) SOUND_EXEC_SLOT = 0;
    }
}

/* STATUS: RECONSTRUCTED  ADDRESS: 0x00177F30 — main-loop call. */
void Schedule__Update(void)
{
    if (SOUND_COUNTDOWN > 0) {
        SOUND_COUNTDOWN--;
        if (SOUND_COUNTDOWN < 1) SOUND_COUNTDOWN = 0;
    }
    Schedule__TickA();
    Job__PollAll();
    Schedule__WalkSlots();                        /* @0x00177140 */
    if (SOUND_COUNTDOWN == 0 &&
        *(volatile s32 *)(SOUND_EXEC_SLOT * 0x134 + 0x8d3904) != 0) {
        Sound__ProcessCommand();
    }
    Schedule__TickB();
}

/* STATUS: RECONSTRUCTED  ADDRESS: 0x00177F00 — main-loop call. */
void Schedule__FrameTick(void)
{
    Sound__UpdateEnd();
    (*(volatile s32 *)0x008D8610)++;
}

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
extern void Command__Requeue(s32 cmd, s32 a, s32 b); /* @0x00177C20: write next free queue slot */
extern void Sound__DeferBgmA(s32, s32);         /* @0x00177C20 = Command__Requeue(1,a,b) */
extern void Sound__DeferBgmB(s32, s32);         /* @0x00177BC0 */
extern void Sound__WaitCh0(int);                /* @0x00177710 */
extern void Sound__WaitCh1(int);                /* @0x001776B0 */
extern void Sound__WaitCh2(s32);                /* @0x00177710 variant ch2  */
extern void Sound__WaitCh3(s32);                /* @0x001776B0 variant ch3  */
extern s32  Stream__Open(s32 rwa, int,int,int,int); /* @0x001A2E00 */
extern void Stream__Control(s32 h, int op);     /* @0x001A3180: 1=stop 2=fade 3=pause */
extern void Stream__CloseWait(s32 h, int, int); /* @0x0018F600 */
extern void Stream__SendConfig(s32 h, void *cfg0x24); /* @0x001A3080 */
extern void Stream__SendCmd(s32 h, void *env);  /* @0x001A2AE8 */
extern void Stream__Resume(s32 h, int);         /* @0x001A3060 */
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
extern void Job__Handler11(s32, s32, s32, s32); /* @0x00176DF0: op5 fires cb */
extern s32  WaitSema(s32);                      /* @0x0010E960 */
extern s32  SignalSema(s32);                    /* @0x0010E940 */
extern void fcn_00177320(void);                 /* @0x00177320 — name pending */
extern void fcn_0017CCF0(void);                 /* @0x0017CCF0 — name pending */
extern void fcn_00192AA0(s32, s32, s32, s32, s32, s32, s32, s32); /* @0x00192AA0 */
extern s32  fcn_00192D50(void);                 /* @0x00192D50 — name pending */
extern s32  fcn_0019F788(void);                 /* @0x0019F788 — name pending */
extern void fcn_001A39E8(void);                 /* @0x001A39E8 — name pending */
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

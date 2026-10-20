/*
 * PS2-NANA-decomp — custom EE call helpers for $t0-$t3 arguments.
 * STATUS: PARTIAL / TARGET-VERIFICATION PENDING
 *
 * Some original functions pass extra arguments in $t0-$t3 instead of the
 * normal o32 stack area. Standard C prototypes do not express that ABI.
 *
 * This header provides:
 *   - a MIPS/GCC inline-asm path when building for the EE;
 *   - a portable fallback that preserves the logical argument order for
 *     host-side syntax checks and documentation.
 *
 * Verify the inline-asm path with the real ps2dev toolchain before using it
 * for matching builds.
 */
#ifndef EE_CUSTOM_ABI_H
#define EE_CUSTOM_ABI_H

#include "ps2.h"

#if defined(__GNUC__) && defined(__mips__)
#define EE_CUSTOM_ABI_GNU_MIPS 1

static inline s32 ee_call5(s32 fn, s32 a0, s32 a1, s32 a2, s32 a3, s32 t0)
{
    register s32 r4 __asm__("$4") = a0;
    register s32 r5 __asm__("$5") = a1;
    register s32 r6 __asm__("$6") = a2;
    register s32 r7 __asm__("$7") = a3;
    register s32 r8 __asm__("$8") = t0;
    register s32 ret __asm__("$2");

    __asm__ volatile(
        "jalr %6"
        : "=&r"(ret)
        : "r"(fn), "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r8)
        : "$31", "memory");
    return ret;
}

static inline s32 ee_call6(s32 fn, s32 a0, s32 a1, s32 a2, s32 a3, s32 t0, s32 t1)
{
    register s32 r4 __asm__("$4") = a0;
    register s32 r5 __asm__("$5") = a1;
    register s32 r6 __asm__("$6") = a2;
    register s32 r7 __asm__("$7") = a3;
    register s32 r8 __asm__("$8") = t0;
    register s32 r9 __asm__("$9") = t1;
    register s32 ret __asm__("$2");

    __asm__ volatile(
        "jalr %7"
        : "=&r"(ret)
        : "r"(fn), "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r8), "r"(r9)
        : "$31", "memory");
    return ret;
}

static inline s32 ee_call8(s32 fn,
                           s32 a0,
                           s32 a1,
                           s32 a2,
                           s32 a3,
                           s32 t0,
                           s32 t1,
                           s32 t2,
                           s32 t3)
{
    register s32 r4 __asm__("$4") = a0;
    register s32 r5 __asm__("$5") = a1;
    register s32 r6 __asm__("$6") = a2;
    register s32 r7 __asm__("$7") = a3;
    register s32 r8 __asm__("$8") = t0;
    register s32 r9 __asm__("$9") = t1;
    register s32 r10 __asm__("$10") = t2;
    register s32 r11 __asm__("$11") = t3;
    register s32 ret __asm__("$2");

    __asm__ volatile(
        "jalr %9"
        : "=&r"(ret)
        : "r"(fn), "r"(r4), "r"(r5), "r"(r6), "r"(r7),
          "r"(r8), "r"(r9), "r"(r10), "r"(r11)
        : "$31", "memory");
    return ret;
}

#define EE_CALL5(fn, a0, a1, a2, a3, t0) \
    ee_call5((s32)(fn), (a0), (a1), (a2), (a3), (t0))
#define EE_CALL6(fn, a0, a1, a2, a3, t0, t1) \
    ee_call6((s32)(fn), (a0), (a1), (a2), (a3), (t0), (t1))
#define EE_CALL8(fn, a0, a1, a2, a3, t0, t1, t2, t3) \
    ee_call8((s32)(fn), (a0), (a1), (a2), (a3), (t0), (t1), (t2), (t3))
#define EE_CALL5_VOID(fn, a0, a1, a2, a3, t0) \
    ((void)EE_CALL5((fn), (a0), (a1), (a2), (a3), (t0)))
#define EE_CALL6_VOID(fn, a0, a1, a2, a3, t0, t1) \
    ((void)EE_CALL6((fn), (a0), (a1), (a2), (a3), (t0), (t1)))
#define EE_CALL8_VOID(fn, a0, a1, a2, a3, t0, t1, t2, t3) \
    ((void)EE_CALL8((fn), (a0), (a1), (a2), (a3), (t0), (t1), (t2), (t3)))

#else

#define EE_CALL5(fn, a0, a1, a2, a3, t0) ((fn)((a0), (a1), (a2), (a3), (t0)))
#define EE_CALL6(fn, a0, a1, a2, a3, t0, t1) ((fn)((a0), (a1), (a2), (a3), (t0), (t1)))
#define EE_CALL8(fn, a0, a1, a2, a3, t0, t1, t2, t3) \
    ((fn)((a0), (a1), (a2), (a3), (t0), (t1), (t2), (t3)))
#define EE_CALL5_VOID(fn, a0, a1, a2, a3, t0) ((void)((fn)((a0), (a1), (a2), (a3), (t0))))
#define EE_CALL6_VOID(fn, a0, a1, a2, a3, t0, t1) \
    ((void)((fn)((a0), (a1), (a2), (a3), (t0), (t1))))
#define EE_CALL8_VOID(fn, a0, a1, a2, a3, t0, t1, t2, t3) \
    ((void)((fn)((a0), (a1), (a2), (a3), (t0), (t1), (t2), (t3))))

#endif

#endif /* EE_CUSTOM_ABI_H */
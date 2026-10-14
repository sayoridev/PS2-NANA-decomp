/*
 * PS2-NANA-decomp — EE startup (crt0 chain)
 * STATUS: RECONSTRUCTED
 * ORIGINAL: SLPM_659.14 @ 0x00100080, Metrowerks CodeWarrior 2.4.1.01
 *
 * Verified against rizin disassembly (see notes/analysis/startup.md).
 * Unknowns are marked inline. Boot chain:
 *   Startup__Entry -> syscall 0x3C (kernel heap/stack) -> syscall 0x3D
 *   -> System__BootInit1 -> syscall 0x64(0) -> Game__Main [infinite loop]
 */
#include "../common/ps2.h"

extern void System__BootInit1(void);   /* @0x00118C98 */
extern s32  System__Syscall64(s32);    /* @0x0010EBA0 — kernel syscall 0x64 wrapper */
extern void Game__Main(void);          /* @0x00141EE0 — contains the main loop */
extern void System__BootTail(void);    /* @0x00127DE0 — tail target, likely exit path */

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x001000F0  SIZE: 0x1C
 * PURPOSE: zeroes FPU registers $f24..$f31. Also called from fcn.0044E1F0
 *          (probable reset path). Why only $f24..$f31: UNKNOWN (ABI guess:
 *          upper half preserved across the call convention used there). */
void Fpu__ClearAll(void)
{
    __asm__ volatile(
        "mtc1 $zero, $f24\n\t"
        "mtc1 $zero, $f25\n\t"
        "mtc1 $zero, $f26\n\t"
        "mtc1 $zero, $f27\n\t"
        "mtc1 $zero, $f28\n\t"
        "mtc1 $zero, $f29\n\t"
        "mtc1 $zero, $f30\n\t"
        "mtc1 $zero, $f31\n\t"
        ::: "memory");
}

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x00100080
 * PURPOSE: crt0 — FPU init, BSS clear, $gp setup, kernel heap/stack
 *          syscalls, hand-off to the boot chain. */
void Startup__Entry(void)
{
    /* 1. Zero all 32 FPU registers and FPU control word (inline in original;
     *    Fpu__ClearAll above is the separately-callable $f24..$f31 tail). */
    __asm__ volatile(
        "mtc1 $zero, $f0\n\t"
        "mtc1 $zero, $f1\n\t"
        "mtc1 $zero, $f2\n\t"
        "mtc1 $zero, $f3\n\t"
        "mtc1 $zero, $f4\n\t"
        "mtc1 $zero, $f5\n\t"
        "mtc1 $zero, $f6\n\t"
        "mtc1 $zero, $f7\n\t"
        "mtc1 $zero, $f8\n\t"
        "mtc1 $zero, $f9\n\t"
        "mtc1 $zero, $f10\n\t"
        "mtc1 $zero, $f11\n\t"
        "mtc1 $zero, $f12\n\t"
        "mtc1 $zero, $f13\n\t"
        "mtc1 $zero, $f14\n\t"
        "mtc1 $zero, $f15\n\t"
        "mtc1 $zero, $f16\n\t"
        "mtc1 $zero, $f17\n\t"
        "mtc1 $zero, $f18\n\t"
        "mtc1 $zero, $f19\n\t"
        "mtc1 $zero, $f20\n\t"
        "mtc1 $zero, $f21\n\t"
        "mtc1 $zero, $f22\n\t"
        "mtc1 $zero, $f23\n\t"
        "sync 0x10\n\t"
        "ctc1 $zero, $31\n\t"
        ::: "memory");

    /* 2. Clear BSS [0x5A3680, 0x1A7900). Original: align-to-16 loop, then
     *    byte loop for the tail. (Some decodes in that range, e.g.
     *    "ext zero, v0, 0, 1", are suspicious rizin artifacts — verify raw
     *    bytes in Ghidra before promoting status.) */
    {
        u8 *p = (u8 *)PS2_MAINLOOP_BSS_START;
        u8 *end = (u8 *)PS2_MAINLOOP_BSS_END;

        while (((uintptr_t)p & 15u) != 0 && p < end) {
            *p++ = 0;
        }
        while (p + 16 <= end) {
            *(u32 *)(p +  0) = 0;  /* original: 128-bit stores; semantic match */
            *(u32 *)(p +  4) = 0;
            *(u32 *)(p +  8) = 0;
            *(u32 *)(p + 12) = 0;
            p += 16;
        }
        while (p < end) {
            *p++ = 0;
        }
    }

    /* 3. Set $gp (0x005AB270 — verified identical to .reginfo value), then
     *    kernel heap/stack setup via syscall 0x3C; v0 becomes $sp.
     *    Semantics of the 0x3D follow-up call: UNKNOWN. */
    __asm__ volatile(
        "li   $a0, 0x005AB270\n\t"    /* $gp value */
        "move $gp, $a0\n\t"
        "li   $a1, 0x001EB000\n\t"    /* heap bound 1 */
        "li   $a2, 0x10000000\n\t"    /* heap bound 2 */
        "li   $a3, 0x005A3A80\n\t"    /* heap end / boot param slot */
        "li   $t0, 0x00010220\n\t"
        "li   $v1, 0x3C\n\t"
        "syscall\n\t"
        "move $sp, $v0\n\t"
        "li   $a0, 0x001A7900\n\t"
        "li   $a1, -1\n\t"
        "li   $v1, 0x3D\n\t"
        "syscall\n\t"
        ::: "a0", "a1", "a2", "a3", "t0", "v0", "v1", "gp", "sp", "memory");

    /* 4. Boot chain. Game__Main never returns (infinite loop), so
     *    System__BootTail is reached only via the original tail jump. */
    System__BootInit1();
    (void)System__Syscall64(0);
    Game__Main();
    System__BootTail();
}

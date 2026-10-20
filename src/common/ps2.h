/*
 * PS2-NANA-decomp — common PS2 EE types and low-level macros.
 * STATUS: RECONSTRUCTED
 * Targets the Emotion Engine (R5900, MIPS-III + MMI), little endian.
 */
#ifndef PS2_H
#define PS2_H

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef u32        uint;          /* Ghidra uint type */
typedef u64        undefined8;   /* Ghidra 64-bit unknown type */
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef float    f32;
typedef double   f64;

typedef u32 vaddr_t;            /* EE virtual address */

/* Memory map constants recovered in Phase 1/2 (see docs/executable.md) */
#define PS2_USER_RAM_BASE       0x00100000u
#define PS2_GP_VALUE            0x005AB270u     /* verified in crt0 + .reginfo */
#define PS2_OVL_LOAD_BASE       0x00B7F800u     /* MWo3 overlay load address */
#define PS2_MAINLOOP_BSS_START  0x001A7900u     /* crt0 BSS clear range */
#define PS2_MAINLOOP_BSS_END    0x005A3680u

/* EE kernel syscall IDs observed in SLPM_659.14 (semantics partly unknown) */
#define PS2_SYSCALL_HEAP_SETUP  0x3Cu   /* crt0: args gp/heap bounds; returns sp in v0 */
#define PS2_SYSCALL_SETUP2      0x3Du   /* crt0: args a0=0x1A7900, a1=-1 */
#define PS2_SYSCALL_ADD_HANDLER 0x56u   /* handler registration (kernel-side install) */

#endif /* PS2_H */

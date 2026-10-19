/*
 * PS2-NANA-decomp — observed EE kernel syscall wrappers.
 * STATUS: PARTIAL / STRUCTURAL
 *
 * The original executable uses Sony EE kernel syscall IDs through small
 * wrapper bodies. Only names verified from the analysis database or source
 * evidence are marked as known; all other entries are address/ID pairs.
 */
#ifndef PS2_SYSCALLS_H
#define PS2_SYSCALLS_H

#include "ps2.h"

/* crt0 and boot-critical IDs documented in src/common/ps2.h. */
#define PS2_SYSCALL_FLUSH_CACHE   0x64u
#define PS2_SYSCALL_UNKNOWN_66    0x66u
#define PS2_SYSCALL_UNKNOWN_67NEG 0xFFFFFF99u /* -103 */
#define PS2_SYSCALL_UNKNOWN_68NEG 0xFFFFFF98u /* -104 */
#define PS2_SYSCALL_UNKNOWN_6A_NEG 0xFFFFFF96u /* -106 */
#define PS2_SYSCALL_UNKNOWN_6B    0x6Bu
#define PS2_SYSCALL_UNKNOWN_6C    0x6Cu
#define PS2_SYSCALL_UNKNOWN_6D    0x6Du
#define PS2_SYSCALL_UNKNOWN_6E    0x6Eu
#define PS2_SYSCALL_UNKNOWN_6F    0x6Fu
#define PS2_SYSCALL_UNKNOWN_70    0x70u
#define PS2_SYSCALL_UNKNOWN_70NEG 0xFFFFFF90u /* -112 */
#define PS2_SYSCALL_UNKNOWN_71    0x71u
#define PS2_SYSCALL_UNKNOWN_71NEG 0xFFFFFF8Fu /* -113 */
#define PS2_SYSCALL_UNKNOWN_72    0x72u
#define PS2_SYSCALL_UNKNOWN_73    0x73u
#define PS2_SYSCALL_UNKNOWN_74    0x74u
#define PS2_SYSCALL_UNKNOWN_75    0x75u
#define PS2_SYSCALL_UNKNOWN_76    0x76u
#define PS2_SYSCALL_UNKNOWN_76NEG 0xFFFFFF8Au /* -118 */

/* Wrapper addresses recovered from the stub table at 0x0010EBA0. */
#define PS2_WRAPPER_FLUSH_CACHE    ((vaddr_t)0x0010EBA0u) /* syscall 0x64 */
#define PS2_WRAPPER_SYSCALL_66     ((vaddr_t)0x0010EBB0u)
#define PS2_WRAPPER_SYSCALL_67NEG  ((vaddr_t)0x0010EBC0u)
#define PS2_WRAPPER_SYSCALL_68NEG  ((vaddr_t)0x0010EBD0u)
#define PS2_WRAPPER_SYSCALL_6A_NEG ((vaddr_t)0x0010EBE0u)
#define PS2_WRAPPER_SYSCALL_6B     ((vaddr_t)0x0010EBF0u)
#define PS2_WRAPPER_SYSCALL_6C     ((vaddr_t)0x0010EC00u)
#define PS2_WRAPPER_SYSCALL_6D     ((vaddr_t)0x0010EC10u)
#define PS2_WRAPPER_SYSCALL_6E     ((vaddr_t)0x0010EC20u)
#define PS2_WRAPPER_SYSCALL_6F     ((vaddr_t)0x0010EC30u)
#define PS2_WRAPPER_SYSCALL_70     ((vaddr_t)0x0010EC40u)
#define PS2_WRAPPER_SYSCALL_70NEG  ((vaddr_t)0x0010EC50u)
#define PS2_WRAPPER_SYSCALL_71     ((vaddr_t)0x0010EC60u)
#define PS2_WRAPPER_SYSCALL_71NEG  ((vaddr_t)0x0010EC70u)
#define PS2_WRAPPER_SYSCALL_72     ((vaddr_t)0x0010EC80u)
#define PS2_WRAPPER_SYSCALL_73     ((vaddr_t)0x0010EC90u)
#define PS2_WRAPPER_SYSCALL_74     ((vaddr_t)0x0010ECA0u)
#define PS2_WRAPPER_SYSCALL_75     ((vaddr_t)0x0010ECB0u)
#define PS2_WRAPPER_SYSCALL_76     ((vaddr_t)0x0010ECC0u)
#define PS2_WRAPPER_SYSCALL_76NEG  ((vaddr_t)0x0010ECD0u)

/* Named wrappers verified during scheduler reconstruction. */
extern s32 WaitSema(s32 sema_id);   /* wrapper @0x0010E960 */
extern s32 SignalSema(s32 sema_id); /* wrapper @0x0010E940 */

#endif /* PS2_SYSCALLS_H */

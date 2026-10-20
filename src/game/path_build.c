/*
 * PS2-NANA-decomp — media path builder
 * STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-20)
 * ORIGINAL: SLPM_659.14 @ 0x00141FA0, size 0x36C
 *
 * Builds a full device path in `dst` from a relative `src`:
 *   mode 1 -> "host0:../img/<src>"   (then '\' -> '/' after ':')
 *   mode 2 -> "cdrom0:<src>;1"       (then '/' -> '\' after ':')
 *   mode 4 -> "hfs:../img/<src>"     (then '\' -> '/' after ':')
 *   mode 5 -> "dvd:<src>;1"          (then '/' -> '\' after ':')
 * Entry modes are canonicalized first: {0,6}->2, {3,7,8,9}->5.
 * Returns the length of the built string (s0 - dst in the original).
 *
 * NOTE: symbols/functions.txt comment labels this address "Config__Parse
 * (dst, 0x301C00, 6); retry loop" — that description does NOT match the
 * decoded body (no 0x301C00 buffers, no retry loop). Kept as FUN_ name;
 * the stale comment needs reclassification.
 *
 * Evidence: 13x String__Format @0x0012C2D0, 4x fcn_0012C788 (strchr-family,
 * ':' search), 2x fcn_0012E7C8 (strlen, result discarded by the compiler).
 * The two strlen calls are dead in practice but preserved verbatim.
 */
#include "../common/ps2.h"

extern s32 String__Format(char *dst, const char *fmt, ...); /* @0x0012C2D0 */
extern char *fcn_0012c788(const char *s, s32 c);            /* @0x0012C788 — strchr */
extern s32 fcn_0012e7c8(const char *s);                     /* @0x0012E7C8 — strlen */

/* STATUS: RECONSTRUCTED (manual MIPS decode, 2026-10-20)
 * ADDRESS: 0x00141FA0, size 0x36C */
s32 FUN_00141FA0(char *dst, const char *src, s32 mode)
{
    char *p;
    char *s0 = dst;         /* write cursor; returned length = s0 - dst */

    switch (mode) {         /* canonicalization, verbatim from entry chain */
    case 9:
    case 3:
    case 8:
    case 7:
        mode = 5;
        break;
    case 0:
    case 6:
        mode = 2;
        break;
    default:
        break;
    }

    switch (mode) {
    case 1:                 /* host0:../img/<src> */
        s0 += String__Format(dst, "host0:../img");
        if (src[0] != '/') {
            if (src[0] != '\\') {
                s0 += String__Format(s0, "/");
            }
        }
        s0 += String__Format(s0, src);
        p = fcn_0012c788(dst, ':');
        if (p[1] == '\0') {
            break;
        }
        for (s0 = p + 1; *s0 != '\0'; s0++) {
            if (*s0 == '\\') {
                *s0 = '/';
            }
        }
        break;

    case 2:                 /* cdrom0:<src>;1 */
        s0 += String__Format(dst, "cdrom0:");
        if (src[0] != '/') {
            if (src[0] != '\\') {
                s0 += String__Format(s0, "/");
            }
        }
        s0 += String__Format(s0, src);
        s0 += String__Format(s0, ";1");
        p = fcn_0012c788(dst, ':');
        s0 = p + 1;
        (void)fcn_0012e7c8(s0);     /* result discarded in the original */
        if (*s0 == '\0') {
            break;
        }
        for (; *s0 != '\0'; s0++) {
            if (*s0 == '/') {
                *s0 = '\\';
            }
        }
        break;

    case 4:                 /* hfs:../img/<src> */
        s0 += String__Format(dst, "hfs:../img");
        if (src[0] != '/') {
            if (src[0] != '\\') {
                s0 += String__Format(s0, "/");
            }
        }
        s0 += String__Format(s0, src);
        p = fcn_0012c788(dst, ':');
        if (p[1] == '\0') {
            break;
        }
        for (s0 = p + 1; *s0 != '\0'; s0++) {
            if (*s0 == '\\') {
                *s0 = '/';
            }
        }
        break;

    case 5:                 /* dvd:<src>;1 */
        s0 += String__Format(dst, "dvd:");
        if (src[0] != '/') {
            if (src[0] != '\\') {
                s0 += String__Format(s0, "/");
            }
        }
        s0 += String__Format(s0, src);
        s0 += String__Format(s0, ";1");
        p = fcn_0012c788(dst, ':');
        s0 = p + 1;
        (void)fcn_0012e7c8(s0);     /* result discarded in the original */
        if (*s0 == '\0') {
            break;
        }
        for (; *s0 != '\0'; s0++) {
            if (*s0 == '/') {
                *s0 = '\\';
            }
        }
        break;

    default:
        break;
    }

    return (s32)(s0 - dst);
}
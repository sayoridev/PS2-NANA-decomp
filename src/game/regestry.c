/*
 * PS2-NANA-decomp — asset & overlay registry boot preload
 * STATUS: RECONSTRUCTED (decompile-verified, Ghidra R5900)
 * ORIGINAL: SLPM_659.14 @ 0x00142660 — called once from Game__Init
 *
 * Iterates the Format A table @ 0x001C9AA0 (12-byte entries:
 * {flag, name_ptr, nkp_ptr}; terminator name==0). flag==0 = NKP embedded in
 * the ELF image; flag!=0 = NKP on disc, loaded lazily by name at boot
 * ("dvd:%s" path built via String__Format) with the result cached in the
 * entry. Format B (executable overlays @ 0x001C9B00) is NOT touched here —
 * overlays load on demand. Registers Registry__OnFrame into the per-frame
 * callback chain (previous handler kept at g_PrevFrameCallback).
 */
#include "../common/ps2.h"

typedef struct {
    s32 flag;        /* 0 = embedded NKP, !=0 = disc NKP */
    s32 name;        /* asset name (terminator when 0 in table walk) */
    s32 data;        /* embedded NKP vaddr, or disc-loaded result */
} Asset__Entry;

extern s32  System__RegisterFrameCallback(void *handler);  /* @0x00101FA0 */
extern void Registry__OnFrame(void *);                     /* @0x001423E0 */
extern void Asset__LoadEmbedded(s32 name, s32 nkpPtr);     /* @0x00139A90 */
extern s32  Asset__LoadFromDisc(char *path, s32);          /* @0x00139BA0 */
extern void String__Format(char *dst, const char *fmt, ...);/* @0x0012C2D0 */

extern Asset__Entry g_AssetTable[];      /* @0x001C9AA0 */
extern void (*g_PrevFrameCallback)(void);/* @0x005A3778 */

extern u32 g_RegistryState[];            /* @0x005A3768..0x005A3778 */

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x00142660
 * PURPOSE: preload all Format A assets; chain into frame callbacks. */
void AssetOverlay__Registry(void)
{
    char path[128];

    g_RegistryState[1] = 0;
    g_RegistryState[0] = 0;
    g_PrevFrameCallback = (void *)System__RegisterFrameCallback(Registry__OnFrame);
    g_RegistryState[2] = 0;
    g_RegistryState[3] = 0;

    for (Asset__Entry *e = g_AssetTable; e->name != 0; e++) {
        if (e->flag == 0) {
            Asset__LoadEmbedded(e->name, e->data);
        } else {
            String__Format(path, "%s", (char *)e->name);
            e->data = Asset__LoadFromDisc(path, 0);
        }
    }
}

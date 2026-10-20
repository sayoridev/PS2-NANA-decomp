/*
 * PS2-NANA-decomp — master game init
 * STATUS: RECONSTRUCTED (decompile-verified, Ghidra R5900)
 * ORIGINAL: SLPM_659.14 @ 0x00141D90 — called once from Game__Main
 *
 * Boot chain (verified end-to-end):
 *   Startup__Entry -> syscall 0x3C/0x3D -> System__BootInit1 ->
 *   System__EarlyInit (config parse, RNG seed) -> Game__Init -> Game__Main
 */
#include "../common/ps2.h"

extern void System__EarlyInit(void);           /* @0x00141B50 */
extern void Memory__InitRegions(int, u32, u32, u32, int); /* @0x00134DD0 heap 0x1EC0000-0x1F60000 */
extern void Schedule__Init(void);              /* @0x00177FD0 */
extern s64  Boot__SystemCheck(int *flag);      /* @0x00176C80: ret>=0 && *flag -> trap+halt */
extern void Draw__BootInit(void);              /* @0x0013B340 */
extern void fcn_00139d60(void);                /* @0x00139D60 — init TBD */
extern void Mode__Init(void);                  /* @0x0013C5D0 */
extern void Display__SetMode(f32, f32, f32, f32, f32);    /* @0x00134B60 (512,512,448,256,224) */
extern void Display__SetAspect(f32, f32);      /* @0x00134B20 (1.0, 1.12) */
extern void Display__SetDepthRange(f32, f32);  /* @0x00134AE0 (1.1, 10000) */
extern void fcn_00132170(int, int, int);       /* @0x00132170 — init TBD */
extern void fcn_00101150(int, int);            /* @0x00101150 — display sibling (14 xrefs) */
extern void AssetOverlay__Registry(void);      /* @0x00142660 */
extern void Msg__Init(void);                   /* @0x001507B0 */
extern void Scene__Init(void);                 /* @0x00147110 */
extern void Stream__Init(void);                /* @0x001B96B0 */
extern void Module__LoadRequest(int);          /* @0x00142630 — cmd 1 = load Title overlay */
extern void Display__EndFrame(void);           /* @0x00134D50 */

/* STATUS: RECONSTRUCTED
 * ADDRESS: 0x00141D90
 * PURPOSE: master init; halts via trap(0) if Boot__SystemCheck fails. */
void Game__Init(void)
{
    int  flag = 0;
    s64  check;

    System__EarlyInit();
    Memory__InitRegions(0, 0x1ec0000, 0x1f60000, 0xa0000, 0);
    Schedule__Init();
    check = Boot__SystemCheck(&flag);
    Draw__BootInit();
    fcn_00139d60();
    Mode__Init();
    Display__SetMode(512.0f, 512.0f, 448.0f, 256.0f, 224.0f);
    Display__SetAspect(1.0f, 1.12f);
    Display__SetDepthRange(1.1f, 10000.0f);
    fcn_00132170(0, 0, 0);
    fcn_00101150(0, 0);
    AssetOverlay__Registry();
    Msg__Init();
    Scene__Init();
    Stream__Init();
    Module__LoadRequest(1);      /* first overlay: Title */
    Display__EndFrame();
    if (check >= 0 && flag != 0) {
        __asm__ volatile("trap");          /* boot check failed: intentional halt */
        for (;;) { }
    }
}
/*
 * PS2-NANA-decomp — RAW Ghidra decompile: Game__Init (0x00141D90)
 * STATUS: RECONSTRUCTED-RAW (verbatim Ghidra R5900 output)
 * ORIGINAL: SLPM_659.14 @ 0x00141D90 — called once from Game__Main
 *
 * Master boot init. Ends with Boot__SystemCheck guard: on failure the game
 * executes trap(0) and halts. Loads first overlay (Title) via
 * Module__LoadRequest(1) at the end of init.
 * Boot chain (fully verified): Startup__Entry -> syscall 0x3C/0x3D ->
 * System__BootInit1 -> System__EarlyInit (config, RNG seed) -> Game__Init ->
 * Game__Main [infinite loop].
 */
void Game__Init(void)

{
  long lVar1;
  int iStack_4;

  FUN_00141b50();
  FUN_00134dd0(0,0x1ec0000,0x1f60000,0xa0000,0);
  FUN_00177fd0();
  lVar1 = FUN_00176c80(&iStack_4);
  FUN_0013b340();
  FUN_00139d60();
  FUN_0013c5d0();
  FUN_00134b60(0x44000000,0x44000000,0x43e00000,0x43800000,0x43600000);
  FUN_00134b20(0x3f800000,0x3f8f5c29);
  FUN_00134ae0(0x3f8ccccd,0x461c4000);
  FUN_00132170(0,0,0);
  FUN_00101150(0,0);
  AssetOverlay__Registry();
  FUN_001507b0();
  FUN_00147110();
  FUN_001b96b0();
  Module__LoadRequest(1);
  FUN_00134d50();
  if ((-1 < lVar1) && (iStack_4 != 0)) {
    trap(0);
    do {
                    /* WARNING: Do nothing block with infinite loop */
    } while( true );
  }
  return;
}

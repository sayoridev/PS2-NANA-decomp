/*
 * PS2-NANA-decomp — RAW Ghidra decompile (0x00142660)
 * STATUS: RECONSTRUCTED-RAW (verbatim Ghidra R5900 output)
 * Boot-time preload of Format A asset table (embedded vs disc NKP).
 */
void AssetOverlay__Registry(void)

{
  int iVar1;
  int *piVar2;
  undefined1 auStack_80 [128];

  uRam005a3774 = 0;
  uRam005a3770 = 0;
  uRam005a3778 = FUN_00101fa0(0x1423e0);
  uRam005a3768 = 0;
  uRam005a376c = 0;
  for (piVar2 = &DAT_001c9aa0; piVar2[1] != 0; piVar2 = piVar2 + 3) {
    if (*piVar2 == 0) {
      FUN_00139a90(piVar2[1],piVar2[2]);
    }
    else {
      FUN_0012c2d0(auStack_80,0x301d88);
      iVar1 = FUN_00139ba0(auStack_80,0);
      piVar2[2] = iVar1;
    }
  }
  return;
}

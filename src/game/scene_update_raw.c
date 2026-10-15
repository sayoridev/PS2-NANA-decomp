/*
 * PS2-NANA-decomp — RAW Ghidra decompile: Scene__Update (0x00146040)
 * STATUS: RECONSTRUCTED-RAW (verbatim Ghidra R5900 output, pending cleanup)
 * ORIGINAL: SLPM_659.14 @ 0x00146040 — main-loop call #12
 *
 * Scene transition controller: fade-in (2 phases) / hold / fade-out (2 phases)
 * with per-item audio-volume ramp (floor -0x80, slope 0x80/n) and DMA packet
 * scheduling into g_DmaPacketBuf. State 7 calls Module__LoadRequest(1) —
 * overlay loading is triggered mid-transition.
 * Analysis: notes/analysis/scene_update_doc.md.
 */
void Scene__Update(void)

{
  bool bVar1;
  int iVar2;
  undefined8 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  
  iVar9 = 0x5bb880;
  if (iRam005bb884 != 0) {
    iVar4 = 0;
    iVar8 = iVar9;
    if (0 < *(int *)(&DAT_001c9e20 + iRam005bb880 * 4)) {
      do {
        iVar4 = iVar4 + 1;
        *(int *)(iVar8 + 0xbc) = *(int *)(iVar8 + 0xbc) + 1;
        iVar8 = iVar8 + 4;
      } while (iVar4 < *(int *)(&DAT_001c9e20 + iRam005bb880 * 4));
    }
    iRam005bb960 = iRam005bb960 + -1;
    if (iRam005bb960 < 1) {
      iRam005bb960 = 0;
    }
    if (iRam005bb960 < 1) {
      iRam005bb968 = iRam005bb968 + 1;
    }
    switch(iRam005bb884) {
    case 1:
      iVar8 = 0;
      if (0 < *(int *)(&DAT_001c9e20 + iRam005bb880 * 4)) {
        iVar5 = 0;
        iVar4 = iVar9;
        do {
          if (*(int *)(&DAT_001c9e48 + iVar5 + iRam005bb880 * 0x84) <= *(int *)(iVar4 + 0xbc)) {
            iRam005bb884 = 2;
            *(undefined4 *)(iVar4 + 0xbc) =
                 *(undefined4 *)(&DAT_001c9e48 + iVar5 + iRam005bb880 * 0x84);
          }
          *(undefined4 *)(iVar4 + 200) = 0;
          iVar8 = iVar8 + 1;
          iVar5 = iVar5 + 0x2c;
          iVar4 = iVar4 + 4;
        } while (iVar8 < *(int *)(&DAT_001c9e20 + iRam005bb880 * 4));
      }
      if (*(int *)(&DAT_001ca164 + iRam005bb964 * 0x10) <= iRam005bb968) {
        iRam005bb968 = *(int *)(&DAT_001ca164 + iRam005bb964 * 0x10);
      }
      break;
    case 2:
    case 3:
      iRam005bb884 = 3;
      iVar8 = 0;
      iVar4 = 0;
      if (0 < *(int *)(&DAT_001c9e20 + iRam005bb880 * 4)) {
        iVar2 = 0;
        iVar5 = iVar9;
        do {
          if (*(int *)(&DAT_001c9e50 + iVar2 + iRam005bb880 * 0x84) <= *(int *)(iVar5 + 0xbc)) {
            *(int *)(iVar5 + 0xbc) = *(int *)(&DAT_001c9e50 + iVar2 + iRam005bb880 * 0x84);
            iVar8 = iVar8 + 1;
          }
          iVar4 = iVar4 + 1;
          iVar2 = iVar2 + 0x2c;
          iVar5 = iVar5 + 4;
        } while (iVar4 < *(int *)(&DAT_001c9e20 + iRam005bb880 * 4));
      }
      iVar4 = *(int *)(&DAT_001ca164 + iRam005bb964 * 0x10);
      bVar1 = iVar4 <= iRam005bb968;
      if (bVar1) {
        iRam005bb968 = iVar4;
      }
      if ((*(int *)(&DAT_001c9e20 + iRam005bb880 * 4) <= iVar8) && (bVar1)) {
        iRam005bb884 = 4;
      }
      break;
    case 4:
    case 5:
    case 6:
      iRam005bb884 = 5;
      iVar4 = 0;
      iVar8 = *(int *)(&DAT_001c9e20 + iRam005bb880 * 4);
      iVar5 = 0;
      if (0 < iVar8) {
        iVar7 = 0;
        iVar2 = iVar9;
        do {
          iVar8 = iVar7 + iRam005bb880 * 0x84;
          if (*(int *)(&DAT_001c9e40 + iVar8 + 0x18) <= *(int *)(iVar2 + 0xbc)) {
            iVar4 = iVar4 + 1;
            *(undefined4 *)(iVar2 + 0xbc) = *(undefined4 *)(&DAT_001c9e40 + iVar8 + 0x14);
          }
          iVar5 = iVar5 + 1;
          iVar7 = iVar7 + 0x2c;
          iVar8 = *(int *)(&DAT_001c9e20 + iRam005bb880 * 4);
          iVar2 = iVar2 + 4;
        } while (iVar5 < iVar8);
      }
      if (iVar8 <= iVar4) {
        iRam005bb884 = 6;
      }
      iRam005bb968 = *(int *)(&DAT_001ca164 + iRam005bb964 * 0x10);
      break;
    case 7:
      FUN_00142630(1);
      iRam005bb884 = 7;
      iVar4 = 0;
      iVar8 = *(int *)(&DAT_001c9e20 + iRam005bb880 * 4);
      iVar5 = 0;
      if (0 < iVar8) {
        iVar6 = 0;
        iVar7 = 0;
        iVar2 = iVar9;
        do {
          if (*(int *)(iVar6 + iRam005bb880 * 0xc + 0x5bb830) == 0) {
            if (*(int *)(&DAT_001c9e60 + iVar7 + iRam005bb880 * 0x84) <= *(int *)(iVar2 + 0xbc)) {
              *(int *)(iVar2 + 0xbc) = *(int *)(&DAT_001c9e60 + iVar7 + iRam005bb880 * 0x84);
              iVar4 = iVar4 + 1;
            }
          }
          else {
            iVar8 = iVar7 + iRam005bb880 * 0x84;
            if (*(int *)(&DAT_001c9e40 + iVar8 + 0x18) <= *(int *)(iVar2 + 0xbc)) {
              *(undefined4 *)(iVar2 + 0xbc) = *(undefined4 *)(&DAT_001c9e40 + iVar8 + 0x14);
            }
            if (*(int *)(&DAT_001c9e60 + iVar7 + iRam005bb880 * 0x84) < 1) {
              *(undefined4 *)(iVar2 + 200) = 0xffffff80;
            }
            else {
              *(int *)(iVar2 + 200) =
                   *(int *)(iVar2 + 200) -
                   0x80 / *(int *)(&DAT_001c9e60 + iVar7 + iRam005bb880 * 0x84);
              if (*(int *)(iVar2 + 200) < -0x80) {
                *(undefined4 *)(iVar2 + 200) = 0xffffff80;
              }
            }
            *(int *)(iVar2 + 0xd4) = *(int *)(iVar2 + 0xd4) + 1;
            if (*(int *)(&DAT_001c9e60 + iVar7 + iRam005bb880 * 0x84) <= *(int *)(iVar2 + 0xd4)) {
              iVar4 = iVar4 + 1;
            }
          }
          iVar5 = iVar5 + 1;
          iVar6 = iVar6 + 4;
          iVar7 = iVar7 + 0x2c;
          iVar8 = *(int *)(&DAT_001c9e20 + iRam005bb880 * 4);
          iVar2 = iVar2 + 4;
        } while (iVar5 < iVar8);
      }
      if (iVar8 <= iVar4) {
        iRam005bb884 = 8;
      }
      if (*(int *)(&DAT_001ca16c + iRam005bb964 * 0x10) <= iRam005bb968) {
        iRam005bb968 = *(int *)(&DAT_001ca16c + iRam005bb964 * 0x10);
      }
      break;
    case 8:
    case 9:
      iRam005bb884 = 9;
      iVar8 = 0;
      iVar4 = 0;
      if (0 < *(int *)(&DAT_001c9e20 + iRam005bb880 * 4)) {
        iVar7 = 0;
        iVar2 = 0;
        iVar5 = iVar9;
        do {
          if (*(int *)(iVar7 + iRam005bb880 * 0xc + 0x5bb830) == 0) {
            if (*(int *)(&DAT_001c9e68 + iVar2 + iRam005bb880 * 0x84) <= *(int *)(iVar5 + 0xbc)) {
              *(int *)(iVar5 + 0xbc) = *(int *)(&DAT_001c9e68 + iVar2 + iRam005bb880 * 0x84);
              iVar8 = iVar8 + 1;
            }
          }
          else {
            iVar6 = iVar2 + iRam005bb880 * 0x84;
            if (*(int *)(&DAT_001c9e40 + iVar6 + 0x18) <= *(int *)(iVar5 + 0xbc)) {
              *(undefined4 *)(iVar5 + 0xbc) = *(undefined4 *)(&DAT_001c9e40 + iVar6 + 0x14);
            }
            if (*(int *)(&DAT_001c9e68 + iVar2 + iRam005bb880 * 0x84) < 1) {
              *(undefined4 *)(iVar5 + 200) = 0xffffff80;
            }
            else {
              *(int *)(iVar5 + 200) =
                   *(int *)(iVar5 + 200) -
                   0x80 / *(int *)(&DAT_001c9e68 + iVar2 + iRam005bb880 * 0x84);
              if (*(int *)(iVar5 + 200) < -0x80) {
                *(undefined4 *)(iVar5 + 200) = 0xffffff80;
              }
            }
            *(int *)(iVar5 + 0xd4) = *(int *)(iVar5 + 0xd4) + 1;
            if (*(int *)(&DAT_001c9e68 + iVar2 + iRam005bb880 * 0x84) <= *(int *)(iVar5 + 0xd4)) {
              iVar8 = iVar8 + 1;
            }
          }
          iVar4 = iVar4 + 1;
          iVar7 = iVar7 + 4;
          iVar2 = iVar2 + 0x2c;
          iVar5 = iVar5 + 4;
        } while (iVar4 < *(int *)(&DAT_001c9e20 + iRam005bb880 * 4));
      }
      iVar4 = *(int *)(&DAT_001ca16c + iRam005bb964 * 0x10);
      bVar1 = iVar4 <= iRam005bb968;
      if (bVar1) {
        iRam005bb968 = iVar4;
      }
      if ((*(int *)(&DAT_001c9e20 + iRam005bb880 * 4) <= iVar8) && (bVar1)) {
        iRam005bb888 = 0;
        iRam005bb884 = 10;
      }
      break;
    case 10:
      iVar8 = 0;
      if (0 < *(int *)(&DAT_001c9e20 + iRam005bb880 * 4)) {
        iVar5 = 0;
        iVar4 = iVar9;
        do {
          iVar8 = iVar8 + 1;
          iVar2 = iVar5 + iRam005bb880 * 0x84;
          iVar5 = iVar5 + 0x2c;
          *(undefined4 *)(iVar4 + 0xbc) = *(undefined4 *)(&DAT_001c9e68 + iVar2);
          iVar4 = iVar4 + 4;
        } while (iVar8 < *(int *)(&DAT_001c9e20 + iRam005bb880 * 4));
      }
      iRam005bb884 = 0;
      iRam005bb968 = *(int *)(&DAT_001ca16c + iRam005bb964 * 0x10);
    case 0:
      iRam005bb888 = 0;
    }
    if ((iRam005bb97c == 0) && (uVar3 = FUN_001347f0(8), iRam005bb888 == 1)) {
      iVar8 = 0;
      if (0 < *(int *)(&DAT_001c9e20 + iRam005bb880 * 4)) {
        do {
          uRam005bb908 = 0;
          uRam005bb8b4 = 4;
          uRam005bb90c = 0;
          uRam005bb910 = 0;
          uRam005bb914 = *(undefined4 *)(iVar9 + 200);
          uRam005bb904 = 0x100;
          uRam005bb8f8 = 0x100;
          uRam005bb8fc = 0x100;
          uRam005bb900 = 0x100;
          FUN_00139470(*(undefined4 *)(iVar9 + 0x18),*(int *)(iVar9 + 0xbc) + -1);
          uRam005bb8a8 = *(undefined4 *)(iRam005a3680 + 0x48);
          uVar3 = FUN_00138700(uVar3,*(undefined4 *)(iVar9 + 0x18),0x5bb8a8);
          iVar8 = iVar8 + 1;
          iVar9 = iVar9 + 4;
        } while (iVar8 < *(int *)(&DAT_001c9e20 + iRam005bb880 * 4));
      }
      if ((iRam005bb970 != 0) && (iRam005bb960 < 1)) {
        uRam005bb8b4 = 0;
        FUN_00139470(iRam005bb970,iRam005bb968 + -1);
        FUN_00138700(uVar3,iRam005bb970,0x5bb8a8);
      }
    }
    FUN_00134790(8);
  }
  return;
}

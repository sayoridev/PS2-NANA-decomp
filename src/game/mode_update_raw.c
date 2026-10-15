/*
 * PS2-NANA-decomp — RAW Ghidra decompile: FUN_0013bde0 (= Mode__Update)
 * STATUS: RECONSTRUCTED-RAW (verbatim Ghidra R5900 output, pending cleanup)
 * ORIGINAL: SLPM_659.14 @ 0x0013BDE0 — called from Game__MainLoop
 *
 * This is the first full decompile with the R5900 language. It is the
 * per-frame pad input processor + command state machine + 2-channel ramp
 * engine for the two mode state blocks (0x5B2F00, stride 0x180).
 * Field map and analysis: notes/analysis/mode_update_doc.md.
 * DO NOT treat raw names (FUN_*, iVar*, uGp*) as final — cleanup pass
 * after the 0x11BXXX pad driver family is decompiled and named.
 */
void FUN_0013bde0(void)

{
  ushort uVar1;
  int iVar2;
  undefined1 uVar3;
  int iVar4;
  long lVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  
  iVar9 = 0;
  iVar8 = 0x5b2f00;
  do {
    FUN_00129ce8(iVar8 + 0x144,0,8);
    FUN_00129ce8(iVar8 + 0x13e,0,6);
    *(undefined2 *)(iVar8 + 300) = 0;
    *(undefined2 *)(iVar8 + 0x168) = 0;
    *(undefined2 *)(iVar8 + 0x16a) = 0;
    *(undefined2 *)(iVar8 + 0x16c) = 0;
    uVar1 = *(ushort *)(iVar8 + 0x164);
    *(undefined2 *)(iVar8 + 0x164) = 0;
    uVar3 = FUN_0011b500(iVar9,0);
    *(undefined1 *)(iVar8 + 0x12e) = uVar3;
    if (*(char *)(iVar8 + 0x12e) == '\0') {
      *(undefined4 *)(iVar8 + 0x134) = 0;
      *(undefined4 *)(iVar8 + 0x130) = 0;
    }
    iVar4 = *(int *)(iVar8 + 0x130);
    if (iVar4 == 0x4d) {
      lVar5 = FUN_0011b668(iVar9,0);
      if (lVar5 == 1) {
        *(int *)(iVar8 + 0x130) = *(int *)(iVar8 + 0x130) + -1;
      }
      lVar5 = FUN_0011b668(iVar9,0);
      if (lVar5 == 0) {
        *(undefined4 *)(iVar8 + 0x130) = 99;
      }
    }
    else if (iVar4 == 0x4c) {
      lVar5 = FUN_0011be78(iVar9,0);
      if (lVar5 == 1) {
        *(int *)(iVar8 + 0x130) = *(int *)(iVar8 + 0x130) + 1;
      }
    }
    else if (iVar4 == 0x48) {
      lVar5 = FUN_0011be18(iVar9,0);
      if (lVar5 == 1) {
        *(undefined4 *)(iVar8 + 0x130) = 0x4c;
      }
      else {
        *(undefined4 *)(iVar8 + 0x130) = 99;
      }
    }
    else if (iVar4 == 0x47) {
LAB_0013c024:
      lVar5 = FUN_0011b668(iVar9,0);
      if (lVar5 == 1) {
        *(int *)(iVar8 + 0x130) = *(int *)(iVar8 + 0x130) + -1;
      }
      lVar5 = FUN_0011b668(iVar9,0);
      if (lVar5 == 0) {
        *(int *)(iVar8 + 0x130) = *(int *)(iVar8 + 0x130) + 1;
      }
    }
    else if (iVar4 == 0x46) {
      FUN_00129ce8(iVar8 + 0x138,0xff,6);
      *(undefined1 *)(iVar8 + 0x138) = 0;
      *(undefined1 *)(iVar8 + 0x139) = 1;
      lVar5 = FUN_0011b668(iVar9,0);
      if (lVar5 != 2) {
        lVar5 = FUN_0011bbe0(iVar9,0,iVar8 + 0x138);
        if (lVar5 == 1) {
          FUN_00129ce8(iVar8 + 0x138,0,6);
          *(int *)(iVar8 + 0x130) = *(int *)(iVar8 + 0x130) + 1;
          goto LAB_0013c024;
        }
      }
    }
    else if (iVar4 == 0x2a) {
      lVar5 = FUN_0011b668(iVar9,0);
      if (lVar5 == 1) {
        *(int *)(iVar8 + 0x130) = *(int *)(iVar8 + 0x130) + -1;
      }
      lVar5 = FUN_0011b668(iVar9,0);
      if (lVar5 == 0) {
        *(undefined4 *)(iVar8 + 0x130) = 0;
      }
    }
    else if (iVar4 == 0x29) {
LAB_0013bf5c:
      lVar5 = FUN_0011ba60(iVar9,0,1,3);
      if (lVar5 == 1) {
        *(int *)(iVar8 + 0x130) = *(int *)(iVar8 + 0x130) + 1;
      }
    }
    else if (iVar4 == 0x28) {
      lVar5 = FUN_0011b928(iVar9,0,4,0xffffffffffffffff);
      if (lVar5 != 0) {
        *(int *)(iVar8 + 0x130) = *(int *)(iVar8 + 0x130) + 1;
        goto LAB_0013bf5c;
      }
      *(undefined4 *)(iVar8 + 0x130) = 99;
    }
    else if (iVar4 == 0) {
      if ((*(char *)(iVar8 + 0x12e) == '\x06') || (*(char *)(iVar8 + 0x12e) == '\x02')) {
        uVar3 = FUN_0011b928(iVar9,0,1,0);
        *(undefined1 *)(iVar8 + 0x12f) = uVar3;
        if (*(char *)(iVar8 + 0x12f) != '\0') {
          lVar5 = FUN_0011b928(iVar9,0,2,0);
          if (0 < lVar5) {
            *(char *)(iVar8 + 0x12f) = (char)lVar5;
          }
          if (*(char *)(iVar8 + 0x12f) == '\a') {
            *(undefined4 *)(iVar8 + 0x130) = 0x46;
          }
          else if (*(char *)(iVar8 + 0x12f) == '\x04') {
            *(undefined4 *)(iVar8 + 0x130) = 0x28;
          }
          else {
            *(undefined4 *)(iVar8 + 0x130) = 99;
          }
        }
      }
    }
    else {
      if (*(char *)(iVar8 + 0x12e) == '\a') {
        *(int *)(iVar8 + 0x134) = *(int *)(iVar8 + 0x134) + 1;
      }
      if (((*(char *)(iVar8 + 0x12e) == '\x06') || (*(char *)(iVar8 + 0x12e) == '\x02')) &&
         (lVar5 = FUN_0011b488(iVar9,0,iVar8 + 0x144), lVar5 != 0)) {
        if (*(char *)(iVar8 + 0x12f) == '\a') {
          FUN_00129ce8(iVar8 + 0x13e,0,6);
          iVar4 = *(int *)(iVar8 + 0x108);
          iVar2 = *(int *)(iVar8 + 0x104);
          if (iVar2 < iVar4) {
            if (*(int *)(iVar8 + 0x100) == 0) {
              iVar7 = *(int *)(iVar8 + 0x10c);
              iVar6 = iVar4 >> 1;
              if (iVar7 == 3) {
                iVar7 = iVar6;
                if (iVar4 < 0) {
                  iVar7 = iVar4 + 1 >> 1;
                }
                if (iVar2 < iVar7) {
                  *(char *)(iVar8 + 0x13e) =
                       (char)*(ushort *)(iVar8 + 0x114) +
                       (char)((int)(iVar2 * ((uint)*(ushort *)(iVar8 + 0x112) -
                                            (uint)*(ushort *)(iVar8 + 0x114))) / iVar7);
                }
                else {
                  if (iVar4 < 0) {
                    iVar6 = iVar4 + 1 >> 1;
                  }
                  *(byte *)(iVar8 + 0x13e) =
                       (char)*(ushort *)(iVar8 + 0x112) +
                       (char)((int)(((uint)*(ushort *)(iVar8 + 0x114) -
                                    (uint)*(ushort *)(iVar8 + 0x112)) * (iVar2 - iVar6)) / iVar6) |
                       1;
                }
                goto LAB_0013c3c0;
              }
              if (iVar7 == 2) {
                *(byte *)(iVar8 + 0x13e) =
                     (char)*(ushort *)(iVar8 + 0x114) +
                     (char)((int)(iVar2 * ((uint)*(ushort *)(iVar8 + 0x112) -
                                          (uint)*(ushort *)(iVar8 + 0x114))) / iVar4) | 1;
                goto LAB_0013c3c0;
              }
              if (iVar7 == 1) {
                *(byte *)(iVar8 + 0x13e) = (byte)*(undefined2 *)(iVar8 + 0x112) | 1;
                goto LAB_0013c3c0;
              }
              iVar4 = *(int *)(iVar8 + 0x104);
            }
            else {
LAB_0013c3c0:
              iVar4 = *(int *)(iVar8 + 0x104);
            }
            *(int *)(iVar8 + 0x104) = iVar4 + 1;
          }
          else {
            *(undefined4 *)(iVar8 + 0x10c) = 0;
          }
          iVar4 = *(int *)(iVar8 + 0x11c);
          iVar2 = *(int *)(iVar8 + 0x118);
          if (iVar2 < iVar4) {
            if (*(int *)(iVar8 + 0x100) == 0) {
              iVar7 = *(int *)(iVar8 + 0x120);
              iVar6 = iVar4 >> 1;
              if (iVar7 == 3) {
                iVar7 = iVar6;
                if (iVar4 < 0) {
                  iVar7 = iVar4 + 1 >> 1;
                }
                if (iVar2 < iVar7) {
                  *(char *)(iVar8 + 0x13f) =
                       (char)*(ushort *)(iVar8 + 0x128) +
                       (char)((int)(iVar2 * ((uint)*(ushort *)(iVar8 + 0x126) -
                                            (uint)*(ushort *)(iVar8 + 0x128))) / iVar7);
                }
                else {
                  if (iVar4 < 0) {
                    iVar6 = iVar4 + 1 >> 1;
                  }
                  *(char *)(iVar8 + 0x13f) =
                       (char)*(ushort *)(iVar8 + 0x126) +
                       (char)((int)(((uint)*(ushort *)(iVar8 + 0x128) -
                                    (uint)*(ushort *)(iVar8 + 0x126)) * (iVar2 - iVar6)) / iVar6);
                }
                goto LAB_0013c4d0;
              }
              if (iVar7 == 2) {
                *(char *)(iVar8 + 0x13f) =
                     (char)*(ushort *)(iVar8 + 0x128) +
                     (char)((int)(iVar2 * ((uint)*(ushort *)(iVar8 + 0x126) -
                                          (uint)*(ushort *)(iVar8 + 0x128))) / iVar4);
                goto LAB_0013c4d0;
              }
              if (iVar7 == 1) {
                *(char *)(iVar8 + 0x13f) = (char)*(undefined2 *)(iVar8 + 0x126);
                goto LAB_0013c4d0;
              }
              iVar4 = *(int *)(iVar8 + 0x118);
            }
            else {
LAB_0013c4d0:
              iVar4 = *(int *)(iVar8 + 0x118);
            }
            *(int *)(iVar8 + 0x118) = iVar4 + 1;
          }
          else {
            *(undefined4 *)(iVar8 + 0x120) = 0;
          }
          FUN_0011bb18(iVar9,0,iVar8 + 0x13e);
          uVar3 = *(undefined1 *)(iVar8 + 0x146);
        }
        else {
          *(undefined4 *)(iVar8 + 0x104) = 0;
          *(undefined4 *)(iVar8 + 0x108) = 0;
          *(undefined4 *)(iVar8 + 0x10c) = 0;
          *(undefined2 *)(iVar8 + 0x110) = 0;
          *(undefined2 *)(iVar8 + 0x112) = 0;
          *(undefined2 *)(iVar8 + 0x114) = 0;
          *(undefined4 *)(iVar8 + 0x118) = 0;
          *(undefined4 *)(iVar8 + 0x11c) = 0;
          *(undefined4 *)(iVar8 + 0x120) = 0;
          *(undefined2 *)(iVar8 + 0x124) = 0;
          *(undefined2 *)(iVar8 + 0x126) = 0;
          *(undefined2 *)(iVar8 + 0x128) = 0;
          uVar3 = *(undefined1 *)(iVar8 + 0x146);
        }
        *(ushort *)(iVar8 + 300) = CONCAT11(uVar3,*(undefined1 *)(iVar8 + 0x147)) ^ 0xffff;
        *(undefined1 *)(iVar8 + 299) = *(undefined1 *)(iVar8 + 0x145);
        if ((*(char *)(iVar8 + 0x12a) != '\0') &&
           (*(char *)(iVar8 + 299) != *(char *)(iVar8 + 0x12a))) {
          *(undefined1 *)(iVar8 + 0x12a) = 0;
          *(undefined4 *)(iVar8 + 0x130) = 0;
        }
        *(undefined1 *)(iVar8 + 0x12a) = *(undefined1 *)(iVar8 + 299);
        if (((int)(uint)*(byte *)(iVar8 + 299) >> 4 == 7) &&
           ((*(ushort *)(iVar8 + 300) & 0xf000) == 0)) {
          if (*(byte *)(iVar8 + 0x14a) < 0x50) {
            *(ushort *)(iVar8 + 300) = *(ushort *)(iVar8 + 300) | 0x8000;
          }
          else if (0xb0 < *(byte *)(iVar8 + 0x14a)) {
            *(ushort *)(iVar8 + 300) = *(ushort *)(iVar8 + 300) | 0x2000;
          }
          if (*(byte *)(iVar8 + 0x14b) < 0x50) {
            *(ushort *)(iVar8 + 300) = *(ushort *)(iVar8 + 300) | 0x1000;
          }
          else if (0xb0 < *(byte *)(iVar8 + 0x14b)) {
            *(ushort *)(iVar8 + 300) = *(ushort *)(iVar8 + 300) | 0x4000;
          }
        }
        *(ushort *)(iVar8 + 0x168) = *(ushort *)(iVar8 + 300) & (uVar1 ^ *(ushort *)(iVar8 + 300));
        *(ushort *)(iVar8 + 0x16c) = (uVar1 ^ *(ushort *)(iVar8 + 300)) & ~*(ushort *)(iVar8 + 300);
        *(undefined2 *)(iVar8 + 0x16a) = *(undefined2 *)(iVar8 + 0x168);
        if (*(short *)(iVar8 + 0x168) == 0) {
          *(short *)(iVar8 + 0x166) = *(short *)(iVar8 + 0x166) + 1;
          if ((uGpffff843c < *(ushort *)(iVar8 + 0x166)) &&
             ((uint)*(ushort *)(iVar8 + 0x166) % (uint)uGpffff8438 == 0)) {
            *(undefined2 *)(iVar8 + 0x16a) = *(undefined2 *)(iVar8 + 300);
          }
        }
        else {
          *(undefined2 *)(iVar8 + 0x166) = 0;
        }
        *(undefined2 *)(iVar8 + 0x164) = *(undefined2 *)(iVar8 + 300);
      }
    }
    iVar9 = iVar9 + 1;
    iVar8 = iVar8 + 0x180;
    if (1 < iVar9) {
      return;
    }
  } while( true );
}

/*
 * PS2-NANA-decomp — RAW Ghidra decompile (0x00177F30)
 * STATUS: RECONSTRUCTED-RAW (verbatim Ghidra R5900 output)
 * Task scheduler tick: countdown, slot walk, task execute.
 */
void Schedule__Update(void)

{
  if ((0 < iRam008d3900) && (iRam008d3900 = iRam008d3900 + -1, iRam008d3900 < 1)) {
    iRam008d3900 = 0;
  }
  FUN_00192bc8();
  FUN_00189468();
  FUN_00177140();
  if ((iRam008d3900 == 0) && (*(int *)(iRam008d8608 * 0x134 + 0x8d3904) != 0)) {
    FUN_00177c80();
  }
  FUN_0017ccb0();
  return;
}

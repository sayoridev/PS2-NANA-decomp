# Analysis — EE Startup & System Interface

_Date: 2026-10-14. Analyst: assisted pass, pending Ghidra confirmation.
All addresses in EE virtual space (main ELF `SLPM_659.14`, entry 0x00100008)._

## 1. Entry region (0x100000–0x10007F): installed handler table

- 30 LE pointers into 0x7000xxxx (stride 0x800), terminator 0x11.
- ELF e_entry (0x100008) lands INSIDE this table — those words are data.
- Writers: `System__InstallHandlers` (0x118478) via `System__InstallHandlerEntry`
(0x118398), IRQs masked during install (`mtc0 zero, a2` = clear COP0 Status IE).
- Source descriptor table: `g_SystemHandlerTable` @ **0x1C5078**
— entries of 0x10 bytes `{ptr, a2, a3, t0}`, count cap **49 (0x31)**.
- Open: where do 0x7000xxxx targets come from (kernel-side handler copy?
precomputed at link time?); confirm with `pd @ fcn.00118398`.

## 2. crt0 (Startup__Entry @ 0x100080)

```javascript
0x100080  (FPU init preamble)
0x100090..0x10010c  mtc1 zero, f0..f31   ; zero all FPU regs
0x100114  sync 0x10
0x100118  ctc1 zero, 31                 ; FPU control = 0
0x10011c..0x100128  v0=0x5A3680, v1=0x1A7900   ; BSS range
0x10012c..0x100148  align-16 + sb zero loop     ; BSS clear
```

- The `mtc1 f24..f31` tail (0x1000f0–0x10010c) is a separate callable unit,
`Fpu__ClearAll` — also CALLED from fcn.0044e1f0 (+0x15bc0), suggesting a
system reset path that reinitializes FPU state.

## 3. Syscall/data region (0x555070)

- rizin auto-labels `syscall.0.555070`; decode there is data-like
(`scd`/`sdc1` absurdities). Likely a module/dispatch table embedding
`syscall` opcodes (0x0000000c) as data. CONFIRM in Ghidra before naming.

## 4. Next steps

- [ ] Disassemble `System__InstallHandlerEntry` (0x118398) — confirm write target.
- [ ] Dump `g_SystemHandlerTable` (0x1C5078, 49×0x10) — entry semantics.
- [ ] Continue crt0 past BSS clear (from 0x10014c) to find `main` and the
call into the game loop.
- [ ] Identify what service the installed handlers provide (SIFRPC glue?
thread scheduler? exception vectors?).

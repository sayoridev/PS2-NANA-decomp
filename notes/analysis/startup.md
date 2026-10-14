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
- RESOLVED (2026-10-14): `System__AddHandler` (0x118398) is a raw **kernel
syscall 0x56 wrapper** — installation is done BY THE KERNEL per entry.
Dispatcher @0x1183A8 validates IDs in [0x0D, 0x30).
- Handler table 0x1C5078 raw dump: header words {0x0D, 0x12, 0x08}, pointers
to 0x1C4E08/0x1C4ED8/0x1C4FF8, then {id, handler} pairs ({0x83, 0x118B00},
{0x5A, 0x118AC8}); code stubs embedded from 0x1C50B8 (mixed data/code zone).

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

## 4. crt0 completion (from 0x10014C)

```javascript
0x100198..0x1001c0  gp=0x5AB270 (lui 0x5b; addiu -0x4d90), heap args a1=0x1EB000,
                    a2=0x10000000, a3=0x5A3A80, t0=0x10220
0x1001c0  move gp, a0
0x1001c4  v1=0x3C; syscall        ; kernel heap/stack setup -> sp = v0
0x1001d0  a0=0x1A7900; a1=-1
0x1001e0  v1=0x3D; syscall        ; secondary kernel setup
0x1001e8  jal System__BootInit1 (0x118C98)
0x1001f0  jal System__BootInit2 (0x10EBA0)  ; a0 = 0
0x1001fc  a0 = *(u32*)0x5A3A80
0x100208  jal System__BootInit3 (0x141EE0)  ; a1 = &0x5A3A84
0x100210  j   System__BootTail  (0x127DE0)  ; tail call
```

$gp value **verified against .reginfo** (0x005AB270). Syscall 0x3C returns
the stack pointer — kernel-managed stack placement.

## 5. Next steps

- [ ] Trace System__BootInit1/2/3 (0x118C98, 0x10EBA0, 0x141EE0) — find `main`
and the overlay loader (MWo3 parser must live somewhere).
- [ ] Decode remaining g_SystemHandlerTable entries (id/handler semantics).
- [ ] rizin raw-binary mode confirmed working for overlays (code @ file+0x80).
- [ ] Identify what service syscall 0x56..0x59 handlers provide.

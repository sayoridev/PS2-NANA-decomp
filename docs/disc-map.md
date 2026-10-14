# Disc Map — Nana (PS2, Japan)

> Produced during Phase 1 from the maintainer's personal dump.
> Structural facts only (paths, sizes). No game data is stored in this repo.
> Local dump path (private, never committed): `~/Downloads/Nana (Japan)/Nana (Japan).iso`
> ISO size: 3,169,124,352 bytes. SHA-256: *pending*.

## Identification

| Field | Value |
| --- | --- |
| Serial | SLPM_659.14 |
| Region | Japan |
| Publisher/engine | Konami (volume descriptor copyright: KCEJ) |
| Master date | 2005-02-02 (volume creation 2005-02-02 14:00) |
| Boot executable | `SLPM_659.14` (4,864,752 bytes) — EE ELF |
| Movie | `RWA/OPMV.PSS` (114,671,620 bytes) — PS2 PSS video |

## SYSTEM.CNF

*Pending `cat` output.*

## Directory overview

| Directory | Files | Role (hypothesis) |
| --- | --- | --- |
| `IOP/` | 10 | IOP drivers: 8 standard PS2 SDK + 2 custom Konami (`RWA.IRX`, `RTFSSIOP.IRX`) |
| `OVL/` | 7 | Loadable program overlays (release builds, suffix `R`): sub-game modules |
| `RWA/` | 5 | Opening movie (PSS), environment stream, SFX bank, sound driver data |
| `RWAB/` | 30 | BGM music streams (`BGM_001`–`BGM_030`, .RWS) |
| `RWAV/` | 25 | Voice streams (`V000`–`V024`, .RWS, ~30–48 MB each, ~1 GB total) |
| `SHOP2/` | 26 | Shop music banks (.NKP) |
| `STORY/` | 1 | `ALLSC.NKP` — 1,650,292,736 bytes; main story data (text/scene/voice) |
| root | 15 | `.NKP` packages + boot ELF |

## Root .NKP packages

| File | Size | Hypothesis |
| --- | --- | --- |
| `STORY/ALLSC.NKP` | 1,650,292,736 | Story data container (dominant file, 52% of disc) |
| `EPI_TXT.NKP` | 32,233,472 | Episode text |
| `WORK.NKP` | 37,754,880 | Work minigame data |
| `MISC_BG.NKP` | 17,254,400 | Misc backgrounds |
| `INTERIOR.NKP` | 5,996,544 | Interior scenes |
| `CHMAKE.NKP` | 9,332,032 | Character maker |
| `CLOTHES2.NKP` | 8,351,744 | Clothes data |
| `JOB_FND2.NKP` | 7,640,160 | Job finder |
| `ROLL.NKP` | 6,459,232 | Credits roll |
| `SHIFTBG.NKP` | 5,326,848 | Shift/schedule backgrounds |
| `MISC.NKP` | 1,853,088 | Misc |
| `TITLE.NKP` | 1,681,424 | Title screen |
| `ACCE.NKP` | 1,099,776 | Accessories |
| `MEMOCA.NKP` | 1,265,504 | Memo/card |
| `UI_WIN.NKP` | 177,328 | UI windows |
| `SELECT2.NKP` | 426,688 | Select screens |

## Overlays (OVL/)

| File | Size | Hypothesis |
| --- | --- | --- |
| `CLOTHE2R.BIN` | 38,272 | Clothes/wardrobe module |
| `ENDINGR.BIN` | 4,382,336 | Ending sequence |
| `JOBFND2R.BIN` | 9,984 | Job finder module |
| `MYROOMR.BIN` | 1,135,232 | "My room" module |
| `SHOP2R.BIN` | 15,872 | Shop module |
| `STATUSR.BIN` | 1,930,112 | Status screen |
| `WORKINGR.BIN` | 1,420,928 | Working minigame |

## IOP modules

| File | Size | Origin |
| --- | --- | --- |
| `IOPRP300.IMG` | 278,305 | Sony (OS image) |
| `CDVDSTM.IRX` | 33,521 | Sony SDK |
| `LIBSD.IRX` | 28,661 | Sony SDK (sound lib) |
| `MCMAN.IRX` / `MCSERV.IRX` | 96,181 / 7,385 | Sony SDK (memory card) |
| `PADMAN.IRX` | 45,797 | Sony SDK (controller) |
| `RWA.IRX` | 198,496 | **Custom Konami** (audio driver, pairs with .RWS streams) |
| `RTFSSIOP.IRX` | 28,412 | **Custom Konami** (purpose TBD) |
| `SDRDRV.IRX` | 9,161 | Sony SDK (sound driver) |
| `SIO2MAN.IRX` | 6,641 | Sony SDK (SIO2) |

## Audio streams (.RWS)

- `RWA/BGM_101.RWS` (6.4 MB), `RWA/ENV_001.RWS` (55.1 MB) — environment/bgm
- `RWAB/BGM_001`–`030` — 30 BGM tracks, 2–10 MB each
- `RWAV/V000`–`024` — 25 voice streams, ~30–48 MB each (~1 GB total) → fully voiced VN
- `RWA/RWA_SE1.NKP` — SFX bank

## Open questions

- [ ] `.NKP` container format (header, index, contents) — Phase 5
- [ ] `.RWS` stream format vs. RWA.IRX driver — Phase 5
- [ ] OVL .BIN load format (raw EE code at fixed address? relocatable?) — check `file`/`readelf`
- [ ] `RTFSSIOP.IRX` purpose — strings pass in Phase 2
- [ ] Is `ALLSC.NKP` self-contained or does it index external .RWS voice files?

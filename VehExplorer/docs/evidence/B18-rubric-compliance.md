# B18 — Rubric compliance audit (MS_1 scenario PDF)

Date: 2026-09-16  
Reference: `rubrics/MS_1/2026_RSE1201_GroupProject_MS1_Scenario.pdf`  
Page images: `rubrics/MS_1/2026_RSE1201_GroupProject_MS1_Scenario_pages/`  
Binary audited: `bin/VehExplorer` (BUILD_ORDER B0–B17 complete)  
Session evidence: `docs/evidence/Scenario01-output-log.txt`

Legend: **PASS** = aligned · **COSMETIC** = demo wording/layout · **GAP** = missing vs PDF · **N/A** = intentional / no public API

---

## Stage 1 — Configuration (PDF p.5)

| Check | Status | Notes |
|-------|--------|-------|
| Scenario filename prompt | **PASS** | Prompted at startup |
| Encrypted y/n | **PASS** | Maps to `fileNeedsDecryption` |
| Randomize start y/n | **PASS** | |
| Mission type 1–8 | **PASS** | Names match `SatComRelay.h` (PDF p.5 lists “Explore Everything” for 8; header says “Map Everything”) |
| Init confirmation line | **PASS** | Library prints `Initializing Vehicle Controller ... done!` |
| Initial energy one-liner | **PASS** | Same field order as PDF |
| Welcome banner | **COSMETIC** | PDF: `*** Welcome to Mr Tian's Vehicle Simulator ! ***`; we use `Manual control ready. Enter 'v'…` |
| Usage instructions (`v`) | **PASS** | Keys match PDF p.5–6 |
| `Press 'y' to continue` before loop | **COSMETIC** | PDF p.6 only; likely tutor demo pause — not implemented |

---

## Stage 2 — Manual control (PDF p.6–57)

| Check | Status | Notes |
|-------|--------|-------|
| `w/s/a/d` movement | **PASS** | Library prints `Vehicle moved '…' by 1 terrain unit.` |
| Numpad `1–9` scan (except `5`) | **PASS** | Eight directions wired |
| Key `5` handling | **COSMETIC** | PDF: `Invalid Command : '5', please try again!`; we: `Key '5' is unused…` |
| `t` radius toggle | **PASS** | Message format matches |
| `i` shield allocate | **PASS** | Library prints `N energy units allocated to shields!` on success |
| Shield validation errors | **COSMETIC** | Same logic; PDF uses `Error:` we use `Error!`; PDF prompt includes `(1 - max)` range |
| `z` status | **COSMETIC** | PDF adds header `Vehicle Status / Statistics :`; we print energy line (+ extra issued/success counters — acceptable superset) |
| `c` scanned map | **PASS** | Local grid; vehicle marked `_` / cyan underline |
| `x` explored map | **PASS** | Visited cells only; spawn + successful moves |
| Map after scan / move | **PASS** | Auto-print after action (matches tutor flow in PDF p.8–10, 41) |
| Map view headers | **COSMETIC** | PDF: `Currently Scanned/Explored Terrain Layout` + `(TUTOR Ver) Print … Map!`; we: `-- Scanned terrain map --` / `-- Explored / travelled map --` |
| Scan failure | **PASS** | `'\0'` handled; message printed |
| Failed move stability | **PASS** | B14 evidence; shield-down guard prevents library destroy segfault |
| Forbidden Print* APIs | **N/A** | Not used in product path (correct per header) |

---

## Stage 3 — Quit report (PDF p.58)

| Check | Status | Notes |
|-------|--------|-------|
| Quit on `q` | **PASS** | |
| `SCRelay_Signal_Veh_Close()` | **PASS** | Skipped if mission-over destroy path already ran |
| Total scans / movements | **COSMETIC** | PDF: “Total no. of scans/movements taken”; we: “Moves/Scans issued + successful” (richer) |
| Energy report block | **PASS** | Same five energy fields |
| Terrain types scanned (incl. zeros) | **GAP** | PDF lists fixed alphabet with zero counts; we list only terrains seen in session |
| Terrain types explored (incl. zeros) | **GAP** | Same |
| Mission statistics (type, progress, %) | **GAP** | PDF shows e.g. “Find All Danger Points”, points found, completion % — **not exposed in `SatComRelay.h`** |
| Extra discovery tables | **N/A** | Our move/scan energy-by-terrain tables exceed PDF; useful for Excel report |

---

## Appendix A / integration (PDF p.3)

| Check | Status | Notes |
|-------|--------|-------|
| Link all provided `*.o` | **PASS** | `makefile` uses `rubrics/MS_1/*.o` |
| Public API only | **PASS** | `SCRelay_Signal_Veh_*` only |
| No binary / `.dat` edits | **PASS** | |
| Works on unseen scenario | **PASS** | B15 evidence |

---

## Verdict

**Functional MS_1 requirements: PASS.** Keys, API usage, local maps, energy tracking, shield/scan/move flows, and second-scenario behaviour match the scenario PDF captures.

**Demo polish gaps (optional):** welcome banner, `Press y`, exact error/map header strings, key-5 message.

**Report gaps (discuss before fixing):** quit summary layout vs p.58 Exploration Report; mission completion stats may require tutor-only symbols or discovery — no public API in `SatComRelay.h`.

---

## Suggested follow-up (if aligning closer to PDF)

1. **Low effort:** Match PDF strings for welcome, key `5`, shield prompt range, `z` header, map titles.
2. **Medium:** Restructure `stats_print_summary` toward p.58 sections (rename headers; add total scans/movements lines).
3. **Hard / needs discovery:** Mission statistics block — investigate whether mission progress is derivable from session behaviour or remains tutor-only.

No code changes in B18 — audit and reference images only.

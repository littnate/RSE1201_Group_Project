# BUILD_ORDER.md — Independently testable build slices (MS_1)

Scope: SPEC.md Milestone 1 only. Check off when verification passes.  
No implementation design here — order and proof only.

---

## Progress

- [x] B0 — Toolchain & link smoke
- [x] B1 — Hardcoded Init / Close
- [x] B2 — Startup prompts → Init

- [x] B3 — Command loop skeleton (`v` / `q`)
- [x] B4 — Status display (`z`)
- [x] B5 — Movement (N/S/E/W)
- [x] B6 — Shield allocate (`i`)
- [x] B7 — Scan radius toggle (`t`)
- [x] B8 — Scan all 8 directions (radius 1)
- [x] B9 — Scan radius 2 (`*2` APIs)
- [x] B10 — Local explored map (`x`)
- [x] B11 — Local scanned map (`c`)
- [x] B12 — Issued vs successful counters
- [x] B13 — Quit summary report + Close
- [x] B14 — Failure / edge-case handling
- [x] B15 — Second-scenario (no hardcoding) check
- [x] B16 — Discovery notes for scenario report (manual)
- [x] B17 — Session transcript artifact
- [x] B18 — Rubric PDF pages + compliance audit
- [x] B19 — Tutor UI alignment (PDF p.5+ flow)

---

### B0 — Toolchain & link smoke

- **Objective:** Prove `gcc` can produce a runnable binary linked against **all** `rubrics/MS_1/*.o` plus a trivial `main`.
- **Dependencies:** None (WSL Ubuntu x86-64, `build-essential`).
- **Deliverable:** `makefile` + `bin/VehExplorer` (or equivalent) that links and starts.
- **Verification:** `make` succeeds with no link errors; running the binary exits 0 (e.g. prints a one-line hello and returns).

### B1 — Hardcoded Init / Close

- **Objective:** Prove SatComRelay accepts the sample encrypted scenario and tears down cleanly.
- **Dependencies:** B0.
- **Deliverable:** Program that calls `SCRelay_Signal_Veh_Init` with fixed path/`true`/mission, prints initial `VehDataType` energy fields, then `SCRelay_Signal_Veh_Close`.
- **Verification:** Run against `rubrics/MS_1/2026_Basic_Scenario_MS_1.dat`; see non-garbage initial energy; process exits without crash after Close.

### B2 — Startup prompts → Init

- **Objective:** Replace hardcoded init args with interactive Stage 1 prompts (filename, encrypted y/n, randomize y/n, mission 1–8).
- **Dependencies:** B1.
- **Deliverable:** Stage 1 CLI → Init → print initial status (still no control loop beyond that).
- **Verification:** Manual run entering sample path + `y` encrypted; wrong path or bad mission input fails clearly without hanging; correct path matches B1 energy printout.

### B3 — Command loop skeleton (`v` / `q`)

- **Objective:** Stage 2 loop that reads one command at a time until quit.
- **Dependencies:** B2.
- **Deliverable:** Loop supporting `v` (help text) and `q` (leave loop; Close may be stubbed until B13).
- **Verification:** After init, `v` shows key help; unknown keys ignored or messaged; `q` ends loop; no move/scan yet required.

### B4 — Status display (`z`)

- **Objective:** Show live vehicle energy/shield fields from latest `VehDataType`.
- **Dependencies:** B3.
- **Deliverable:** `z` prints status comparable to scenario PDF (initial / used / left energy & shield).
- **Verification:** Immediately after init, `z` matches Init printout; still no mutations required.

### B5 — Movement (N/S/E/W)

- **Objective:** Wire `w/s/a/d` to the four Move APIs and refresh held `VehDataType`.
- **Dependencies:** B4.
- **Deliverable:** Successful and failed moves observable via status energy change (or lack thereof).
- **Verification:** Sequence of moves changes `z` when successful; deliberate bump into blocked/out-of-range stays put and does not corrupt later commands.

### B6 — Shield allocate (`i`)

- **Objective:** Prompt for amount; call `AllocateToShield`; reject invalid amounts.
- **Dependencies:** B4 (B5 helpful but not required).
- **Deliverable:** `i` flow with success and error messages per sample behavior.
- **Verification:** Valid amount moves energy into shield on `z`; amount `>` reserves and `≤0` rejected; energy totals consistent.

### B7 — Scan radius toggle (`t`)

- **Objective:** Maintain local radius state 1 ↔ 2 and confirm via message.
- **Dependencies:** B3.
- **Deliverable:** `t` toggles and prints current radius; no scans required yet.
- **Verification:** Repeated `t` alternates “radius 1” / “radius 2” messages; state persists across other commands.

### B8 — Scan all 8 directions (radius 1)

- **Objective:** Map numpad (except 5) to the eight radius-1 Scan APIs; handle `'\0'` failure.
- **Dependencies:** B4, B7 (radius must be 1 for this slice).
- **Deliverable:** Scans return printable terrain chars or clear failure; `VehDataType` updated.
- **Verification:** Each of 8 directions invoked once; successes show a char and energy change on `z`; out-of-range yields failure without crash.

### B9 — Scan radius 2 (`*2` APIs)

- **Objective:** When radius is 2, call the corresponding `Scan*2` functions instead of radius-1.
- **Dependencies:** B7, B8.
- **Deliverable:** Same keys, different API by radius mode.
- **Verification:** With radius 2, scans differ from radius 1 (reach/energy); toggling back to 1 restores radius-1 behavior.

### B10 — Local explored map (`x`)

- **Objective:** Track cells visited by **successful** moves only; print explored/travelled map with `_` at current position.
- **Dependencies:** B5.
- **Deliverable:** `x` view built only from local state (no forbidden Print* APIs).
- **Verification:** After a known move path, `x` shows visited cells and correct underline; failed moves do not add cells; position underline matches last successful move.

### B11 — Local scanned map (`c`)

- **Objective:** Record successful scan results into a separate scanned grid; print with `_` at vehicle.
- **Dependencies:** B8 (B9 for radius-2 cells); B10 for shared position convention.
- **Deliverable:** `c` view distinct from `x`; unknown cells unmarked; no hard-coded terrain set.
- **Verification:** Scan then `c` shows new symbols; unscanned areas blank/unknown; `x` unchanged by scans-only; unknown symbol chars still display as returned.

### B12 — Issued vs successful counters

- **Objective:** Count move/scan commands issued vs successful (and any other SPEC G7 counters needed).
- **Dependencies:** B5, B8.
- **Deliverable:** Counters visible on `z` (or dedicated status lines).
- **Verification:** One failed move increments issued but not success; one successful scan increments both; totals match a short scripted session.

### B13 — Quit summary report + Close

- **Objective:** Stage 3: summary (final veh stats; per-terrain scanned vs explored counts; energy/shield/scan consumption breakdown as far as local data allows) then `Close` and exit.
- **Dependencies:** B6, B10, B11, B12.
- **Deliverable:** `q` prints report, closes vehicle, process exits cleanly.
- **Verification:** After a mixed session, report numbers reconcile with `z`/maps; second run still Inits (no stuck global state); no crash on Close.

### B14 — Failure / edge-case handling

- **Objective:** Harden SPEC edge cases without new features.
- **Dependencies:** B13.
- **Deliverable:** Stable handling for bad path, bad shield input, repeated failed move/scan, energy exhaustion still allowing `z`/`c`/`x`/`q`.
- **Verification:** Checklist run of SPEC §11 cases; program never segfaults; maps/counters not corrupted by failures.

### B15 — Second-scenario (no hardcoding) check

- **Objective:** Same binary works on another scenario path with no source changes.
- **Dependencies:** B14.
- **Deliverable:** Evidence (notes or transcript) of a full short session on a second `.dat` entered only at prompts.
- **Verification:** Init + move + scan + both maps + quit succeed; no baked-in filename/size/terrain table required; behavior plausible for a different map.

### B16 — Discovery notes for scenario report (manual)

- **Objective:** Capture experiment results for the Excel scenario report (terrain types; move/shield/scan energy patterns).
- **Dependencies:** B13 (B15 optional).
- **Deliverable:** Written notes (file or sheet draft) — **not** hard-coded into the program.
- **Verification:** Notes list observed terrains and example energy deltas from controlled trials on the default scenario; reproducible by repeating the same key sequence.

### B17 — Session transcript artifact

- **Objective:** Produce a submission-style terminal log of a demo session.
- **Dependencies:** B13.
- **Deliverable:** `docs/evidence/Scenario01-output-log.txt` via external `tee`/`script` per SPEC default U2.
- **Verification:** Log file exists, contains startup prompts + several commands + quit report; readable as a full session replay.

---

## Suggested dependency graph (summary)

```
B0 → B1 → B2 → B3 → B4 → B5 → B10
                 │         ├→ B6 ──────────────┐
                 ├→ B7 → B8 → B9 → B11 ────────┤
                 │         └→ B12 ← B5,B8 ─────┤
                 └─────────────────────────────→ B13 → B14 → B15
                                                    ├→ B16
                                                    └→ B17
```

B6 may proceed in parallel with B5 after B4. B16/B17 are documentation/artifact slices after core playable path (B13+). B18 is rubric reference + audit (no code required). B19 aligns CLI/report wording with scenario PDF captures.

### B18 — Rubric PDF pages + compliance audit

- **Objective:** Persist readable page images of the MS_1 scenario PDF and audit `VehExplorer` against tutor captures.
- **Dependencies:** B13+ (playable binary to compare).
- **Deliverable:** `rubrics/MS_1/2026_RSE1201_GroupProject_MS1_Scenario_pages/` (58 PNGs + `INDEX.md`); `docs/evidence/B18-rubric-compliance.md`; doc pointers in `PROJECT.md` / `SPEC.md`.
- **Verification:** 58 PNGs present; open p.05 (startup) and p.58 (quit report); read compliance doc — functional MS_1 marked PASS; cosmetic/report gaps listed for optional follow-up.

### B19 — Tutor UI alignment (PDF p.5+ flow)

- **Objective:** Align Stage 1–3 CLI wording and output order with scenario PDF captures; keep deliverable report fields; skip tutor-only mission %.
- **Dependencies:** B18.
- **Deliverable:** Updated `ui_cli` / map banners / shield errors / Exploration Report; `docs/evidence/tutor-replay-input.txt`, `tutor-replay-actual.txt`, `B19-tutor-alignment.md`.
- **Verification:** Replay script exits 0; checklist patterns in `B19-tutor-alignment.md` PASS; second-scenario smoke still Inits + quits cleanly.

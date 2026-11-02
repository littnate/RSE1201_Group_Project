# BUILD_ORDER.md — Independently testable build slices

MS_1 (B0–B20), MS_2 (M2-0 through C16), and the student-style refactor (S0) are complete.  
Current scope: **presentation** — slides and demo on the simplified sources.  
Check off when verification passes. No implementation design here — order and proof only.

---

## Progress (slides)

- [x] SL1 — Slide flow diagrams: MS1 overall, MS2 overall, scan, explore
- [x] SL1b — Slide call graphs: which source file calls which, MS1 and MS2
- [x] SL1c — MS1 slide: how `SatComRelay.h` was read, and which of our functions call it
- [ ] SL2 — Assemble the slide deck and demo script

### SL1 — Slide flow diagrams

- **Objective:** Compact program-flow diagrams for the presentation: overall architecture for MS1 and MS2, plus scan and explore as separate flows.
- **Dependencies:** S0 sources (the behaviour the diagrams describe).
- **Deliverable:** Diagrams in the slides chat, matched to `main`, `ui_cli`, `veh_action`, and `autopilot_run`. No new program behaviour.
- **Verification:** Each box names a stage or call that exists in those files. Scan writes the scanned grid only. A successful move marks the cell explored. MS2 `p` reuses `veh_action_*` and does not call SatComRelay itself.

Drafted 2026-11-02. No binary change, so the existing `bin/VehExplorer.run` session was left running.

### SL1b — File call graphs

- **Objective:** One slide-sized chart of cross-file calls for manual control, and one with `autopilot.c` added.
- **Dependencies:** SL1.
- **Deliverable:** Diagrams in the slides chat. Arrow means a call into that file. Each arrow has a few words for when that call happens.
- **Verification:** MS1 edges match `main.c`, `ui_cli.c`, `veh_action.c`, `mission_control.c`, `stats.c`. MS2 adds `ui_cli.c` → `autopilot.c`, and `autopilot.c` calls `veh_action`, `mission_control`, `local_map`, `stats`, and `util_input` only. `autopilot.c` does not call `SCRelay_Signal_Veh_*`.

Drafted 2026-11-02. No binary change.

### SL1c — SatComRelay usage (MS1)

- **Objective:** One slide section on how the public header was interpreted, and which of our functions call each group.
- **Dependencies:** SL1b.
- **Deliverable:** Diagram and table in the slides chat. Only `main.c` and `mission_control.c` call `SCRelay_Signal_Veh_*`. The four `Print*` functions are unused.
- **Verification:** Matches `MS_1_files/SatComRelay.h`, `main.c`, `mission_control.c`, and `docs/evidence/B16-discovery-notes.md`.

Drafted 2026-11-02. No binary change.

---

## Progress (presentation link)

- [x] P0 — Link `libsatcom.so`; smoke-test the reveal scenarios
- [x] S0 — Student-style refactor from `nathan` (`be7b116`)

### S0 — Student-style refactor

- **Objective:** Keep MS_1 and MS_2 behaviour, written with `switch`, `if` / `else`, and `while (true)` instead of function pointers, `goto`, and ternaries.
- **Dependencies:** P0.
- **Deliverable:** Fast-forward of `origin/nathan` onto `main`. Same eight modules. Study guides in `docs/v2_student_*.md`.
- **Verification:** `gcc -Wall -Wextra` from `VehExplorer/` with the usual link line, no warnings. Same scripted stdin on this binary and the previous `main` binary. Manual session on `New_2026_Basic_Scenario_MS_1.dat` (allocate 10000, north move, north scan, quit) and type 1 auto on `New_2026_Test_Scenario_MS_2.dat` (maps off, delays 0, move cap 3). Transcripts identical: auto stopped `move limit reached`, 3 successful moves, 31 successful scans.

Verified 2026-11-01 on WSL after the fast-forward. Local `bin/VehExplorer.run` was rebuilt from the new sources.

### P0 — Shared library link

- **Objective:** Produce `bin/VehExplorer.run` linked to `special-reveal/libsatcom.so`, and show the new scenarios Init, move or scan, and quit.
- **Dependencies:** MS_2 complete. Read-only drop in `special-reveal/`.
- **Deliverable:** One `gcc` line from `VehExplorer/` links `./libsatcom.so` with `-Wl,-rpath=.` and writes `bin/VehExplorer.run`. The line is in `README.md` and `TESTING.md`. No makefile. New `.dat` symlinks in `input-files/`. The eight `MS_1_files/*.o` files stay on disk and are not on the link line. Student sources unchanged.
- **Verification:** The `gcc` line succeeds. `ldd bin/VehExplorer.run` resolves `libsatcom.so`. From `VehExplorer/`: `./bin/VehExplorer.run`, then a manual session on `New_2026_Basic_Scenario_MS_1.dat` (allocate, one move, one scan, quit); type 1 auto with move cap 3 on `New_2026_Test_Scenario_MS_2.dat` and `Scenario05.dat`. One Init attempt on each older `.dat`.

Verified 2026-10-31 on WSL. The `gcc` line in `README.md` produced `bin/VehExplorer.run` with no warnings. `readelf -d` NEEDED is `./libsatcom.so` and `libc.so.6` only. Run `./bin/VehExplorer.run` from `VehExplorer/` (encrypted `y`, randomize `n`, mission 1). Quiet auto options: maps `n`, delays `0`, move cap `3`. The makefile was removed after that check; the same `gcc` line was run again.

| Check | Result |
|-------|--------|
| `ldd bin/VehExplorer.run` from `VehExplorer/` | Resolves `./libsatcom.so` |
| Same binary started from `/tmp` | Exit 127, `cannot open shared object file`. Expected: `-Wl,-rpath=.` loads `./libsatcom.so` from the current directory. Documented as a run constraint in `README.md`, `docs/PROJECT.md`, and `docs/SPEC.md` §5.4 |
| `New_2026_Basic_Scenario_MS_1.dat` | Init energy 210761; allocate 10000; move north succeeded; scan north returned `'w'`; `Vehicle closed` |
| `New_2026_Test_Scenario_MS_2.dat` | Init energy 255992; auto stopped `move limit reached`; 3 successful moves, 31 successful scans |
| `Scenario05.dat` | Init energy 20000; auto stopped `move limit reached`; 3 successful moves, 24 successful scans of 31 issued |
| `2026_Basic_Scenario_MS_1.dat` and `2026_Test_Scenario_MS_2.dat` | Both print `Initializing Vehicle Controller ... done!` with energy 100000000. A north scan on the old MS_1 file failed. Use the `New_*` files and `Scenario05.dat` for the demo. |

---

## Progress (MS_2)

- [x] M2-0 — Course files in `MS_2_files/`
- [x] M2-S — SPEC / PROJECT approved (draft in `docs/SPEC.md`)
- [x] C0 — Discovery (MS_2 `.dat`: end / danger / perimeter symbols, mission-over text)
- [x] C1 — `o` options prompts (Appendix A skip rules + stop a–c)
- [x] C2 — `p` toggle + idle auto loop (no planner yet)
- [x] C3 — Display gating + delays (`sleep`)
- [x] C4 — Stop a–c (energy / move cap / scan cap)
- [x] C5 — Shared act-vs-print helpers (manual maps still print; auto respects `o`)
- [x] C6 — Planner: shield + scan-before-move + walls
- [x] C7 — Type 1 casual (only a–c)
- [x] C8 — Types 2 and 3 (find 1 end / reach 1 end)
- [x] C9 — Types 4 and 5 (find/reach ALL ends)
- [x] C10 — Type 6 (find ALL danger, do not visit)
- [x] C11 — Type 7 (map boundaries)
- [x] C12 — Type 8 (map everything)
- [x] C13 — `p`/`q` interrupt; stuck/exhausted stop; Close rules
- [x] C14 — MS_1 regression + second-scenario auto smoke
- [x] C15 — Help text + evidence notes
- [x] C16 — MS2 scenario terrain workbook

Verified 2026-10-24 on WSL (`make` already up to date; `./bin/VehExplorer`). Quiet `o` (maps n, delays 0) unless noted. Scenario `input-files/2026_Test_Scenario_MS_2.dat` except the MS1 rows.

| Check | Result |
|-------|--------|
| `o` option 1 = y | Prompts 2–4 skipped |
| `o` t4 = 1 | Prompts 11–13 skipped; one scan cap slept ~2s (allocate + scan) |
| Bad y/n | Re-prompted |
| `p` before `o` | Options, then auto, move cap 3 |
| Type 1 move cap 5 / scan cap 3 | `move limit reached` / `scan limit reached` |
| Type 1 move cap 80 | Saw 2 `'E'`, explored 0, still stopped for the cap |
| Type 2 | Objective stop; `'E'` scanned 1, explored 0 |
| Type 3 | Objective stop; `'E'` explored 1 |
| Type 4 | Objective stop; `'E'` scanned 3 (did not stop at the first) |
| Type 5 | Objective stop; `'E'` scanned 3, explored 3 |
| Type 6 | Objective stop; `'X'` scanned 54, explored 0 |
| Type 7 | Objective stop; `'#'` scanned 80, explored 0; fewer moves than type 8 |
| Type 8 | Objective stop; every passable symbol explored == scanned; `'X'` and `'#'` explored 0 |
| Energy floor above starting energy | `energy below limit` before any move or scan |
| Maps n | No scanned/explored map dump during auto |
| `p` off and `q` during auto | Terminal session: `toggled off` returns to the prompt; `q` prints the report and `Vehicle closed` |
| MS1 manual | Shield allocate, one successful move, one scan, both map keys, quit |
| MS1 auto | Move cap 3, `Vehicle closed` |
| Missing file | Exit 1, `Cannot open scenario file` |

---

## Progress (MS_1 — complete)

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
- [x] B20 — MS1 scenario terrain workbook

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

---

## MS_2 slices

### M2-0 — Course files in `MS_2_files/`

- **Objective:** Keep the MS_2 brief and test scenario in-repo, read-only.
- **Dependencies:** None.
- **Deliverable:** `MS_2_files/2026_RSE1201_GroupProject_MS2_Scenario.pdf`, `2026_Test_Scenario_MS_2.dat`, page PNGs, `INDEX.md`; `input-files/` symlink.
- **Verification:** Files exist; `.dat` is not edited; program still Inits that path from `VehExplorer/` (MS_1 binary).

### M2-S — SPEC / PROJECT approved

- **Objective:** Lock MS_2 behaviour before coding.
- **Dependencies:** M2-0.
- **Deliverable:** `docs/SPEC.md` (MS_2), `docs/PROJECT.md`, this file’s MS_2 progress list.
- **Verification:** User replies Approve / Approve with changes.

### C0 — Discovery

- **Objective:** Record end / danger / perimeter symbols and any mission-over messages on `2026_Test_Scenario_MS_2.dat`.
- **Dependencies:** M2-S (may start in parallel as notes-only using the MS_1 binary).
- **Deliverable:** `docs/evidence/M2-discovery-notes.md` — **not** hard-coded cost tables.
- **Verification:** Notes list observed chars and example stop evidence; reproducible.

### C1 — `o` options prompts

- **Objective:** Interactive Appendix A + stop a–c with skip rules.
- **Dependencies:** M2-S.
- **Deliverable:** `o` in the command loop; saved options; summary print.
- **Verification:** 1=y skips 2–4; t4>0 skips t5–t7; t4=0 asks t5–t7; bad input re-prompts.

### C2 — `p` toggle + idle auto loop

- **Objective:** `p` on enters a loop that does **no** vehicle actions yet (or a no-op tick) and can be turned off.
- **Dependencies:** C1.
- **Deliverable:** Auto on/off flag; `p` before `o` runs `o` first; help lists `o`/`p`.
- **Verification:** `p` … `p` returns to `Enter your command`; `q` still quits.

### C3 — Display gating + delays

- **Objective:** When auto later acts, prints/sleeps follow options.
- **Dependencies:** C2, C5 (may land with first real action in C6).
- **Deliverable:** Sleeps t1–t7 / t4 override; stats/maps only if enabled.
- **Verification:** With maps=n and delays=0, auto output stays quiet except stop messages; with t4=1, ~1s pause per action.

### C4 — Stop a–c

- **Objective:** Energy floor, move cap, scan cap pause auto-pilot.
- **Dependencies:** C2.
- **Deliverable:** Checks after each auto action (and before starting).
- **Verification:** Cap of 1 move (once planner exists) or injected counts; energy floor stops; N=0 caps disabled.

### C5 — Shared act-vs-print helpers

- **Objective:** Manual `handle_move`/`handle_scan` still auto-print maps; auto-pilot can suppress.
- **Dependencies:** M2-S.
- **Deliverable:** Shared perform-move/scan functions with print flags.
- **Verification:** Manual session still prints maps after move/scan (MS_1 tutor flow).

### C6 — Planner: shield + scan-before-move + walls

- **Objective:** One generic explorer: allocate shield if needed, scan unknown neighbours, move onto known passable, treat impassable as walls.
- **Dependencies:** C5, C0 (for not dying on first hazard).
- **Deliverable:** `autopilot` next-action; uses `mission_control_*` only.
- **Verification:** Short auto run on MS_2 `.dat` moves/scans without destroy; `#` (or equivalent) not entered after a failed bump is known.

### C7 — Type 1 casual

- **Objective:** Casual explore runs until a–c / `p` / `q` / destroy — never “mission complete”.
- **Dependencies:** C4, C6.
- **Verification:** Mission 1 with move cap 5 stops for cap, not for finding `'E'`.

### C8 — Types 2 and 3 (find/reach 1 end)

- **Objective:** Stop on first scanned end vs first occupied end.
- **Dependencies:** C6, C0.
- **Verification:** Type 2 stops when scanned map shows one end and vehicle may not be on it; type 3 continues until occupy.

### C9 — Types 4 and 5 (ALL ends)

- **Objective:** Stop when auto-pilot believes all ends are found / all found ends visited.
- **Dependencies:** C8.
- **Verification:** Notes + session: does not stop at first `'E'` for type 4; type 5 visits each known end.

### C10 — Type 6 (ALL danger)

- **Objective:** Locate all danger; do not occupy known danger cells.
- **Dependencies:** C6, C0.
- **Verification:** Explored map has no danger cell as a visited destination (spawn exception only if spawn is danger).

### C11 — Type 7 (boundaries)

- **Objective:** Stop when perimeter / bounding walls are believed complete.
- **Dependencies:** C6.
- **Verification:** Auto stops after tracing `#` (or failed-scan edges) without requiring full interior mapping.

### C12 — Type 8 (map everything)

- **Objective:** Stop when scanned map is believed complete for reachable terrain.
- **Dependencies:** C6.
- **Verification:** Auto stops; remaining frontier empty; type 1 with same map would still only stop on a–c.

### C13 — Interrupts and teardown

- **Objective:** `p`/`q` stop auto; stuck message; Close rules unchanged.
- **Dependencies:** C2, C6.
- **Verification:** `q` during auto still reports + Close; destroy still skips Close.

### C14 — Regression + second scenario

- **Objective:** MS_1 keys still work; auto works on another path without source changes.
- **Dependencies:** C8 (minimum playable auto).
- **Verification:** Manual MS_1 `.dat` smoke; auto Init on both `.dat` files.

### C15 — Help + evidence

- **Objective:** `v` documents `o`/`p`; short transcript of an auto session.
- **Dependencies:** C8.
- **Deliverable:** Help text; `docs/evidence/` auto transcript.
- **Verification:** Help mentions configure + toggle; log contains `o` and `p`.

### C16 — MS2 scenario terrain workbook

- **Objective:** Same terrain-characteristics workbook as B20, measured on `2026_Test_Scenario_MS_2.dat`.
- **Dependencies:** C0, C12.
- **Deliverable:** `docs/evidence/MS2-scenario-terrain-report.xlsx`, plus session logs `M2-session-A-type8.txt`, `M2-session-B-probes.txt`, `M2-session-C-repeat.txt`. Not hard-coded into the program.
- **Verification:** Sheets Source, Terrain, Scan, Edge cases, Findings. Terrain `f` is 348 move / 559 shield, 74 samples, all from session A. Failed `#` move in session B: energy delta 394 and shield delta 1599 (repeat in session C matches). Session A scan totals: 416 successful + 80 out-of-range = 496 issued. Out-of-range radius-2 scans cost 180 cardinal and 720 diagonal, the same as a successful scan.

## Suggested MS_2 dependency graph

```
M2-0 → M2-S → C1 → C2 → C4
                 C5 → C6 → C7
                 C0 ↗     ├→ C8 → C9
                          ├→ C10
                          ├→ C11
                          └→ C12
              C2/C6 → C13
              C8 → C14, C15
         C3 with first real auto actions
```

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

### B20 — MS1 scenario terrain workbook

- **Objective:** Put the default-scenario terrain measurements into the Excel report the course asks for (PDF page 4, Appendix B: scan cost, move/shield cost, whether a symbol’s cost stays constant, impassable moves).
- **Dependencies:** B16.
- **Deliverable:** `docs/evidence/MS1-scenario-terrain-report.xlsx` — not hard-coded into the program.
- **Verification:** Workbook opens with sheets Source, Terrain, Scan, Edge cases, Findings. Terrain `f` is 645 move / 954 shield, 9 samples. Session C failed `#` move: totEnergyUsed 51553 → 52247 (694) and totShieldEnergyUsed 1863 → 3798 (1935). Session A scans sum to 5400, and 100000 allocate + 5400 = final totEnergyUsed 105400. Out-of-range scan left blank: those transcripts never failed a scan.

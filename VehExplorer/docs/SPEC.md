# SPEC — RSE1201 Group Project (Milestone 1)

**Course:** RSE1201 — C Programming (SIT)  
**Product name (working):** VehExplorer (from sample interactions; rename later if desired)  
**Scope of this spec:** Milestone 1 — Manual Control only  
**Status:** Draft for approval (no implementation until approved)

---

## 1. Problem summary

Build a **C** program that lets a human operator manually control a damaged exploration vehicle via SpaceXXX’s still-working satellite relay stack.

Your code does **not** reimplement vehicle physics, map loading, encryption, or scanning. It:

1. Prompts for scenario / mission configuration  
2. Calls the provided `SatComRelay` API  
3. Presents a terminal UI for move / scan / shield / maps / stats / quit  
4. Tracks enough local state to display maps and produce the required summary report correctly on **any** valid scenario file (not only the sample `.dat`)

Milestone 2 (auto-pilot) is **out of scope** for implementation now, but MS_1 structure should not paint us into a corner (clean separation of UI, control loop, and vehicle-facing API).

---

## 2. Goals and non-goals

### Goals (MS_1 compulsory)

| ID | Requirement |
|----|-------------|
| G1 | Startup prompts: scenario filename, encrypted y/n, randomize start y/n, mission type 1–8 |
| G2 | Move vehicle N / S / E / W |
| G3 | Scan 8 directions, radius 1 or 2 |
| G4 | Admin: init (via startup), allocate energy to shields, close/shutdown |
| G5 | Display map of terrain **travelled / explored** by the vehicle |
| G6 | Display map of terrain **scanned** by the vehicle |
| G7 | Display vehicle statistics (energy, shield, move/scan command counts & successes, etc.) |
| G8 | On quit: summary report — final veh stats; per-terrain scanned vs explored counts; energy / shield / scan costs as required by deliverables |
| G9 | Interactive loop until user shuts down and quits |
| G10 | Works on **unseen** scenario `.dat` files without source changes (no hard-coded map, terrains, or energy tables) |
| G11 | Build with `gcc` on Ubuntu/Linux x86-64; link provided `.o` files |

### Non-goals (for now)

- Auto-pilot / MS_2 decision engine  
- GUI  
- Decrypting or editing scenario files yourself  
- Reverse-engineering or modifying provided `.o` binaries  
- Using verification-only APIs meant for tutor testing (`PrintEntireMap` / password-gated helpers marked “Do not use!”) for normal gameplay  
- Fancy extras beyond a small optional list (see §11)

---

## 3. Assumptions

| # | Assumption |
|---|------------|
| A1 | Solo development for now. |
| A2 | Primary environment: **WSL2 Ubuntu** (x86-64). Course VM Ubuntu 22.04 is for later clone/demo if needed. |
| A3 | Provided `.o` files are ELF x86-64 and will link on WSL Ubuntu; same sources should build on an Ubuntu 22.04 VM if glibc/toolchain are compatible (see §8). |
| A4 | Official integration surface is **`SatComRelay.h` only**. Other modules are linked transitively via the provided objects. |
| A5 | When `fileNeedsDecryption == true`, `SCRelay_Signal_Veh_Init` (and underlying libs) handle decryption — we pass the flag, we do **not** decrypt `.dat` ourselves. |
| A6 | UI should follow the **sample CLI keybindings** in the MS_1 scenario PDF (not a numbered menu), so demo behavior matches tutor expectations. |
| A7 | Mission type is selected at init for correctness / future MS_2; MS_1 does not auto-complete missions. |
| A8 | “No hardcoding” means: no fixed map size, no fixed terrain symbol list/costs, no fixed scenario name, no assumptions that only the sample map exists. Values are discovered via API returns + runtime experimentation for the scenario report. |
| A9 | Course materials live under `rubrics/` and are reference-only; application source will live elsewhere in the repo (see §9). The MS_1 scenario PDF is image-only — rendered page PNGs live in `rubrics/MS_1/2026_RSE1201_GroupProject_MS1_Scenario_pages/` (see `INDEX.md` and `docs/evidence/B18-rubric-compliance.md`). |
| A10 | Submission naming (`TeamN-MS_1`, …) is deferred; ignore until packaging time. |

---

## 4. Core requirements (behavioral)

### 4.1 Stage 1 — Configuration

On startup, prompt approximately as in the scenario PDF:

1. Scenario file name / path (e.g. `2026_Basic_Scenario_MS_1.dat`)  
2. Is the file encrypted? (`y`/`n`) → maps to `bool fileNeedsDecryption`  
3. Randomize start position? (`y`/`n`)  
4. Mission type `1`–`8` (see `SatComRelay.h` comments; sample UI labels “Explore Everything” for type 8)

Then call:

```c
VehDataType SCRelay_Signal_Veh_Init(
    char *srcFilename,
    bool fileNeedsDecryption,
    bool randomizeStartPosition,
    int missionType
);
```

Show initial energy / status after successful init. Fail gracefully if init fails (file missing, bad path, etc.) — exact failure modes depend on library behavior (to be recorded during discovery).

### 4.2 Stage 2 — Manual control loop

Match sample interaction style (Appendix B + MS_1 scenario screen captures):

| Key | Action |
|-----|--------|
| `w` / `s` / `a` / `d` | Move North / South / West / East |
| Numpad `1`–`9` except `5` | Scan SW,S,SE,W,E,NW,N,NE |
| `t` | Toggle scan radius between 1 and 2 |
| `i` | Allocate energy to shields (prompt amount; reject invalid / insufficient) |
| `z` | Show vehicle status / statistics |
| `c` | Show scanned terrain map |
| `x` | Show explored / travelled terrain map |
| `v` | Show usage / help |
| `q` | Quit → Stage 3 |

Notes:

- In map display, underline `_` (or equivalent) marks current vehicle location (per sample).  
- Scan/move must call the corresponding `SCRelay_Signal_Veh_*` functions.  
- Track **local** counters the API does not expose (e.g. total move commands issued vs successful), if required for G7/G8.

### 4.3 Stage 3 — Shutdown & report

1. Display / generate summary report (final stats; per-terrain scanned vs explored; movement / shield / scan energy consumption breakdown as required by deliverables).  
2. Call `SCRelay_Signal_Veh_Close()`.  
3. Exit cleanly.

A terminal interaction log (`ScenarioXX-output-log.txt` style) is a **submission artifact**; capture strategy is TBD (manual `script` vs in-program logging) — see Unresolved Decisions.

### 4.4 Correctness criteria (grading-facing)

- Explored map matches actual moves that succeeded.  
- Scanned map matches successful scans.  
- Energy / shield / counters match library state + observed costs.  
- Same binary works on **new unseen** scenarios without recompile of hard-coded data.

---

## 5. Technical architecture

### 5.1 Ecosystem

```
[ Your *.c / *.h ]  +  [ SatComRelay.h + provided *.o ]  +  [ libc ]
                         └─ VehController, RadarSensor, Terrain*,
                            ConfigSettings, FileUtil, ExpReport, ...
        └─ gcc link ─►  VehExplorer (executable)
```

You only **call** `SCRelay_Signal_Veh_*`. You do not open-code against other modules unless discovery proves a header is missing and tutor materials require it (currently only `SatComRelay.h` is provided).

### 5.2 Suggested module split (MS_1, MS_2-friendly)

| Module | Responsibility |
|--------|----------------|
| `main.c` | Entry, high-level stage flow |
| `ui_cli.c` / `.h` | Prompts, key read loop, help text, map/stat printing |
| `mission_control.c` / `.h` | Translate keys → SatComRelay calls; scan radius state; shield allocate flow |
| `local_map.c` / `.h` | Maintain explored/scanned grids from scan results & successful moves (no reliance on forbidden full-map dump) |
| `stats.c` / `.h` | Command success/fail counters; report assembly |
| `util_input.c` / `.h` | Safe line/char input helpers |

Exact file names can change; the separation matters more than names.

### 5.3 Data the program must own locally

`VehDataType` from the API gives energy fields. For maps and rich reports, assume we must **record**:

- Relative or absolute grid of cells we have seen (char terrain symbols from scan returns)  
- Which cells were actually visited  
- Current position (infer from moves; underline on print)  
- Scan radius mode (1 vs 2)  
- Issued vs successful move/scan counts  
- Optionally: observed energy deltas per action for the Excel discovery report  

**Do not** call password-gated `SCRelay_Signal_Veh_PrintEntireMap` (and siblings marked “Do not use!”) as part of the student solution path.

### 5.4 Build

- Compiler: `gcc`  
- Link all provided MS_1 `*.o` with your objects  
- `makefile` producing binary under `bin/` (final packaging layout per Appendix E-3 when we package)  
- Suggested flags (adjust after first successful link): `-Wall -Wextra` ; C standard e.g. `-std=c11` unless course requires otherwise  

---

## 6. Data sources and storage

| Source | Role | Rules |
|--------|------|--------|
| `*.dat` scenario files | Encrypted (usual) or rare `*.txt` | Pass path + decrypt flag into Init; **never edit** scenario files |
| User keyboard | Commands & config | Validate ranges; reject bad shield amounts |
| SatComRelay return values | Terrain chars, `VehDataType` energy | Sole runtime truth for vehicle state |
| In-memory local map/stats | Display + report | Derived only from successful API interactions |
| Optional log file | Submission evidence | Append-only transcript of session |

No database. No network I/O beyond what the libs might do internally (assume local).

---

## 7. APIs and integrations

Primary header: `rubrics/MS_1/SatComRelay.h`

### Admin

- `SCRelay_Signal_Veh_Init(...)`  
- `SCRelay_Signal_Veh_Close()`  
- `SCRelay_Signal_Veh_AllocateToShield(long amount)`

### Movement

- `..._MoveUpNorth / MoveDownSouth / MoveRightEast / MoveLeftWest`

### Scanning

- `..._Scan{Direction}` and `..._Scan{Direction}2` for radius 2  
- Returns `char` terrain; `'\0'` if unsuccessful  

### Forbidden / tutor-only (do not use in product path)

- `PrintEntireMap` / `PrintScannedMap` / `PrintExploredMap` / `PrintReport` with password  

Mission type integers 1–8 as documented in the header.

---

## 8. Security

| Topic | Approach |
|-------|----------|
| Scenario encryption | Library-handled via Init flag; no custom crypto |
| Binary integrity | Do not patch/strip provided `.o` |
| Input safety | Bounds-check buffer reads; reject negative / oversize shield alloc |
| Secrets | Ignore password-gated tutor APIs |
| Path handling | Treat scenario path as untrusted string; don’t invent decryptor that writes plaintext next to assignment files unless library does |

---

## 9. Performance

Not a performance-critical system. Maps are small classroom grids.

- Prefer simple 2D arrays / dynamic grow if bounds unknown until discovery  
- Avoid unnecessary full-map scans every keypress  
- No concurrency required for MS_1  

---

## 10. Deployment & portability (WSL ↔ Ubuntu VM)

**Will WSL Ubuntu code work on Ubuntu 22.04 VM after clone?**

**Usually yes for your source + makefile**, with these caveats:

1. **Architecture** must stay x86-64 (provided `.o` are x86-64).  
2. **Bring the provided `.o` + `.h` + `.dat` with the repo** (or copy from `rubrics/MS_1`). Don’t assume the VM has them.  
3. **glibc / Ubuntu version mismatch** is the main risk: if WSL is a much newer Ubuntu than 22.04, prebuilt `.o` almost always still *link*, but if anything ever fails, rebuild/test on the VM early. Prefer installing **Ubuntu 22.04 on WSL** if easy, to match the course VM.  
4. Clone → install `build-essential` → `make` → run with scenario path relative to `input-files/`.

No cloud deployment. Deliverable is a local Linux executable + docs.

### Proposed repo layout (working tree)

```
kor/
  docs/                    # SPEC.md, PROJECT.md, BUILD_ORDER.md
  rubrics/                 # course materials (read-only reference)
    docs/                  # main brief, deliverable sheet, contribution form
    MS_1/                  # header, objects, sample .dat, scenario PDF + page PNGs
    MS_2/                  # held for later
  src/                     # (to be created) your C sources
  inc/                     # (to be created) your headers
  ...                      # makefile / bin / obj / input-files when implementing
```

---

## 11. Edge cases

| Case | Expected handling |
|------|-------------------|
| Missing / wrong scenario path | Clear error; do not crash silently |
| Encrypted flag wrong | Observe library behavior; surface error; don’t invent decrypt |
| Move into impassable / out of range | Detect via energy unchanged / API semantics; count as failed move; stay put |
| Scan out of range | `'\0'` or unsuccessful; count failed scan; don’t corrupt local map |
| Shield alloc > remaining energy or ≤ 0 | Reject with message (sample shows insufficient energy) |
| Unknown terrain symbols on new maps | Store and display the `char` as returned; don’t assume a fixed alphabet |
| Energy exhaustion mid-mission | Disable or fail moves/scans cleanly; still allow status/maps/quit |
| Randomize start | Don’t assume a fixed spawn cell in code or tests |
| Repeated failed moves/scans | Must remain stable (hinted in scenario PDF) |

---

## 12. Trade-offs

| Decision | Choice | Why | Alternative |
|----------|--------|-----|-------------|
| UI style | Sample keybindings | Matches tutor captures / demo muscle memory | Numbered menu — clearer for novices, less demo-aligned |
| Map source | Local tracking from API | Avoids forbidden print APIs; works offline from tutor password | Calling tutor print helpers — risky / against header |
| Extras | **Minimum first** | Marks are correctness + report; extras are GTH | Small extras later (see below) |
| Encrypt handling | Flag only | Assignment forbids DIY decrypt | Custom decrypt — out of scope / likely wrong |
| Hardcoded costs | None in code | Unseen scenarios | Tables from sample only — fails new maps |

### Optional “good to have” (defer unless time)

Worth considering **after** core works — not required now:

1. **Session log writer** — helps produce required `ScenarioXX-output-log.txt` without relying on external `script`.  
2. **Replay-friendly verbose mode** — echo energy delta after each action (helps Excel discovery report).  
3. **Config file** for defaults (still overridable at prompt) — nice, not necessary.

Course note: basic input validation is **not** graded as an “enhancement”; it’s expected hygiene.

---

## 13. Deliverables awareness (docs — not coding yet)

When packaging MS_1, course expects roughly:

- Program + `makefile` in Appendix E-3 layout  
- Word architecture/design/testing/reflection report  
- Excel scenario discovery (terrain types, move/shield/scan energy findings)  
- PowerPoint for demo  

This spec does not schedule those; implementation plan will after approval.

---

## 14. Things easy to miss

1. Linking **all** provided `.o` files, not only `SatComRelay.o`.  
2. Building explored vs scanned maps as **two different views**.  
3. Radius-2 scans use the `*2` API variants.  
4. Stats must distinguish **commands issued** vs **successful** actions.  
5. Report section on **per-terrain** and **per-scan-direction/radius** energy — needs disciplined discovery experiments, not guesses.  
6. Don’t hardcode the sample filename or map dimensions.  
7. Keep MS_2 out of the control loop for now, but keep `mission_control` / `ui` separable so autopilot can call the same actions later.

---

## 15. Unresolved decisions

| ID | Question | Default if unanswered |
|----|----------|------------------------|
| U1 | Exact executable name (`VehExplorer` vs other)? | `VehExplorer` |
| U2 | In-program session logging vs external `script`/`tee`? | External `script` for MS_1 minimum; add in-program log if packaging becomes painful |
| U3 | How to learn map bounds without full-map API — grow grid from relative moves, or probe until failure? | Grow from relative coordinates starting at (0,0); mark unknown as unseen |
| U4 | WSL Ubuntu version (22.04 vs 24.04)? | Check with `lsb_release -a`; prefer 22.04 if choosing a new WSL distro |
| U5 | C standard flag required by tutor? | `-std=c11` until told otherwise |
| U6 | Depth of MS_1 summary report vs what `ExpReport.o` might already compute if linked — may we call those symbols without a header? | Prefer own report from local stats; revisit only if discovery shows a supported public path |
| U7 | Final on-disk project folder names for submission | Defer until packaging |

---

## 16. Success definition (MS_1)

A successful MS_1 is when, on WSL Ubuntu:

1. `make` builds an executable linked against `rubrics/MS_1/*.o`  
2. Operator can configure, move, scan (r=1/2), allocate shields, view both maps + stats, quit with report  
3. Behavior stays correct on a **second** scenario file not baked into source  
4. No custom decryption and no modification of provided binaries or `.dat` files  

---

## 17. Approval gate

**No implementation plan and no code until you approve this SPEC** (or request edits).

Please reply with:

- **Approve** / **Approve with changes** (list changes)  
- Answers to any Unresolved Decisions you care about (especially U1, U2, U4)

# SPEC — RSE1201 Group Project (Milestone 2)

**Course:** RSE1201 — C Programming (SIT)  
**Product name:** VehExplorer  
**Scope of this spec:** Milestone 2 — Auto-pilot (MS_1 manual control remains)  
**Status:** Approved (2026-10-24)

**MS_1 baseline:** Manual control (B0–B19) is complete and **must keep working**. This spec adds auto-pilot on top of that program.

**Brief:** `MS_2_files/2026_RSE1201_GroupProject_MS2_Scenario.pdf` (3 pages; renders in `MS_2_files/pages/`).

---

## 1. Problem summary

MS_1 restored **manual** satellite control. MS_2 must **evolve that same program** so the vehicle can also fly on **auto-pilot**: it chooses when to scan, which way and how far, how to move cheaply, and when/how much to allocate to shields — in order to meet the **mission type already selected at Init** (1–8).

The operator still uses the terminal. Auto-pilot is a function that, once started, runs **without further keypresses** until a configured pause/stop condition is met (or the operator toggles it off).

You still do **not** reimplement physics, map loading, encryption, or scanning. You still only call `SCRelay_Signal_Veh_*` (via the existing `mission_control` wrappers).

---

## 2. Goals and non-goals

### Goals (MS_2 compulsory)

| ID | Requirement |
|----|-------------|
| G1 | Keep all MS_1 behaviour: Stage 1 prompts, manual keys, maps, stats, quit report, Close |
| G2 | New key **`o`**: configure auto-pilot **display, delays, and stop criteria** (Appendix A + page 3) |
| G3 | New key **`p`**: toggle auto-pilot **on / off** |
| G4 | While on, auto-pilot chooses a sequence of scan / move / shield actions (no human move/scan keys required) |
| G5 | Decisions cover: scan direction and radius; energy-aware travel; move direction; shield allocation |
| G6 | Aim for an **efficient and/or effective** sequence for the **pre-configured** mission type (Init 1–8) |
| G7 | Auto-pilot **keeps running** until pause/stop criteria, `p` off, `q` quit, or library mission-over / destroy |
| G8 | Configurable **output filters and pauses** so the terminal stays followable (Agent T) |
| G9 | Configurable **stop/pause**: energy floor, max moves, max scans, plus mission-objective stop (page 3) |
| G10 | Same binary works on **unseen** scenario `.dat` files (including `MS_2_files/2026_Test_Scenario_MS_2.dat`) |
| G11 | Still `gcc` + link **all** provided `MS_1_files/*.o`; do not decrypt `.dat` or patch binaries |

### Non-goals

- GUI / ncurses  
- Replacing manual control  
- Custom decryption or editing scenario files  
- Reverse-engineering / modifying provided `.o`  
- Password-gated `PrintEntireMap` / `PrintScannedMap` / `PrintExploredMap` / `PrintReport`  
- Calling undocumented symbols from other `.o` files  
- Hard-coded map size, spawn, full terrain alphabet, or energy-cost tables from one scenario  

---

## 3. Assumptions

| # | Assumption |
|---|------------|
| A1 | Solo development unless told otherwise. |
| A2 | Primary env: **WSL2 Ubuntu x86-64**; later demo on Ubuntu 22.04 VM. |
| A3 | MS_2 uses the **same** `SatComRelay.h` + `MS_1_files/*.o`. No new course objects were provided. |
| A4 | Mission type is still chosen at **Init** (MS_1 Stage 1). Auto-pilot does not re-prompt mission type. |
| A5 | Auto-pilot must call the same `mc_*` actions as the keyboard so maps/stats/report stay consistent. |
| A6 | CLI keys `o` and `p` match the MS_2 PDF examples; other MS_1 keys unchanged. |
| A7 | `'E'` (end), `'X'` (danger), `'#'` (perimeter / impassable) in the PDF are **examples**. Roles are discovered from scans + move failures; do not assume every map uses only those letters, and do not assume MS_1’s passable `'X'` is danger on every file. |
| A8 | “Believes it has found ALL …” means: local knowledge + exploration of remaining unknown frontier is exhausted (or a library mission-over message), not a tutor-only progress API. |
| A9 | Course files: `MS_1_files/` (library) and `MS_2_files/` (MS_2 PDF + test `.dat`). Do not edit either. |
| A10 | Submission zip names deferred until packaging. |

---

## 4. Core requirements (behavioral)

### 4.1 Stage 1 — Configuration (unchanged)

Same as MS_1: scenario path, encrypted y/n, randomize start y/n, mission type 1–8, then `SCRelay_Signal_Veh_Init`.

Typical MS_2 sample:

```text
input-files/2026_Test_Scenario_MS_2.dat
y
n
<1–8>
```

### 4.2 Stage 2 — Manual control (unchanged) plus two keys

All MS_1 keys remain (`w/s/a/d`, numpad scans, `t`, `i`, `z`, `c`, `x`, `v`, `q`).

| Key | Action |
|-----|--------|
| `o` | Run the auto-pilot **options** prompts (Appendix A + stop criteria). May be used before first `p`, and again while auto-pilot is **off** / paused. |
| `p` | Toggle auto-pilot **on** or **off**. |

If `p` is pressed before `o` has been completed at least once, run `o` first (operator must have display/stop settings), then start auto-pilot.

Help text (`v`) must mention `o` and `p`.

### 4.3 Auto-pilot options (`o`) — Appendix A

Prompt in this order. Use **yes/no** for 1–6 and **non-negative numbers** for delays and stop limits. Reject empty / garbage; re-prompt.

**Display**

| # | Prompt (sense) | Notes |
|---|----------------|--------|
| 1 | After **every** action, show vehicle status / energy stats? (y/n) | If **y**, do **not** prompt 2–4 (1 overrides 2–4). If **n**, prompt 2–4. |
| 2 | After every **scan**, show energy stats? | Only if 1 = n |
| 3 | After every **move**, show energy stats? | Only if 1 = n |
| 4 | After every **shield allocation**, show energy stats? | Only if 1 = n |
| 5 | After every scan, show **scanned** map? (y/n) | |
| 6 | After every move, show **travelled** map? (y/n) | |

**Delays (seconds)** — PDF labels `t1`…`t7`

| # | Prompt (sense) | Notes |
|---|----------------|--------|
| 7 | `t1` delay after displaying energy stats | |
| 8 | `t2` delay after displaying scanned map | |
| 9 | `t3` delay after displaying travelled map | |
| 10 | `t4` delay after **any** auto-pilot action | If **t4 > 0**, do **not** prompt 11–13. If **t4 == 0**, prompt 11–13. |
| 11 | `t5` delay after each auto **scan** | Only if t4 == 0 |
| 12 | `t6` delay after each auto **move** | Only if t4 == 0 |
| 13 | `t7` delay after each auto **shield** | Only if t4 == 0 |

Delay `0` means no sleep. Use whole seconds (`sleep`); fractional not required.

**Stop / pause criteria** (PDF page 3)

| # | Prompt (sense) | Notes |
|---|----------------|--------|
| a | Pause/stop when current energy available falls **below** N | `currEnergy < N` |
| b | Pause/stop when total **move actions** exceed N | Count **issued** auto+manual moves unless discovery says otherwise; default = issued (`stats` moves_issued) |
| c | Pause/stop when total **scan actions** exceed N | Same: `scans_issued` |
| d | Mission-objective stop | Not a y/n — always **on**. Behaviour depends on Init mission type (see §4.5). |

**Cap meaning (default):** for **b** and **c**, `N == 0` means **no cap** (disabled). For **a**, `N == 0` means stop when energy is **≤ 0** (already unsafe to continue).

Print a one-screen summary of the saved options after `o` completes.

### 4.4 Auto-pilot run (`p` on)

When auto-pilot is **on**:

1. Do **not** wait for move/scan keys. Loop: decide one action → perform it through `mission_control` → optional print → optional delay → check stop.
2. Manual keys other than `p` / `q` are not required during a run. If a key is read, only **`p` (off)** and **`q` (quit)** need to interrupt; other keys may be ignored until auto is off. (See U12 if we later allow `o` mid-run.)
3. Each action must update `local_map` and `stats` the same way a human keypress does (reuse the existing handle paths or shared helpers).
4. Honour display flags: energy one-liner and/or maps only when the matching option is yes.
5. Honour delays after the matching display / action.
6. If shields would be too low to survive a damaging move, **allocate** before moving (never move with `currShieldEnergy <= 0` — MS_1 already DESTROYs the vehicle).
7. Prefer **scan-before-move** onto unknown cells. Do not knowingly step onto impassable cells.
8. For “find danger” missions, do **not** occupy a known danger cell.
9. Scan radius: choose 1 or 2 via `mc_set_scan_radius` (to be added); do not require the user to press `t`.
10. Stop the loop when any §4.5 condition is true; leave the operator in the **manual** command loop (auto off), unless `q` or library mission-over already ended Stage 2.

### 4.5 Mission-objective stop (page 3)

Uses Init `missionType`. Auto-pilot **believes** based on **local_map** (and library DESTROYED / MISSION OVER if it occurs).

| Type | Name | Stop when |
|------|------|-----------|
| 1 | Casual Explore | **Only** energy / move-cap / scan-cap (a–c). No “map complete” stop. |
| 2 | Find 1 End Point | A successful scan has recorded **one** end-point terrain (PDF example `'E'`) |
| 3 | Reach 1 End Point | Vehicle **occupies** an end-point cell (successful move onto it, or spawn on it) |
| 4 | Find ALL End Points | Auto-pilot believes **all** end points on the reachable/scannable map are located (frontier exhausted + no remaining unknown that could hide another, within what scans can reveal) |
| 5 | Reach ALL End Points | Auto-pilot believes it has **visited** every end point it has found, and that no further end points remain (same “all” belief as type 4, plus occupancy of each found end) |
| 6 | Find ALL Danger Points | Same “all located” belief for **danger** cells (PDF example `'X'`). Do not require visiting them. |
| 7 | Find Map Boundaries | Auto-pilot believes it has located the **perimeter** (PDF example `'#'`, plus cells where scan/move fails at the edge) |
| 8 | Map Everything | Auto-pilot believes it has scanned/located the **entire** reachable map of terrains |

Print a clear reason when auto-pilot pauses or stops, e.g. `Auto-pilot stopped: found 1 end point.` / `energy below limit.` / `toggled off.`

### 4.6 Stage 3 — Shutdown (unchanged)

`q` → summary report → `Close` (skip Close if already mission-over destroy) → exit.

### 4.7 Correctness

- Explored/scanned maps and counters still match successful API results.  
- Auto-pilot must not call forbidden Print* APIs.  
- Auto-pilot must not decrypt `.dat`.  
- Destroy / energy-0 / shield-0 guards from MS_1 remain.  
- Second scenario without source changes still Inits and can run auto-pilot.

---

## 5. Technical architecture

### 5.1 Ecosystem (unchanged)

```
[ src/*.c ]  +  [ MS_1_files/SatComRelay.h + *.o ]  →  bin/VehExplorer
```

MS_2 adds **student** modules only.

### 5.2 Module split

| Module | Responsibility |
|--------|----------------|
| `main.c` | Stages unchanged |
| `ui_cli` | Prompts, keys, help, `o`/`p`, print gating for auto output |
| `mission_control` | SatComRelay wrappers; add `mc_set_scan_radius` |
| `local_map` | Explored/scanned grids; extra queries for planner (bounds, peek, frontier) |
| `stats` | Counters + quit report (unchanged contract) |
| `util_input` | Safe input |
| **`autopilot` (new)** | Options struct; decide next action; mission-goal tests; run loop |
| **`ap_config` (optional)** | If `o` prompts get large, keep them next to autopilot rather than bloating `ui_cli` |

Auto-pilot **must not** call `SCRelay_Signal_Veh_*` directly.

### 5.3 Suggested planner (implementation, after spec approval)

Each tick:

1. If stop condition already true → turn auto off, return to manual loop.  
2. If shield below a safe reserve and energy remains → allocate a chunk (leave energy for scans/moves).  
3. Inspect 8 neighbours in `local_map`.  
4. If a mission-useful unknown cell is adjacent → scan it (radius 1 if adjacent unknown; radius 2 only when that helps).  
5. Else if a useful passable cell is known → BFS on known passable cells, step one cardinal move toward it.  
6. Else expand frontier (scan farther / move toward unknown).  
7. If no legal action (stuck) → stop with a message (do not spin).

“Useful” depends on mission type (unknown, end, danger, perimeter, unvisited end, etc.).

Terrain **roles** (end / danger / wall) start from PDF examples and **discovery on the MS_2 `.dat`**, then treat unknown future symbols as data: walls = failed impassable moves; ends/dangers = classified symbols list that can grow if library messages name them. Do **not** copy MS_1 per-terrain cost tables into source.

### 5.4 Build

Same makefile: `wildcard src/*.c` already picks up new files. Link all `MS_1_files/*.o`. Flags: `-std=c11 -Wall -Wextra`.

---

## 6. Data sources

| Source | Role |
|--------|------|
| `*.dat` | Passed to Init; never edited |
| Keyboard | Stage 1, manual keys, `o` config, `p`/`q` |
| SatComRelay returns | Vehicle energy + scan chars |
| `local_map` / `stats` | Belief for maps, report, and “ALL found” |
| Auto-pilot options | In-memory; not required to persist across process runs |

---

## 7. APIs

Same public header: `MS_1_files/SatComRelay.h`.

Forbidden: password Print* helpers.

---

## 8. Security / safety

- No custom crypto; Init flag only.  
- Bounded input; reject negative delays and negative stop limits.  
- Never move with shields down.  
- Never call vehicle APIs after DESTROYED / MISSION OVER.  
- Do not store tutor passwords.

---

## 9. Performance

Classroom maps. Planner may be simple BFS on the known grid. Sleeps are for the operator, not for speed.

---

## 10. Deployment

Unchanged: local Linux x86-64 executable. Sample MS_2 path: `input-files/2026_Test_Scenario_MS_2.dat` (symlink to `MS_2_files/`).

### Repo layout (actual)

```
RSE1201_Group_Project/
  MS_1_files/              # SatComRelay.h, *.o, MS_1 .dat + PDF  (read-only)
  MS_2_files/              # MS_2 PDF, test .dat, page PNGs       (read-only .dat/PDF)
  VehExplorer/
    docs/                  # SPEC.md, PROJECT.md, BUILD_ORDER.md
    src/  inc/  obj/  bin/
    input-files/           # symlinks to scenario .dat files
    makefile
```

---

## 11. Edge cases

| Case | Handling |
|------|----------|
| `p` before `o` | Run `o`, then start auto |
| Invalid y/n or number in `o` | Re-prompt; do not start with garbage |
| Energy hits floor mid-run | Stop auto; manual `z/x/c/q` still work |
| Move/scan cap hit | Stop auto; do not issue another of that action |
| Type 1 (casual) | Never stop for “mission complete”; only a–c, `p`, `q`, or destroy |
| Unknown terrain char | Store and display; classify as wall only after impassable failure |
| `'X'` on MS_1 vs MS_2 | Do not globally hardcode `'X'` as danger from MS_1 costs; use MS_2 discovery + PDF roles |
| Destroy | `mc_is_mission_over`; leave auto; skip Close |
| No frontier left but goal unmet | Stop with “stuck / map exhausted” rather than infinite loop |
| Delay while demoing | `sleep` is OK; keep it interruptible later only if easy (U13) |

---

## 12. Trade-offs

| Decision | Choice | Why |
|----------|--------|-----|
| Auto uses `mc_*` only | Yes | One success/fail/map path |
| Interrupt keys during auto | `p` and `q` | Brief: runs until stop; operator can still abort |
| “ALL” completion | Frontier exhaustion + classified symbols | No public mission-% API (MS_1 B18) |
| Cost-aware routing | Prefer empirically cheaper known cells when choosing among equals | Meets “energy cost effective”; no baked-in table required — use observed deltas already in `stats` if available, else treat unknown as scan-first |
| Defaults for `o` | Conservative (see U10) | First run must be configurable; defaults only if Enter-on-empty is allowed |

---

## 13. Deliverables awareness

Course still expects program + makefile, reports, Excel discovery, demo slides at packaging time. This spec does not schedule Word/PPT.

---

## 14. Things easy to miss

1. Option **1** skips prompts **2–4**; option **10** (`t4 > 0`) skips **11–13**.  
2. Casual explore has **no** objective stop.  
3. Find vs reach: scan vs occupy.  
4. Find-all danger: locate, **do not** visit.  
5. Radius 2 is a decision, not only the `t` key.  
6. Display suppression: auto-pilot must **not** always print maps like current manual `handle_move` / `handle_scan`. Factor print vs act.  
7. `sleep` after prints so Agent T can read.  
8. Link all MS_1 `.o`; MS_2 folder has **no** extra objects.  

---

## 15. Unresolved decisions

| ID | Question | Default if unanswered |
|----|----------|------------------------|
| U8 | Default values if operator wants a “just start” `o`? | Stats after every action = **n**; maps after scan/move = **y**; all delays = **0**; energy floor = **1**; move/scan caps = **0** (disabled) |
| U9 | Do move/scan caps count **all** session actions or **auto-only**? | **All session** issued counts (`stats`) |
| U10 | Allow empty Enter to accept U8 defaults during `o`? | **Yes** — empty uses default for that prompt |
| U11 | May `o` run while auto is on? | **No** — must `p` off first |
| U12 | Must auto-pilot be interruptible **during** `sleep`? | Best-effort: sleep full seconds first; non-blocking stdin later if needed |
| U13 | How to classify end/danger if a new map uses different letters? | End = `'E'` only (PDF example). Danger = `'X'`. Walls = `'#'` plus impassable failures. Do **not** treat spawn or `'S'` as an end. |
| U14 | Energy-aware path: use live `stats` averages per scanned symbol vs simple BFS? | **Scan-first + BFS** first; add cheapest-known-cell tie-break if time |
| U15 | Should auto print the same tutor one-liner as manual when stats=y? | **Yes** (`ui_print_veh_data`) |

MS_1 U1–U7 remain: executable `VehExplorer`, external `script`/`tee` OK, grow map from (0,0), `-std=c11`.

---

## 16. Success definition (MS_2)

On WSL Ubuntu, after `make`:

1. MS_1 manual session still works on both sample `.dat` files.  
2. `o` prompts follow Appendix A skip rules (1 overrides 2–4; t4>0 overrides t5–t7).  
3. `p` runs auto-pilot; `p` again returns to manual loop.  
4. Auto-pilot performs scans/moves/shields without those keys, updates maps/stats, and stops for a–c and the matching mission objective.  
5. Type 2 stops after scanning one end; type 3 after occupying one; type 6 locates dangers without requiring visits.  
6. No decrypt, no forbidden Print*, no crash on shield-0 / missing file.  

---

## 17. Approval gate

Approved 2026-10-24. Unresolved U8–U15 use the defaults in §15.

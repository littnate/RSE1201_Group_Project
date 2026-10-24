# PROJECT.md — AI working notes (RSE1201 MS_2)

Read with `SPEC.md`. Prefer this file for hard constraints that are easy to violate.

## Tech stack

- Language: **C only** (`gcc`, target **Linux x86-64**)
- Primary env: **WSL2 Ubuntu**; must also build on Ubuntu 22.04 VM later
- Standard: `-std=c11` unless tutor says otherwise
- UI: **terminal CLI only** (no GUI, no ncurses unless later approved)
- Executable working name: `VehExplorer`

## Scope lock

- **MS_1 manual control stays.** Do not regress keys, maps, stats, quit report, or Close behaviour.
- Implement **Milestone 2 auto-pilot** as specified in `SPEC.md` (`o` config, `p` toggle, planner, stop criteria).
- `SPEC.md` is **approved**. Implement MS_2 slices in `BUILD_ORDER.md` (C0 onward).
- Keep UI / `mission_control` / `local_map` / `stats` separable; autopilot calls `mc_*` only.

## Folder structure

```
RSE1201_Group_Project/
  MS_1_files/              # READ-ONLY: SatComRelay.h, provided *.o, MS_1 .dat + PDF
  MS_2_files/              # READ-ONLY .dat/PDF: MS_2 brief + 2026_Test_Scenario_MS_2.dat
    pages/                 # derived PNG renders of the MS_2 PDF (OK to regenerate)
  VehExplorer/
    docs/                  # SPEC.md, PROJECT.md, BUILD_ORDER.md
    src/  inc/  obj/  bin/
    input-files/           # symlinks to scenario *.dat
    makefile
```

- Never edit course binaries or scenario payloads (`*.o`, `*.dat`, `SatComRelay.h` content, MS_2 `.dat` / PDF)
- Derived page PNGs are OK
- Makefile links `../MS_1_files/*.o` + `SatComRelay.h`; MS_2 folder has **no** extra `.o`

## Rubric reference

| Asset | Path |
|-------|------|
| MS_1 PDF | `MS_1_files/2026_RSE1201_GroupProject_MS1_Scenario.pdf` |
| MS_2 PDF | `MS_2_files/2026_RSE1201_GroupProject_MS2_Scenario.pdf` |
| MS_2 pages | `MS_2_files/pages/` |
| MS_2 index | `MS_2_files/INDEX.md` |
| MS_1 compliance | `docs/evidence/B18-rubric-compliance.md` |

## Integration rules (critical)

- **Only** call the public API in `MS_1_files/SatComRelay.h` (`SCRelay_Signal_Veh_*`)
- Auto-pilot goes through **`mission_control`**, not SatComRelay directly
- Link **all** provided MS_1 `*.o` files
- Pass encryption as `fileNeedsDecryption` to Init — **never** decrypt/edit `.dat` / `.txt`
- **Do not use** password / “Do not use!” helpers: `PrintEntireMap`, `PrintScannedMap`, `PrintExploredMap`, `PrintReport`
- Do not reverse-engineer, patch, or strip provided objects
- Do not call undocumented symbols from other `.o` files unless SPEC is updated

## Hardcoding ban

Must work on **unseen** scenarios without source changes:

- No hard-coded scenario filename, map size, spawn, full terrain alphabet, or energy/shield/scan cost tables
- Store terrain as returned `char` values
- Grow local map from relative coords (origin `(0,0)`)
- PDF examples `'E'` end, `'X'` danger, `'#'` perimeter are **starting roles after discovery**, not a claim that every map uses only those letters
- End points are **`'E'` only**. Spawn / `'S'` is not an end.
- Do not treat MS_1 passable `'X'` as danger on every file

## CLI contract

MS_1 keys unchanged. MS_2 adds:

| Key | Action |
|-----|--------|
| `o` | Auto-pilot options (Appendix A display/delays + stop a–c) |
| `p` | Toggle auto-pilot on/off (run `o` first if never configured) |

Option **1** skips prompts **2–4**. Delay **t4 > 0** skips **t5–t7**.

While auto is on, loop actions until stop; interrupt with `p` or `q`. Do not prompt `o` while auto is on (SPEC U11 default).

Mark current vehicle cell with underline `_` on map prints.

## Local state we own

API `VehDataType` = energy fields only. We track:

- explored vs scanned grids (separately)
- relative position
- scan radius
- commands issued vs successful (move/scan)
- auto-pilot options + on/off flag
- enough belief to stop on mission objectives

## Naming / modules

- `main` — stages
- `ui_cli` — prompts, keys, printing (gate auto prints)
- `mission_control` — key/auto → SatComRelay; `mc_set_scan_radius`
- `local_map` — explored/scanned grids + planner queries
- `stats` — counters + summary report
- `util_input` — safe input
- `autopilot` — options, goal tests, next-action decision, run loop

Handlers may use a `handle` prefix. Keep names boring.

## Libraries

**Allowed:** libc, `stdbool.h`, `unistd.h` (`sleep`, `dup`), what `SatComRelay.h` already needs.

**Avoid unless explicitly approved:** ncurses, SDL, networking libs, crypto libs, third-party map engines, C++.

## Coding standards

- `-Wall -Wextra`; fix warnings rather than silencing
- Early returns; check API failure (`'\0'` scan, move success from library text)
- No `gets`; bounded input
- Factor **act** vs **print** so auto-pilot can suppress maps/stats
- No secrets, no tutor passwords in repo

## Build / run

```bash
cd VehExplorer
make
./bin/VehExplorer
```

MS_2 sample path: `input-files/2026_Test_Scenario_MS_2.dat` (encrypted `y`).

## Testing strategy

1. Regression: short MS_1 manual session on `2026_Basic_Scenario_MS_1.dat`
2. `o` skip-logic: option 1 = y must not ask 2–4; t4 > 0 must not ask t5–t7
3. `p` on/off with type 1 (casual) and a low move cap
4. Type 2 (find 1 end) and type 3 (reach 1 end) on MS_2 `.dat`
5. Type 6 does not require stepping on danger
6. Second scenario without code changes
7. Transcript via external `script`/`tee` OK
8. Record end/danger/perimeter symbols in discovery notes — do not bake a full cost table into source

## Do not

- Drop MS_1 manual control
- Implement GUI
- Modify course binaries / `.dat` / header content
- Decrypt scenarios
- Hardcode world data / cost tables
- Use forbidden Print* APIs
- Add dependencies “for convenience”
- Contradict `SPEC.md` unresolved defaults without asking

# PROJECT.md — AI working notes (RSE1201 MS_1)

Read with `SPEC.md`. Prefer this file for hard constraints that are easy to violate.

## Tech stack

- Language: **C only** (`gcc`, target **Linux x86-64**)
- Primary env: **WSL2 Ubuntu**; must also build on Ubuntu 22.04 VM later
- Standard: `-std=c11` unless tutor says otherwise
- UI: **terminal CLI only** (no GUI, no ncurses unless later approved)
- Executable working name: `VehExplorer`

## Scope lock

- Implement **Milestone 1 (manual control)** only
- Do **not** implement MS_2 auto-pilot yet
- Keep UI / mission actions / local map-stats separable for later MS_2

## Folder structure

```
kor/
  docs/                    # SPEC.md, PROJECT.md, BUILD_ORDER.md
  rubrics/                 # READ-ONLY course materials — do not modify binaries
    docs/                  # briefs, rubrics spreadsheet, forms
    MS_1/                  # SatComRelay.h, provided *.o, sample *.dat, scenario PDF
      2026_RSE1201_GroupProject_MS1_Scenario_pages/  # PNG renders of scenario PDF (reference)
    MS_2/                  # leave alone until MS_2
  src/                     # our *.c (create when implementing)
  inc/                     # our *.h
  obj/                     # our *.o
  bin/                     # VehExplorer
  input-files/             # copies/links of scenario *.dat for runs
  makefile
```

- Never edit course binaries under `rubrics/` (`*.o`, `*.dat`, `SatComRelay.h` content)
- Adding **derived reference assets** (e.g. PDF page PNGs under `..._Scenario_pages/`) is OK; do not alter provided objects or scenarios
- Copy or reference `rubrics/MS_1/*.o` + `SatComRelay.h` from the makefile; do not duplicate-edit binaries

## Rubric reference (MS_1 scenario PDF)

The official behaviour spec is image-based — use the rendered pages for audits:

| Asset | Path |
|-------|------|
| Source PDF | `rubrics/MS_1/2026_RSE1201_GroupProject_MS1_Scenario.pdf` |
| Page images (58) | `rubrics/MS_1/2026_RSE1201_GroupProject_MS1_Scenario_pages/page-NN.png` |
| Page index + regen | `rubrics/MS_1/2026_RSE1201_GroupProject_MS1_Scenario_pages/INDEX.md` |
| Compliance audit | `docs/evidence/B18-rubric-compliance.md` |

Regenerate PNGs with the Python snippet in `INDEX.md` (requires `.venv-pdf` + `pymupdf`; venv is gitignored).

## Integration rules (critical)

- **Only** call the public API in `rubrics/MS_1/SatComRelay.h` (`SCRelay_Signal_Veh_*`)
- Link **all** provided MS_1 `*.o` files (not just `SatComRelay.o`)
- Pass encryption as `fileNeedsDecryption` to `Init` — **never** decrypt/edit `*.dat` / `*.txt` scenarios yourself
- **Do not use** password / “Do not use!” helpers: `PrintEntireMap`, `PrintScannedMap`, `PrintExploredMap`, `PrintReport`
- Do not reverse-engineer, patch, or strip provided objects
- Do not call undocumented symbols from other `.o` files unless SPEC is updated

## Hardcoding ban

Must work on **unseen** scenarios without source changes:

- No hard-coded scenario filename, map size, spawn, terrain alphabet, or energy/shield/scan cost tables
- Store terrain as returned `char` values; treat unknown symbols as data, not errors
- Grow local map from relative coords (default origin `(0,0)`); unknown cells = unscanned

## CLI contract (match scenario PDF)

| Key | Action |
|-----|--------|
| `w` `s` `a` `d` | Move N S W E |
| Numpad `1`–`9` except `5` | Scan SW S SE W E NW N NE |
| `t` | Toggle scan radius 1 ↔ 2 (`*2` APIs when radius 2) |
| `i` | Allocate to shields (reject ≤0 / insufficient energy) |
| `z` | Vehicle status/statistics |
| `c` | Scanned map |
| `x` | Explored/travelled map |
| `v` | Help |
| `q` | Report → `Close` → exit |

Startup prompts: scenario path, encrypted y/n, randomize start y/n, mission type **1–8**.

Mark current vehicle cell with underline `_` on map prints.

## Local state we own

API `VehDataType` = energy fields only. We must track:

- explored vs scanned grids (separately)
- relative position
- scan radius
- commands issued vs successful (move/scan)
- enough deltas for quit summary / discovery notes

## Naming / modules

Prefer small modules over one giant `main.c`:

- `main` — stages
- `ui_cli` — prompts, keys, printing
- `mission_control` — key → SatComRelay calls
- `local_map` — explored/scanned grids
- `stats` — counters + summary report
- `util_input` — safe input

Event-style handlers may use a `handle` prefix if helpful; keep names boring and consistent.

## Libraries

**Allowed:** libc, `stdbool.h`, what `SatComRelay.h` already needs.

**Avoid unless explicitly approved:** ncurses, SDL, networking libs, crypto libs, third-party map engines, C++.

## Coding standards (this project)

- `-Wall -Wextra`; fix warnings rather than silencing
- Early returns; check API failure (`'\0'` scan, energy unchanged / messages for failed moves)
- No `gets`; bounded input
- Input validation is required hygiene (not an “extra feature”)
- No secrets, no tutor passwords in repo

## Build / run

- `makefile` builds `bin/VehExplorer` from `src/` + links `rubrics/MS_1/*.o`
- Run on Linux x86-64 only (objects are ELF64 x86-64)
- Scenario path: user-provided; keep samples under `input-files/` for convenience

## Testing strategy

1. Manual session against `rubrics/MS_1/2026_Basic_Scenario_MS_1.dat` (encrypted = yes)
2. Verify maps/stats after move, failed move, scan r=1, scan r=2, bad shield alloc
3. Re-run with a **different** scenario path (e.g. MS_2 `.dat` only as smoke input if Init accepts it) without code changes
4. Record terrain/energy discoveries for the Excel report — do not bake discoveries into source as constants
5. No automated unit-test framework required unless added later
6. Session transcript: external `script`/`tee` OK for MS_1; in-program log optional later

## Deployment / delivery

- Deliverable = local `gcc` binary + sources + makefile (course zip layout later)
- Not a web/cloud app
- Do not invent `TeamN-MS_1` submission names until packaging

## Do not

- Implement GUI or auto-pilot
- Modify course binaries under `rubrics/` (`*.o`, `*.dat`, header content)
- Decrypt scenarios
- Hardcode world data
- Use forbidden Print* APIs
- Add dependencies “for convenience”
- Write implementation plans/code that contradict `SPEC.md` unresolved defaults without asking

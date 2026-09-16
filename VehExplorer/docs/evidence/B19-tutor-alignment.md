# B19 — Tutor UI alignment (scenario PDF p.5+)

Date: 2026-09-16  
Reference pages: `rubrics/MS_1/2026_RSE1201_GroupProject_MS1_Scenario_pages/`  
Replay input: `docs/evidence/tutor-replay-input.txt`  
Replay output: `docs/evidence/tutor-replay-actual.txt`

## Scope decisions (from planning)

- Match professor **flow / features**, not absolute energy numbers (different `.dat`).
- Keep shield-down move guard (safety; not required by deliverables).
- Quit report follows **deliverables sheet** (energy, terrain counts, move/shield/scan energy tables) — **no** mission completion % (tutor-only / no public API).
- Clean spelling (do not copy PDF typos).

## Changes made

| Area | Change |
|------|--------|
| Startup prompts | Scenario / encrypted / randomize wording closer to PDF p.5 |
| Config echo | `fileName : …, isFileEncrypted : …, randomizeStartPosition : …` |
| Welcome + help | Printed after init; includes `c`/`x` and `_` note |
| Continue gate | `Press 'y' key and hit Enter to continue!` |
| `z` | Header `Vehicle Status / Statistics :` + energy + issued/success counters (deliverable item 6) |
| `x` / `c` | `Currently Explored/Scanned Terrain Layout :` + map banners |
| Map banners | `-- Print Explored Map --` / `-- Print Scanned Map --` |
| Shield prompt | Includes `(1 - max)` range; `Error:` prefix |
| Key `5` / unknown | `Invalid Command : 'X', please try again!` |
| Quit | `Exploration Report` with scan/move totals, energy block, counters, terrain + energy tables |
| Post-init energy | No duplicate print (library already prints one-liner) |

## Replay checklist (patterns vs PDF)

| Pattern | Result |
|---------|--------|
| Startup prompts + config echo | PASS |
| Init + energy one-liner | PASS |
| Welcome + usage + Press `y` | PASS |
| `z` status header | PASS |
| `x` / `c` layout titles + map banners | PASS |
| Shield insufficient / invalid / success | PASS |
| `t` toggle messages | PASS |
| Invalid `5` | PASS |
| Scan → symbol → energy → scanned map | PASS |
| Move → library msg → energy → explored map | PASS |
| Quit Exploration Report + Close | PASS |
| Second scenario still works | PASS (smoke) |

## Intentionally different from tutor PDF

- Energy / terrain values (our `.dat`, not `Scenario05.dat`)
- No `(TUTOR Ver)` / “Mr Tian” branding
- No mission completion % block (not in MS_1 deliverables; no public API)
- Shield-down move refused locally (demo stability)
- `z` still shows issued/success counters (required by deliverable item 6)

# B15 — Second-scenario (no hardcoding) check

Date: 2026-09-16  
Binary: `bin/VehExplorer` (unchanged sources between scenarios)

## Hardcoding audit (source)

- Scenario path: prompted only (`ui_prompt_startup`); no baked-in `.dat` name in `src/`.
- Map size: `local_map` grows from relative coords; no fixed width/height.
- Terrain alphabet / costs: stored as returned `char`s; quit report tallies observed symbols only (no cost tables).
- Convenience symlink only: `input-files/2026_Test_Scenario_MS_2.dat` → `rubrics/MS_2/...` (not compiled into the binary).

## Session A — MS_1 baseline (sanity)

Path entered at prompt: `input-files/2026_Basic_Scenario_MS_1.dat`  
Encrypted `y`, randomize `n`, mission `8`.

- Init energy: **1686088**
- `z` / `q` / Close: OK

## Session B — second scenario (B15 deliverable)

Path entered at prompt: `input-files/2026_Test_Scenario_MS_2.dat`  
Encrypted `y`, randomize `n`, mission `8`.  
Full transcript: `docs/evidence/B15-second-scenario-session.txt`

Commands exercised: shield alloc → status → scan N/E (r=1) → toggle r=2 → scan N → move N → move E (blocked: shields depleted after damaging cell) → `x` → `c` → `z` → `q`.

Results:

| Check | Result |
|-------|--------|
| Init on different `.dat` | OK — Init energy **2047936** (≠ MS_1) |
| Successful move | 1 / 2 issued |
| Successful scans | 3 / 3 (terrains `X`, `f`, `~`) |
| Explored map (`x`) | Shows visited cells + vehicle |
| Scanned map (`c`) | Distinct view with scanned symbols |
| Quit summary + Close | Clean exit 0 |

## Verdict

Same binary works on a second scenario path with **no source changes**. Behavior differs from MS_1 (energy budget, terrain layout) and is driven only by the path typed at startup.

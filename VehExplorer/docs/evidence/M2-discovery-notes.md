# M2-C0 — Discovery notes (`2026_Test_Scenario_MS_2.dat`)

Date: 2026-10-24  
Path: `input-files/2026_Test_Scenario_MS_2.dat`  
Encrypted `y`, randomize `n`  
Binary: MS_1 `bin/VehExplorer` (pre-autopilot)

Init energy observed: **511984**

## Terrain symbols (wander + spawn scans)

| Symbol | Role used by auto-pilot |
|--------|-------------------------|
| `#` | Perimeter / impassable (PDF example). 15 scans in a short wander. |
| `X` | Danger (PDF example). Common; spawn has `X` immediately **north**. Do not step on it for type 6. |
| `E` | End point (PDF). Present on this map but **not** at spawn; type 2 finds it after exploring. |
| `S` | Extra letter (spawn can be `'S'`). **Not** an end point. |
| `f` `w` `j` `~` ` ` | Ordinary passable |

Scan energy on this file matched MS_1: cardinal r1=90 / r2=180; diagonal r1=360 / r2=720.

Failed `#` bumps were not required to classify `#` — scans already return the symbol.

## Library messages

No `MISSION OVER` / `DESTROYED` in these trials (shields allocated before moves).

## Implication for planner

Start roles: end = `'E'` only; danger = `'X'`; wall = `'#'` or a failed move into an unknown cell. `'S'` is ordinary terrain for mission goals.

# B16 — Discovery notes (MS_1 default scenario)

Date: 2026-09-16  
Scenario: `input-files/2026_Basic_Scenario_MS_1.dat` (encrypted `y`, randomize `n`, mission `8`)  
Binary: `bin/VehExplorer`  
Init energy (all runs): **1686088**

These notes feed the course Excel / Word “Testing and Discovery” section.  
**Nothing here is hard-coded into the program** — values were read from live `VehDataType` deltas and quit-summary tables.

Transcripts (reproducible key sequences):

| Session | File | Purpose |
|---------|------|---------|
| A | `B16-session-A-scans.txt` | All 8 dirs × radius 1 then 2 |
| B | `B16-session-B-moves.txt` | Scan-then-move survey |
| C | `B16-session-C-hazard.txt` | Impassable `#` failed move |
| D | `B16-session-D-shield-gate.txt` | Move blocked when shields = 0 |
| E | `B16-session-E-east-M.txt` | Move onto `M`; more r=2 scans |

---

## 1. Terrain symbols observed

From successful scans on the default map (sessions A–E):

| Symbol | Role (inferred from behavior) |
|--------|-------------------------------|
| `f` | Passable; moderate move + shield cost |
| `w` | Passable |
| `h` | Passable; higher shield cost |
| `j` | Passable |
| `M` | Passable; highest move + shield among sampled passables |
| ` ` (space) | Passable; lowest move/shield among sampled |
| `X` | Passable; high shield cost |
| `#` | **Impassable** — library prints `INVALID / IMPASSABLE LOCATION!`; vehicle stays put |
| `.` | Appears on explored map at spawn / vehicle underline context (local display); not confirmed as a distinct scanned terrain char in these trials |

No fixed alphabet is assumed in code; unknown future symbols should be stored/displayed as returned.

---

## 2. Shield allocation

Controlled check (session A / D):

- Allocate amount **A** → `totEnergyUsed` += A, `currEnergy` -= A, `currShieldEnergy` += A.
- `totShieldEnergyUsed` does **not** increase on allocate (only on damaging moves / failed impassable attempts that still burn shield).
- Example: `i` → `100000` → left energy 1586088, shield left 100000.

Program gate (session D): with shield **0**, move is refused locally (`Cannot move: shields are down…`) and **no** energy change. After allocating 1000, move onto `f` succeeds (shield drops by 954).

---

## 3. Scan energy (direction / radius)

**Finding:** scan cost depends on **direction class** (cardinal vs diagonal) and **radius**, **not** on the terrain symbol returned. Same cost for `f`, `#`, `X`, space, etc.

Session A (one sample each), deltas of `totEnergyUsed` (shield scan delta always 0):

| Direction | Radius 1 | Radius 2 |
|-----------|----------|----------|
| N / S / E / W (cardinal) | **90** | **180** (= 2×) |
| NW / NE / SW / SE (diagonal) | **360** | **720** (= 2×) |

Diagonal ≈ 4× cardinal at the same radius; radius 2 ≈ 2× radius 1 for the same direction class.

Quit report (session A) matches these totals with `Samples = 1` per (dir, radius).

---

## 4. Move / shield energy by destination terrain

Method: **scan** the adjacent cell first, then **move** into it; delta = change in `totEnergyUsed` / `totShieldEnergyUsed`.

| Destination | Move ΔE | Shield ΔS | Evidence |
|-------------|---------|-----------|----------|
| ` ` (space) | 610 | 814 | Session B (3 samples → avg in quit table) |
| `w` | 638 | 909 | Session B |
| `f` | 645 | 954 | Sessions B, D, E (repeated) |
| `j` | 686 | 901 | Session B |
| `X` | 694 | 1935 | Session B |
| `h` | 696 | 2043 | Sessions B, E |
| `M` | 747 | 2117 | Session E (3 identical samples) |

**Impassable `#` (session C):** failed move north onto `#` still consumed **694** move energy and **1935** shield (same numbers as a successful move onto `X` in these trials). Vehicle did not change cell; explored map unchanged; counters: moves issued +1, successful moves unchanged.

**Caveat for quit-report terrain rows:** move energy is attributed to the **previously scanned** destination symbol. Moves without a prior scan of that cell are logged under `'?'` and should not be used for Excel “per terrain” costs.

---

## 5. Answers to course discovery prompts

From deliverable sheet § Testing and Discovery:

1. **What terrain symbols were there?**  
   `f`, `w`, `h`, `j`, `M`, ` ` (space), `X`, `#` (impassable), plus spawn/display `.` / unscanned `?` in local maps.

2. **Energy to move onto each terrain?**  
   See §4 table (empirical on default scenario).

3. **Shield energy expenditure per terrain?**  
   Same table; shield burns on successful moves onto damaging cells and also on the failed `#` attempt observed.

4. **Scan energy — depends on terrain?**  
   **No** in these trials. Depends on cardinal vs diagonal and radius 1 vs 2 (§3).

5. **Other operational notes**  
   - Allocate shields before exploring; many cells burn shield.  
   - Failed impassable moves can still cost energy/shield.  
   - Scan-before-move is required for trustworthy per-terrain costing.

---

## 6. How to re-verify

From repo root:

```bash
# Session A (scans) — key sequence embedded in transcript header path
printf '%s\n' \
  'input-files/2026_Basic_Scenario_MS_1.dat' y n 8 \
  i 100000 z \
  8 2 4 6 7 9 1 3 t \
  8 2 4 6 7 9 1 3 z q \
| ./bin/VehExplorer | tee /tmp/B16-A.txt

# Compare scan table in quit report to §3.
```

Repeat session E for `M` costs (`i` 150000, then `6`/`d` pairs) and session C for impassable `#` (`8`/`w` twice then `8`/`w` onto `#`).

# VehExplorer (RSE1201 — Milestone 1)

C terminal program for manual control of the exploration vehicle via the provided `SatComRelay` API.

This is **not** a Node/Python app. There is no `npm install`.

## Prerequisites

- **OS:** Linux **x86-64** (Ubuntu / WSL2 Ubuntu recommended; matches course VM)
- **Toolchain:** `gcc` (`build-essential` on Ubuntu)

```bash
sudo apt update
sudo apt install build-essential
```

The presentation library `../special-reveal/libsatcom.so` is **ELF 64-bit x86-64**. It will not link on macOS, native Windows, or ARM (e.g. Apple Silicon without an x86-64 Linux environment).

This folder lives in the group repo. `bin/VehExplorer.run` is already built. Start it only from `VehExplorer/`:

```bash
cd VehExplorer
./bin/VehExplorer.run
```

`libsatcom.so` in this folder is a symlink to `../special-reveal/libsatcom.so`.

After a source change, rebuild with this one command from `VehExplorer/` (`bin/` already exists):

```bash
gcc -Iinc -I../MS_1_files src/autopilot.c src/local_map.c src/main.c src/mission_control.c src/stats.c src/ui_cli.c src/util_input.c src/veh_action.c ./libsatcom.so -o bin/VehExplorer.run -Wl,-rpath=.
```

**Run constraint:** `-Wl,-rpath=.` sets the runtime search path to the current directory, and the binary asks the loader for `./libsatcom.so`. That path is resolved from the directory you are in, which must be `VehExplorer/` (the folder that contains the symlink). Starting `bin/VehExplorer.run` from any other directory, including `/tmp`, exits 127 with `cannot open shared object file`. `cd VehExplorer` first, then run `./bin/VehExplorer.run`.

At the startup prompts, presentation sample paths are:

```text
input-files/New_2026_Basic_Scenario_MS_1.dat
input-files/New_2026_Test_Scenario_MS_2.dat
input-files/Scenario05.dat
```

Encrypted: `y`. Then choose randomize / mission type as usual.

`bin/VehExplorer.run` is a build output (`*.run` is gitignored). A fresh clone does not contain that binary, so run the `gcc` command above once, then `./bin/VehExplorer.run`.

## What must be in the repo

For someone else to build after clone, these must be present (committed):

- `VehExplorer/src/`, `inc/`, `libsatcom.so` symlink
- `special-reveal/libsatcom.so` (course shared library)
- `MS_1_files/SatComRelay.h` (same header; the old `*.o` files are no longer linked)
- Scenario samples under `VehExplorer/input-files/` and `special-reveal/*.dat`

## What you do *not* need

| Thing | Needed to play? |
|-------|------------------|
| `npm` / Node | No |
| Python / pip | No |
| `.venv-pdf` / PyMuPDF | No — only for regenerating PDF page PNGs of the scenario brief |

## Quick controls (MS_1)

| Key | Action |
|-----|--------|
| `w` `s` `a` `d` | Move N / S / W / E |
| Numpad `1`–`9` (except `5`) | Scan 8 directions |
| `t` | Toggle scan radius 1 ↔ 2 |
| `i` | Allocate energy to shields |
| `z` | Vehicle status |
| `c` | Scanned map |
| `x` | Explored / travelled map |
| `v` | Help |
| `q` | Summary report → close → quit |
| `o` | Configure auto-pilot options |
| `p` | Toggle auto-pilot on / off |

## Docs

| File | Purpose |
|------|---------|
| `docs/SPEC.md` | Product / MS_2 requirements (MS_1 remains) |
| `docs/PROJECT.md` | Hard constraints for implementers |
| `docs/BUILD_ORDER.md` | Build slices & progress |
| `docs/NAME_CHANGES.md` | Old-to-new variable and function names, by file |
| `TESTING.md` | Short manual test reminder |

## Scope note

Milestone 1 = **manual control**. Milestone 2 = **auto-pilot** (`o` configure, `p` toggle) — see `docs/SPEC.md`.

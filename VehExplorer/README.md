# VehExplorer (RSE1201 — Milestone 1)

C terminal program for manual control of the exploration vehicle via the provided `SatComRelay` API.

This is **not** a Node/Python app. There is no `npm install`.

## Prerequisites

- **OS:** Linux **x86-64** (Ubuntu / WSL2 Ubuntu recommended; matches course VM)
- **Toolchain:** `gcc` and `make` (`build-essential` on Ubuntu)

```bash
sudo apt update
sudo apt install build-essential
```

The course-provided `../MS_1_files/*.o` files are **ELF 64-bit x86-64**. They will not link on macOS, native Windows, or ARM (e.g. Apple Silicon without an x86-64 Linux environment).

This folder lives in the group repo. From the repo root:

```bash
cd VehExplorer
make
./bin/VehExplorer
```

At the startup prompts, a typical sample path is:

```text
input-files/2026_Basic_Scenario_MS_1.dat
```

Encrypted: `y` (for the sample scenario). Then choose randomize / mission type as usual.

### Useful make targets

| Command   | What it does                          |
|-----------|----------------------------------------|
| `make`    | Build `bin/VehExplorer`                |
| `make clean` | Remove `obj/` and `bin/VehExplorer` |

`bin/` and `obj/` are build outputs (gitignored). Every clone must run `make`.

## What must be in the repo

For someone else to build after clone, these must be present (committed):

- `VehExplorer/src/`, `inc/`, `makefile`
- `MS_1_files/*.o` and `MS_1_files/SatComRelay.h` (course library — one level up)
- Scenario samples under `VehExplorer/input-files/`

The group `.gitignore` already keeps `MS_1_files/*.o` (it ignores other `*.o` build junk).

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

## Docs

| File | Purpose |
|------|---------|
| `docs/SPEC.md` | Product / MS_1 requirements |
| `docs/PROJECT.md` | Hard constraints for implementers |
| `docs/BUILD_ORDER.md` | Build slices & progress |
| `TESTING.md` | Short manual test reminder |

## Scope note

Milestone 1 = **manual control only**. Auto-pilot (MS_2) is out of scope for this tree until scheduled separately.

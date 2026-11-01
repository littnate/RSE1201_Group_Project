# v2 Student Deep-Dive Guide 7: `local_map.h` & `local_map.c`

> [!NOTE]
> **Beginner-Friendly Visual Edition (`nathan` branch):** Covers [local_map.h](file:///c:/Users/nathanyu/Documents/GitHub/RSE1201_Group_Project/VehExplorer/inc/local_map.h) (lines 1–53) and all 614 lines of [local_map.c](file:///c:/Users/nathanyu/Documents/GitHub/RSE1201_Group_Project/VehExplorer/src/local_map.c), with **3-layer map diagrams**, a **worked negative-coordinate `cell_index` visual**, and a **before/after grid expansion diagram** so you can literally *see* how the vehicle builds its map in RAM! Every C term is re-explained inline right inside each section.

---

## 1. Visual Mental Model 1: How Do You Draw a Map When You Don't Know Where You Spawned?

When `libsatcom.so` drops your rover into an arena (especially with `randomizeStartPosition = y`), **it never tells you the map's width, height, or your `(row, col)`!**

`local_map.c` solves this like drawing a map on expandable graph paper starting from **wherever you woke up as `(0, 0)`**:
- **Spawn = `(0, 0)`**
- **West = `x - 1`**, **East = `x + 1`**
- **North = `y - 1`** (upward on screen), **South = `y + 1`** (downward on screen)

### Why Does `LocalMap` Store **3 Separate Layers** (`explored`, `scanned`, `blocked`)?

Picture 3 transparent plastic sheets stacked on top of each other over a 3×3 area around `(0, 0)`:

```text
Layer 1: explored[] (bool)     Layer 2: scanned[] (char)      Layer 3: blocked[] (bool)
(Did the rover WALK here?)     (What did RADAR see here?)     (Wall '#' or out-of-range?)
   x=-1  x=0  x=+1                x=-1  x=0  x=+1                x=-1  x=0  x=+1
  +----+----+----+               +----+----+----+               +----+----+----+
  |  0 |  1 |  0 | y=-1          | '#'| 'h'| 'E'| y=-1          |  1 |  0 |  0 | y=-1
  +----+----+----+               +----+----+----+               +----+----+----+
  |  0 |  1 |  0 | y= 0          | '\0'| '\0'| '.'| y= 0        |  0 |  0 |  0 | y= 0
  +----+----+----+               +----+----+----+               +----+----+----+

Now look at how character_to_print() combines them for the 2 user map keys!
  Press 'x' (Explored Map — ONLY shows tiles where explored == 1):
      Row y=-1:   [   ] [ h ] [   ]   <-- (0,-1) was walked on & scanned as 'h' (rover '_' if here!)
      Row y= 0:   [   ] [ . ] [   ]   <-- (0, 0) was walked on WITHOUT scanning -> shows '.'!

  Press 'c' (Scanned Map — shows ALL tiles where scanned != '\0'):
      Row y=-1:   [ # ] [ h ] [ E ]   <-- Shows wall '#', hill 'h', and End 'E' even before walking!
      Row y= 0:   [   ] [ _ ] [ . ]   <-- Rover '_' underlined at current position!
```

---

## 2. Visual Mental Model 2: Turning Negative `(x, y)` into a 1D Array Index (`cell_index`)!

C arrays cannot have negative indices like `grid[-1][-1]`! So `local_map.c` stores a 2D rectangle `[min_x..max_x] × [min_y..max_y]` inside a **flat 1D array** using `cell_index(x, y)` on line 46:

$$\text{cell\_index}(x, y) = (y - \text{min\_y}) \times \text{width} + (x - \text{min\_x})$$

Trace it with real numbers! Suppose `min_x = -1, max_x = +1` (`width = 3`) and `min_y = -1, max_y = 0` (`height = 2`):

```text
2D Relative Grid (width = 3, height = 2):
             x = -1            x = 0             x = +1
        +-----------------+-----------------+-----------------+
  y=-1  | (-1,-1) -> idx 0| ( 0,-1) -> idx 1| (+1,-1) -> idx 2|   (y - (-1)) = 0
        +-----------------+-----------------+-----------------+
  y= 0  | (-1, 0) -> idx 3| ( 0, 0) -> idx 4| (+1, 0) -> idx 5|   (y - (-1)) = 1
        +-----------------+-----------------+-----------------+
          (x - (-1)) = 0    (x - (-1)) = 1    (x - (-1)) = 2

Flattened 1D Heap Array in RAM (size = width * height = 6):
  Index:    [0]       [1]       [2]       [3]       [4]       [5]
         +---------+---------+---------+---------+---------+---------+
         | (-1,-1) | ( 0,-1) | (+1,-1) | (-1, 0) | ( 0, 0) | (+1, 0) |
         +---------+---------+---------+---------+---------+---------+
```
Check `(0, 0)`: `(0 - (-1)) * 3 + (0 - (-1)) = 1 * 3 + 1 = 4` → **Slot `[4]`!** Subtracting `min_x` and `min_y` shifts the top-left corner `(-1, -1)` cleanly to slot `[0]`!

---

## 3. Header Walkthrough: `local_map.h` (Lines 1–53)

### Full Header ([local_map.h](file:///c:/Users/nathanyu/Documents/GitHub/RSE1201_Group_Project/VehExplorer/inc/local_map.h#L1-L53))

```c
#ifndef LOCAL_MAP_H
#define LOCAL_MAP_H

#include <stdbool.h>

/* Relative map grown from successful moves and scans.
 * Spawn starts at (0, 0); North decreases y and East increases x. */

#define LOCAL_MAP_TERRAIN_UNKNOWN '?'

typedef struct {
    char terrain;
    unsigned scanned;
    unsigned explored;
} LocalMapTerrainCount;

void local_map_init(void);
void local_map_reset(void);

int local_map_get_x(void);
int local_map_get_y(void);

/* Mark current cell explored (for example, at spawn). */
void local_map_mark_explored_here(void);

/* Step vehicle position by (dx, dy) and mark the destination explored. */
void local_map_move_success(int dx, int dy);

/* Store scanned terrain symbol at offset (dx, dy) from the vehicle. */
void local_map_record_scan(int dx, int dy, char terrain);

/* Return known terrain at (x, y) or offset (dx, dy), or '?'. */
char local_map_peek_terrain(int x, int y);
char local_map_peek_terrain_at_offset(int dx, int dy);

bool local_map_is_explored(int x, int y);
bool local_map_is_blocked(int x, int y);
void local_map_mark_blocked(int x, int y);
void local_map_mark_blocked_at_offset(int dx, int dy);

void local_map_get_bounds(int *min_x, int *max_x, int *min_y, int *max_y);
bool local_map_has_grid(void);

unsigned local_map_count_scanned(char terrain);

/* Fill out[] with scanned and explored counts per discovered terrain symbol. */
unsigned local_map_terrain_counts(LocalMapTerrainCount *out, unsigned capacity);

/* Print the explored ('x') or scanned ('c') map view. */
void local_map_print_explored(void);
void local_map_print_scanned(void);

#endif /* LOCAL_MAP_H */
```

#### Every Term Explained Inline
- **`#ifndef LOCAL_MAP_H` / `#define LOCAL_MAP_H` / `#endif`:** Include guard preventing duplicate declarations.
- **`#define LOCAL_MAP_TERRAIN_UNKNOWN '?'`:** Returned by `local_map_peek_terrain` whenever a cell has not been scanned yet.
- **`unsigned`:** Shorthand for `unsigned int` (non-negative whole number `>= 0`).
- **Two Coordinate Styles:** Functions ending in `_at_offset(dx, dy)` measure from the **rover's current `(x, y)`** (handy for `veh_action.c`); functions taking `(x, y)` measure from **spawn `(0, 0)`** (handy for `autopilot.c` BFS!).

---

## 4. Source Walkthrough: `local_map.c` (Lines 1–614)

### Chunk A: Constants, `LocalMap` Struct, Free & `cell_index` ([local_map.c](file:///c:/Users/nathanyu/Documents/GitHub/RSE1201_Group_Project/VehExplorer/src/local_map.c#L1-L55))

```c
#include "local_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOCAL_MAP_NOT_SCANNED '\0'
#define LOCAL_MAP_VISITED_MARK '.'
#define LOCAL_MAP_VEHICLE_MARK '_'
#define LOCAL_MAP_CELL_WIDTH 3

/* Stores current vehicle coordinates, bounding box, and three cell layers. */
typedef struct {
    int x;
    int y;
    int min_x;
    int max_x;
    int min_y;
    int max_y;
    int width;
    int height;
    bool *explored;
    char *scanned;
    bool *blocked;
} LocalMap;

static LocalMap local_map;

/* Free the three heap arrays and clear grid dimensions. */
static void free_map_arrays(void)
{
    free(local_map.explored);
    free(local_map.scanned);
    free(local_map.blocked);
    local_map.explored = NULL;
    local_map.scanned = NULL;
    local_map.blocked = NULL;
    local_map.width = 0;
    local_map.height = 0;
}

/* Convert 2D map coordinates (x, y) into a 1D row-major array index. */
static int cell_index(int x, int y)
{
    return (y - local_map.min_y) * local_map.width + (x - local_map.min_x);
}

/* Check whether (x, y) falls inside the currently allocated rectangle. */
static bool in_bounds(int x, int y)
{
    return x >= local_map.min_x && x <= local_map.max_x &&
           y >= local_map.min_y && y <= local_map.max_y;
}
```

#### Every Term Explained Inline
- **`static LocalMap local_map;`:** Private file-scope struct holding the rover's live `(x, y)`, bounding box `min_x..max_y`, `width`/`height`, and the 3 heap pointers (`explored`, `scanned`, `blocked`). Automatically zero-initialized at startup!
- **`LOCAL_MAP_NOT_SCANNED` (`'\0'`):** Because `calloc` fills new memory with byte `0` (`'\0'`), every newly allocated cell in `scanned[]` automatically starts out marked "not scanned yet"!

---

### Chunk B: `grow_map_to_include` — Expanding the Grid Dynamically ([local_map.c](file:///c:/Users/nathanyu/Documents/GitHub/RSE1201_Group_Project/VehExplorer/src/local_map.c#L56-L153))

```c
/* Expand the map rectangle to include (x, y), copying existing cell values. */
static bool grow_map_to_include(int x, int y)
{
    int new_min_x, new_max_x, new_min_y, new_max_y, new_w, new_h;
    bool *new_explored;
    char *new_scanned;
    bool *new_blocked;
    int yy;

    /* First cell ever recorded: allocate a 1x1 grid at (x, y) */
    if (local_map.explored == NULL) {
        local_map.min_x = x;
        local_map.max_x = x;
        local_map.min_y = y;
        local_map.max_y = y;
        local_map.width = 1;
        local_map.height = 1;
        local_map.explored = calloc(1, sizeof(*local_map.explored));
        local_map.scanned = calloc(1, sizeof(*local_map.scanned));
        local_map.blocked = calloc(1, sizeof(*local_map.blocked));
        return local_map.explored != NULL && local_map.scanned != NULL &&
               local_map.blocked != NULL;
    }

    /* Nothing to do if (x, y) is already within the current bounds */
    if (in_bounds(x, y)) {
        return true;
    }

    /* Compute expanded rectangle limits */
    if (x < local_map.min_x) { new_min_x = x; } else { new_min_x = local_map.min_x; }
    if (x > local_map.max_x) { new_max_x = x; } else { new_max_x = local_map.max_x; }
    if (y < local_map.min_y) { new_min_y = y; } else { new_min_y = local_map.min_y; }
    if (y > local_map.max_y) { new_max_y = y; } else { new_max_y = local_map.max_y; }

    new_w = new_max_x - new_min_x + 1;
    new_h = new_max_y - new_min_y + 1;

    /* Allocate zero-filled arrays for the expanded grid */
    new_explored = calloc((size_t)(new_w * new_h), sizeof(*new_explored));
    new_scanned = calloc((size_t)(new_w * new_h), sizeof(*new_scanned));
    new_blocked = calloc((size_t)(new_w * new_h), sizeof(*new_blocked));
    if (new_explored == NULL || new_scanned == NULL || new_blocked == NULL) {
        free(new_explored);
        free(new_scanned);
        free(new_blocked);
        return false;
    }

    /* Copy old cells from their old indices to their new indices */
    for (yy = local_map.min_y; yy <= local_map.max_y; yy++) {
        int xx;
        for (xx = local_map.min_x; xx <= local_map.max_x; xx++) {
            int old_i = cell_index(xx, yy);
            int new_i = (yy - new_min_y) * new_w + (xx - new_min_x);
            new_explored[new_i] = local_map.explored[old_i];
            new_scanned[new_i] = local_map.scanned[old_i];
            new_blocked[new_i] = local_map.blocked[old_i];
        }
    }

    /* Replace old arrays and bounds with the expanded ones */
    free_map_arrays();
    local_map.min_x = new_min_x;
    local_map.max_x = new_max_x;
    local_map.min_y = new_min_y;
    local_map.max_y = new_max_y;
    local_map.width = new_w;
    local_map.height = new_h;
    local_map.explored = new_explored;
    local_map.scanned = new_scanned;
    local_map.blocked = new_blocked;
    return true;
}
```
*(Note: lines 91–113 in `local_map.c` write those four `if / else` blocks out across multiple lines for readability.)*

#### Visual Diagram: Why Must Old Cells Move from `old_i` to `new_i`?
```text
OLD 2x2 Grid (width = 2):                 NEW 3x2 Grid after scanning East (new_w = 3):
  [0] (0,0) | [1] (1,0)                     [0] (0,0) | [1] (1,0) | [2] (2,0) <-- new!
  ----------+----------                     ----------+-----------+----------
  [2] (0,1) | [3] (1,1)                     [3] (0,1) | [4] (1,1) | [5] (2,1) <-- new!
   ^                                         ^
   Notice cell (0,1) WAS at old_i = 2, and MUST move to new_i = 3 because width changed!
```
- **`calloc(count, elem_size)`:** Allocates heap memory AND zeroes every byte so all new border slots start empty (`false` / `'\0'`).

---

### Chunk C: Map Updates, Queries & Stage 3 Terrain Counts ([local_map.c](file:///c:/Users/nathanyu/Documents/GitHub/RSE1201_Group_Project/VehExplorer/src/local_map.c#L155-L380))

```c
void local_map_init(void)  { local_map_reset(); }

void local_map_reset(void)
{
    free_map_arrays();
    memset(&local_map, 0, sizeof(local_map));
}

int local_map_get_x(void) { return local_map.x; }
int local_map_get_y(void) { return local_map.y; }

/* Mark the vehicle's current cell as explored. */
void local_map_mark_explored_here(void)
{
    if (!grow_map_to_include(local_map.x, local_map.y)) {
        fprintf(stderr, "local_map: out of memory\n");
        return;
    }
    local_map.explored[cell_index(local_map.x, local_map.y)] = true;
}

/* Move vehicle coordinates by (dx, dy) and mark the new cell explored. */
void local_map_move_success(int dx, int dy)
{
    local_map.x += dx;
    local_map.y += dy;
    local_map_mark_explored_here();
}

/* Save a scanned terrain symbol at offset (dx, dy) and flag '#' walls blocked. */
void local_map_record_scan(int dx, int dy, char terrain)
{
    int sx = local_map.x + dx;
    int sy = local_map.y + dy;

    if (terrain == '\0') {
        return;
    }
    if (!grow_map_to_include(sx, sy)) {
        fprintf(stderr, "local_map: out of memory\n");
        return;
    }
    local_map.scanned[cell_index(sx, sy)] = terrain;
    if (terrain == '#') {
        local_map.blocked[cell_index(sx, sy)] = true;
    }
}
```
- **Lookup & Blocked Functions (`L215–307`):**
  - `local_map_peek_terrain(x, y)`: Returns `'?'` if `scanned == NULL`, `!in_bounds(x, y)`, or `scanned[i] == '\0'`; otherwise returns the scanned symbol.
  - `local_map_is_explored(x, y)` & `local_map_is_blocked(x, y)`: Return `false` if out-of-bounds, else read `explored[i]` / `blocked[i]`.
  - `local_map_mark_blocked(x, y)`: Grows grid to include `(x, y)` and sets `blocked[i] = true`.
  - `local_map_get_bounds`: Writes `min_x, max_x, min_y, max_y` through non-`NULL` caller pointers (`*min_x = local_map.min_x;`).
  - `local_map_count_scanned(terrain)`: Nested `y` / `x` loop counting how many cells equal `terrain`.
- **Stage 3 Terrain Tallies (`add_terrain_count` & `local_map_terrain_counts`, `L310–380`):**
  - Walks every cell in `[min_y..max_y] × [min_x..max_x]`. If a cell was scanned, increments `scanned` (and `explored` if also walked on). If a cell was walked on *without* ever being scanned (like spawn `(0, 0)`!), tallies it under `'?'` (`LOCAL_MAP_TERRAIN_UNKNOWN`) with `explored = 1`!
  - Notice **`(*used)++`** on line 346: parentheses are essential because postfix `++` has higher precedence than `*`—`(*used)++` increments the caller's `unsigned used` counter rather than moving the pointer!

---

### Chunk D: ANSI Colours, Tight Cropping & Grid Printing ([local_map.c](file:///c:/Users/nathanyu/Documents/GitHub/RSE1201_Group_Project/VehExplorer/src/local_map.c#L382-L614))

```c
/* Choose an ANSI foreground colour code for a terrain character. */
static int terrain_color(char ch)
{
    switch (ch) {
    case '#': case 's': case 'X': return 31; /* red: walls, swamp, danger */
    case 'h': case 'j': case 'g': return 32; /* green: hills, jungle, grass */
    case 'E': case 'M':           return 35; /* magenta: end point, mountain */
    case 'f': case '~': case '_': return 36; /* cyan: forest, water, vehicle */
    case 'w': case '-': case '.':
    default:                      return 37; /* white: normal / default */
    }
}

/* Check once whether stdout is a terminal so piped output stays plain text. */
static bool stdout_is_terminal(void)
{
    static int cached = -1;

    if (cached < 0) {
        if (isatty(STDOUT_FILENO)) {
            cached = 1;
        } else {
            cached = 0;
        }
    }
    return cached == 1;
}
```

#### How `bounds_of_known_cells`, `print_map_cell` & `print_grid` Render the Screen (`L451–614`):
1. **Why calculate `bounds_of_known_cells` (`L473–529`) instead of using `local_map.min_x..max_x` directly?**
   Remember when you scanned West at radius 2 past the map border (`Error! Scanning location is BEYOND map range!`), `veh_action_scan` marked `(x - 2, y)` **blocked**! That grew `local_map.min_x` outside the arena walls, even though those outside cells are empty (`' '`). `bounds_of_known_cells` shrinks the print rectangle tightly around **actual visited/scanned cells + the rover**, keeping the printed map crisp!
2. **3-Character Cell Alignment (`print_map_cell`, `L532–563`):**
   - Each cell occupies `LOCAL_MAP_CELL_WIDTH` (`3`) characters: `'  h'`, `'  #'`, `'  _'`.
   - **`printf("%*c", LOCAL_MAP_CELL_WIDTH, ch)`:** `*` inside `%*c` substitutes `3` as the width!
   - **Underlining the Rover (`\033[36;1;4m%c\033[0m`):** Prints 2 spaces *first* (`for (pad = 0; pad < 2; pad++) putchar(' ');`), and then underlines **only the 1 character** where the rover stands (`36` = cyan, `1` = bold, `4` = underline, `0` = reset)! And if output is redirected to a file (`!stdout_is_terminal()`), it prints plain characters with zero `\033` codes!

---

## 5. Ready-to-Use Presentation Script & Likely Teacher Q&A

### 45-Second Walkthrough Script
> "`local_map.c` builds our vehicle's map dynamically from `(0, 0)` without needing map dimensions or spawn coordinates from `libsatcom.so`. We store three heap-allocated layers—`explored`, `scanned`, and `blocked`—indexed by `(y - min_y) * width + (x - min_x)` so negative North and West coordinates map cleanly to zero-based 1D arrays. Whenever a move or scan reaches outside `[min_x..max_x, min_y..max_y]`, `grow_map_to_include` allocates zeroed arrays with `calloc` and copies existing cells to their new row-major indices. For display, `local_map_print_explored` and `local_map_print_scanned` crop tightly to known cells, underline the live vehicle tile, and check `isatty` so redirected log files stay clean ASCII."

### Likely Questions From Your Lecturer
1. **Q: Why couldn't `grow_map_to_include` just call `realloc` like `stats.c` did?**
   - **A:** `stats.c` stores a 1D list of rows where existing indices never change. `local_map.c` stores a 2D rectangle in row-major order `(y - min_y) * width + (x - min_x)`. Whenever `width` or `min_x` changes, existing `(x, y)` cells shift to new array indices, so we `calloc` the new rectangle and copy each `(xx, yy)` from `old_i` to `new_i`.
2. **Q: What does `static int cached = -1;` inside `stdout_is_terminal` do?**
   - **A:** `static` on a local variable gives it static storage duration—it keeps its value between function calls. So `isatty(STDOUT_FILENO)` is only queried from the OS on the first call (`cached < 0`), and every later cell print reuses the cached `0` or `1`.

#include "local_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LM_UNSEEN '\0'
#define LM_VISITED_MARK '.'
#define LM_VEHICLE_MARK '_'

/* Tutor TerrainMap_Print* uses cell field-width 3 (see VehController esi=3). */
#define LM_CELL_WIDTH 3

#define LM_CLR_RESET "\033[0m"

typedef struct {
    int x;
    int y;
    int min_x;
    int max_x;
    int min_y;
    int max_y;
    int width;
    int height;
    bool *explored; /* visited by successful move / spawn */
    char *scanned;  /* terrain char, or LM_UNSEEN */
} LocalMap;

static LocalMap g_map;

static void free_grids(void)
{
    free(g_map.explored);
    free(g_map.scanned);
    g_map.explored = NULL;
    g_map.scanned = NULL;
    g_map.width = 0;
    g_map.height = 0;
}

static int index_at(int x, int y)
{
    return (y - g_map.min_y) * g_map.width + (x - g_map.min_x);
}

static bool in_bounds(int x, int y)
{
    return x >= g_map.min_x && x <= g_map.max_x && y >= g_map.min_y &&
           y <= g_map.max_y;
}

/* Grow rectangle to include (x,y). Preserves existing cell data. */
static bool ensure_cell(int x, int y)
{
    int new_min_x;
    int new_max_x;
    int new_min_y;
    int new_max_y;
    int new_w;
    int new_h;
    bool *new_explored;
    char *new_scanned;
    int yy;

    if (g_map.explored == NULL) {
        g_map.min_x = g_map.max_x = x;
        g_map.min_y = g_map.max_y = y;
        g_map.width = 1;
        g_map.height = 1;
        g_map.explored = calloc(1, sizeof *g_map.explored);
        g_map.scanned = calloc(1, sizeof *g_map.scanned);
        return g_map.explored != NULL && g_map.scanned != NULL;
    }

    if (in_bounds(x, y)) {
        return true;
    }

    new_min_x = (x < g_map.min_x) ? x : g_map.min_x;
    new_max_x = (x > g_map.max_x) ? x : g_map.max_x;
    new_min_y = (y < g_map.min_y) ? y : g_map.min_y;
    new_max_y = (y > g_map.max_y) ? y : g_map.max_y;
    new_w = new_max_x - new_min_x + 1;
    new_h = new_max_y - new_min_y + 1;

    new_explored = calloc((size_t)new_w * (size_t)new_h, sizeof *new_explored);
    new_scanned = calloc((size_t)new_w * (size_t)new_h, sizeof *new_scanned);
    if (new_explored == NULL || new_scanned == NULL) {
        free(new_explored);
        free(new_scanned);
        return false;
    }

    for (yy = g_map.min_y; yy <= g_map.max_y; yy++) {
        int xx;
        for (xx = g_map.min_x; xx <= g_map.max_x; xx++) {
            int old_i = index_at(xx, yy);
            int new_i = (yy - new_min_y) * new_w + (xx - new_min_x);
            new_explored[new_i] = g_map.explored[old_i];
            new_scanned[new_i] = g_map.scanned[old_i];
        }
    }

    free_grids();
    g_map.min_x = new_min_x;
    g_map.max_x = new_max_x;
    g_map.min_y = new_min_y;
    g_map.max_y = new_max_y;
    g_map.width = new_w;
    g_map.height = new_h;
    g_map.explored = new_explored;
    g_map.scanned = new_scanned;
    return true;
}

void local_map_init(void)
{
    local_map_reset();
}

void local_map_reset(void)
{
    free_grids();
    memset(&g_map, 0, sizeof g_map);
}

int local_map_get_x(void)
{
    return g_map.x;
}

int local_map_get_y(void)
{
    return g_map.y;
}

void local_map_mark_explored_here(void)
{
    if (!ensure_cell(g_map.x, g_map.y)) {
        fprintf(stderr, "local_map: out of memory\n");
        return;
    }
    g_map.explored[index_at(g_map.x, g_map.y)] = true;
}

void local_map_move_success(int dx, int dy)
{
    g_map.x += dx;
    g_map.y += dy;
    local_map_mark_explored_here();
}

void local_map_record_scan(int dx, int dy, char terrain)
{
    int sx = g_map.x + dx;
    int sy = g_map.y + dy;

    if (terrain == '\0') {
        return;
    }

    if (!ensure_cell(sx, sy)) {
        fprintf(stderr, "local_map: out of memory\n");
        return;
    }
    g_map.scanned[index_at(sx, sy)] = terrain;
}

char local_map_peek_terrain(int x, int y)
{
    if (g_map.scanned == NULL || !in_bounds(x, y)) {
        return LM_TERRAIN_UNKNOWN;
    }

    {
        char ch = g_map.scanned[index_at(x, y)];
        return (ch != LM_UNSEEN) ? ch : LM_TERRAIN_UNKNOWN;
    }
}

char local_map_peek_terrain_at_offset(int dx, int dy)
{
    return local_map_peek_terrain(g_map.x + dx, g_map.y + dy);
}

static void tally_bump(LocalMapTerrainCount *out,
                       size_t capacity,
                       size_t *used,
                       char terrain,
                       bool scanned_inc,
                       bool explored_inc)
{
    size_t i;

    for (i = 0; i < *used; i++) {
        if (out[i].terrain == terrain) {
            if (scanned_inc) {
                out[i].scanned++;
            }
            if (explored_inc) {
                out[i].explored++;
            }
            return;
        }
    }

    if (*used >= capacity) {
        return;
    }

    out[*used].terrain = terrain;
    out[*used].scanned = scanned_inc ? 1UL : 0UL;
    out[*used].explored = explored_inc ? 1UL : 0UL;
    (*used)++;
}

size_t local_map_terrain_counts(LocalMapTerrainCount *out, size_t capacity)
{
    size_t used = 0;
    int x;
    int y;

    if (out == NULL || capacity == 0 || g_map.explored == NULL) {
        return 0;
    }

    for (y = g_map.min_y; y <= g_map.max_y; y++) {
        for (x = g_map.min_x; x <= g_map.max_x; x++) {
            int i = index_at(x, y);
            char scanned = g_map.scanned[i];
            bool is_explored = g_map.explored[i];

            if (scanned != LM_UNSEEN) {
                tally_bump(out, capacity, &used, scanned, true, is_explored);
            } else if (is_explored) {
                tally_bump(out,
                           capacity,
                           &used,
                           LM_TERRAIN_UNKNOWN,
                           false,
                           true);
            }
        }
    }

    return used;
}

static char cell_char(int x, int y, bool explored_view)
{
    if (g_map.explored == NULL || !in_bounds(x, y)) {
        return ' ';
    }

    {
        int i = index_at(x, y);

        if (explored_view) {
            if (!g_map.explored[i]) {
                return ' ';
            }
            if (g_map.scanned[i] != LM_UNSEEN) {
                return g_map.scanned[i];
            }
            return LM_VISITED_MARK;
        }

        return (g_map.scanned[i] != LM_UNSEEN) ? g_map.scanned[i] : ' ';
    }
}

/* Approximate TERRAIN_GetTerrainColorRepresentation ANSI codes. */
static int terrain_color(char ch)
{
    switch (ch) {
    case '#':
    case 's':
    case 'X':
        return 31; /* red */
    case 'h':
    case 'j':
    case 'g':
        return 32; /* green */
    case 'E':
    case 'M':
        return 35; /* magenta */
    case 'f':
    case '~':
    case '_':
        return 36; /* cyan */
    case 'w':
    case '-':
    case '.':
    default:
        return 37; /* white */
    }
}

static bool color_enabled(void)
{
    static int cached = -1;

    if (cached < 0) {
        cached = isatty(STDOUT_FILENO) ? 1 : 0;
    }
    return cached == 1;
}

static void print_green_line(const char *text)
{
    if (color_enabled()) {
        printf("\033[32m%s\033[0m\n", text);
    } else {
        printf("%s\n", text);
    }
}

static void print_tutor_header(const char *title)
{
    char line[80];

    /* Match VehController banner style (without the TUTOR watermark). */
    print_green_line("-------------------------------------");
    snprintf(line, sizeof line, "-- %s --", title);
    print_green_line(line);
    print_green_line("-------------------------------------");
}

static bool view_bounds(bool explored_view,
                        int *out_min_x,
                        int *out_max_x,
                        int *out_min_y,
                        int *out_max_y)
{
    int x;
    int y;
    int min_x;
    int max_x;
    int min_y;
    int max_y;
    bool any = false;

    if (g_map.explored == NULL) {
        return false;
    }

    min_x = max_x = g_map.x;
    min_y = max_y = g_map.y;
    any = true;

    for (y = g_map.min_y; y <= g_map.max_y; y++) {
        for (x = g_map.min_x; x <= g_map.max_x; x++) {
            int i = index_at(x, y);
            bool include = explored_view ? g_map.explored[i]
                                         : (g_map.scanned[i] != LM_UNSEEN);
            if (!include) {
                continue;
            }
            if (x < min_x) {
                min_x = x;
            }
            if (x > max_x) {
                max_x = x;
            }
            if (y < min_y) {
                min_y = y;
            }
            if (y > max_y) {
                max_y = y;
            }
        }
    }

    *out_min_x = min_x;
    *out_max_x = max_x;
    *out_min_y = min_y;
    *out_max_y = max_y;
    return any;
}

/*
 * Tutor cell print (spacing = 3):
 *   vehicle: (spacing-1) spaces + "\033[36;1;4m%c\033[0m"  (cyan bold underline)
 *   filled:  "\033[<color>;1m%*c\033[0m" with width = spacing
 *   empty:   "%*c" space with width = spacing
 */
static void put_tutor_cell(char ch, bool is_vehicle)
{
    int color = terrain_color(ch);

    if (is_vehicle) {
        int pad;
        for (pad = 0; pad < LM_CELL_WIDTH - 1; pad++) {
            putchar(' ');
        }
        /* Unknown underfoot → '_'; known terrain stays and is underlined. */
        if (ch == ' ' || ch == LM_VISITED_MARK) {
            ch = LM_VEHICLE_MARK;
        }
        if (color_enabled()) {
            printf("\033[36;1;4m%c\033[0m", ch);
        } else {
            putchar(ch == LM_VEHICLE_MARK ? '_' : ch);
        }
        return;
    }

    if (ch == ' ') {
        printf("%*c", LM_CELL_WIDTH, ' ');
        return;
    }

    if (color_enabled()) {
        printf("\033[%d;1m%*c\033[0m", color, LM_CELL_WIDTH, ch);
    } else {
        printf("%*c", LM_CELL_WIDTH, ch);
    }
}

static void print_grid(bool explored_view)
{
    int x;
    int y;
    int min_x;
    int max_x;
    int min_y;
    int max_y;

    if (!view_bounds(explored_view, &min_x, &max_x, &min_y, &max_y)) {
        printf("(empty map)\n");
        return;
    }

    putchar('\n');

    /* North = smaller y at top — same orientation as tutor. */
    for (y = min_y; y <= max_y; y++) {
        /* Leading indent like TerrainMap_Print* ("%*c", spacing, ' '). */
        printf("%*c", LM_CELL_WIDTH, ' ');

        for (x = min_x; x <= max_x; x++) {
            bool is_veh = (x == g_map.x && y == g_map.y);
            char ch = cell_char(x, y, explored_view);

            if (is_veh && ch == ' ') {
                ch = LM_VEHICLE_MARK;
            }
            put_tutor_cell(ch, is_veh);
        }
        putchar('\n');
    }

    putchar('\n');
}

void local_map_print_explored(void)
{
    putchar('\n');
    print_tutor_header("Print Explored Map");
    putchar('\n');
    print_grid(true);
}

void local_map_print_scanned(void)
{
    putchar('\n');
    print_tutor_header("Print Scanned Map");
    putchar('\n');
    print_grid(false);
}

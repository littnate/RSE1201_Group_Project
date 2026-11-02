#include "local_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOCAL_MAP_NOT_SCANNED '\0'
#define LOCAL_MAP_VISITED_MARK '.'
#define LOCAL_MAP_VEHICLE_MARK '_'
#define LOCAL_MAP_CELL_WIDTH 3

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

static int cell_index(int x, int y)
{
    return (y - local_map.min_y) * local_map.width + (x - local_map.min_x);
}

static bool in_bounds(int x, int y)
{
    return x >= local_map.min_x && x <= local_map.max_x &&
           y >= local_map.min_y && y <= local_map.max_y;
}

static bool grow_map_to_include(int x, int y)
{
    int new_min_x;
    int new_max_x;
    int new_min_y;
    int new_max_y;
    int new_w;
    int new_h;
    bool *new_explored;
    char *new_scanned;
    bool *new_blocked;
    int yy;

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

    if (in_bounds(x, y)) {
        return true;
    }

    if (x < local_map.min_x) {
        new_min_x = x;
    } else {
        new_min_x = local_map.min_x;
    }

    if (x > local_map.max_x) {
        new_max_x = x;
    } else {
        new_max_x = local_map.max_x;
    }

    if (y < local_map.min_y) {
        new_min_y = y;
    } else {
        new_min_y = local_map.min_y;
    }

    if (y > local_map.max_y) {
        new_max_y = y;
    } else {
        new_max_y = local_map.max_y;
    }

    new_w = new_max_x - new_min_x + 1;
    new_h = new_max_y - new_min_y + 1;

    new_explored = calloc((new_w * new_h), sizeof(*new_explored));
    new_scanned = calloc((new_w * new_h), sizeof(*new_scanned));
    new_blocked = calloc((new_w * new_h), sizeof(*new_blocked));
    if (new_explored == NULL || new_scanned == NULL || new_blocked == NULL) {
        free(new_explored);
        free(new_scanned);
        free(new_blocked);
        return false;
    }

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

void local_map_init(void)
{
    local_map_reset();
}

void local_map_reset(void)
{
    free_map_arrays();
    memset(&local_map, 0, sizeof(local_map));
}

void local_map_mark_explored_here(void)
{
    if (!grow_map_to_include(local_map.x, local_map.y)) {
        fprintf(stderr, "local_map: out of memory\n");
        return;
    }
    local_map.explored[cell_index(local_map.x, local_map.y)] = true;
}

void local_map_move_success(int dx, int dy)
{
    local_map.x += dx;
    local_map.y += dy;
    local_map_mark_explored_here();
}

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

char local_map_peek_terrain(int x, int y)
{
    char ch;

    if (local_map.scanned == NULL || !in_bounds(x, y)) {
        return LOCAL_MAP_TERRAIN_UNKNOWN;
    }

    ch = local_map.scanned[cell_index(x, y)];
    if (ch != LOCAL_MAP_NOT_SCANNED) {
        return ch;
    }
    return LOCAL_MAP_TERRAIN_UNKNOWN;
}

char local_map_peek_terrain_at_offset(int dx, int dy)
{
    return local_map_peek_terrain(local_map.x + dx, local_map.y + dy);
}

void local_map_mark_blocked(int x, int y)
{
    if (!grow_map_to_include(x, y)) {
        fprintf(stderr, "local_map: out of memory\n");
        return;
    }
    local_map.blocked[cell_index(x, y)] = true;
}

void local_map_mark_blocked_at_offset(int dx, int dy)
{
    local_map_mark_blocked(local_map.x + dx, local_map.y + dy);
}

static void add_terrain_count(LocalMapTerrainCount *out,
                              unsigned capacity,
                              unsigned *used,
                              char terrain,
                              bool scanned_inc,
                              bool explored_inc)
{
    unsigned i;

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
    if (scanned_inc) {
        out[*used].scanned = 1;
    } else {
        out[*used].scanned = 0;
    }
    if (explored_inc) {
        out[*used].explored = 1;
    } else {
        out[*used].explored = 0;
    }
    (*used)++;
}

unsigned local_map_terrain_counts(LocalMapTerrainCount *out, unsigned capacity)
{
    unsigned used = 0;
    int x;
    int y;

    if (out == NULL || capacity == 0 || local_map.explored == NULL) {
        return 0;
    }

    for (y = local_map.min_y; y <= local_map.max_y; y++) {
        for (x = local_map.min_x; x <= local_map.max_x; x++) {
            int i = cell_index(x, y);
            char scanned = local_map.scanned[i];
            bool is_explored = local_map.explored[i];

            if (scanned != LOCAL_MAP_NOT_SCANNED) {
                add_terrain_count(out, capacity, &used, scanned, true, is_explored);
            } else if (is_explored) {
                add_terrain_count(out,
                                  capacity,
                                  &used,
                                  LOCAL_MAP_TERRAIN_UNKNOWN,
                                  false,
                                  true);
            }
        }
    }

    return used;
}

static char character_to_print(int x, int y, bool explored_view)
{
    int i;

    if (local_map.explored == NULL || !in_bounds(x, y)) {
        return ' ';
    }

    i = cell_index(x, y);

    if (explored_view) {
        if (!local_map.explored[i]) {
            return ' ';
        }
        if (local_map.scanned[i] != LOCAL_MAP_NOT_SCANNED) {
            return local_map.scanned[i];
        }
        return LOCAL_MAP_VISITED_MARK;
    }

    if (local_map.scanned[i] != LOCAL_MAP_NOT_SCANNED) {
        return local_map.scanned[i];
    }
    return ' ';
}

static int terrain_color(char ch)
{
    switch (ch) {
    case '#':
    case 's':
    case 'X':
        return 31;
    case 'h':
    case 'j':
    case 'g':
        return 32;
    case 'E':
    case 'M':
        return 35;
    case 'f':
    case '~':
    case '_':
        return 36;
    case 'w':
    case '-':
    case '.':
    default:
        return 37;
    }
}

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

static void print_green_line(const char *text)
{
    if (stdout_is_terminal()) {
        printf("\033[32m%s\033[0m\n", text);
    } else {
        printf("%s\n", text);
    }
}

static void print_map_banner(const char *title)
{
    char line[80];

    print_green_line("-------------------------------------");
    snprintf(line, sizeof(line), "-- %s --", title);
    print_green_line(line);
    print_green_line("-------------------------------------");
}

static bool bounds_of_known_cells(bool explored_view,
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

    if (local_map.explored == NULL) {
        return false;
    }

    min_x = local_map.x;
    max_x = local_map.x;
    min_y = local_map.y;
    max_y = local_map.y;

    for (y = local_map.min_y; y <= local_map.max_y; y++) {
        for (x = local_map.min_x; x <= local_map.max_x; x++) {
            int i = cell_index(x, y);
            bool include;

            if (explored_view) {
                include = local_map.explored[i];
            } else {
                include = (local_map.scanned[i] != LOCAL_MAP_NOT_SCANNED);
            }

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
    return true;
}

static void print_map_cell(char ch, bool is_vehicle)
{
    int color = terrain_color(ch);

    if (is_vehicle) {
        int pad;
        for (pad = 0; pad < LOCAL_MAP_CELL_WIDTH - 1; pad++) {
            putchar(' ');
        }

        if (ch == ' ' || ch == LOCAL_MAP_VISITED_MARK) {
            ch = LOCAL_MAP_VEHICLE_MARK;
        }
        if (stdout_is_terminal()) {
            printf("\033[36;1;4m%c\033[0m", ch);
        } else {
            putchar(ch);
        }
        return;
    }

    if (ch == ' ') {
        printf("%*c", LOCAL_MAP_CELL_WIDTH, ' ');
        return;
    }

    if (stdout_is_terminal()) {
        printf("\033[%d;1m%*c\033[0m", color, LOCAL_MAP_CELL_WIDTH, ch);
    } else {
        printf("%*c", LOCAL_MAP_CELL_WIDTH, ch);
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

    if (!bounds_of_known_cells(explored_view, &min_x, &max_x, &min_y, &max_y)) {
        printf("(empty map)\n");
        return;
    }

    putchar('\n');

    for (y = min_y; y <= max_y; y++) {
        printf("%*c", LOCAL_MAP_CELL_WIDTH, ' ');

        for (x = min_x; x <= max_x; x++) {
            bool is_veh = (x == local_map.x && y == local_map.y);
            char ch = character_to_print(x, y, explored_view);

            if (is_veh && ch == ' ') {
                ch = LOCAL_MAP_VEHICLE_MARK;
            }
            print_map_cell(ch, is_veh);
        }
        putchar('\n');
    }

    putchar('\n');
}

void local_map_print_explored(void)
{
    putchar('\n');
    print_map_banner("Explored Map");
    putchar('\n');
    print_grid(true);
}

void local_map_print_scanned(void)
{
    putchar('\n');
    print_map_banner("Scanned Map");
    putchar('\n');
    print_grid(false);
}

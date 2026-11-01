#define _DEFAULT_SOURCE

#include "autopilot.h"

#include "local_map.h"
#include "mission_control.h"
#include "stats.h"
#include "util_input.h"
#include "veh_action.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#define AUTOPILOT_SHIELD_RESERVE 2500L
#define AUTOPILOT_END 'E'
#define AUTOPILOT_DANGER 'X'
#define AUTOPILOT_WALL '#'

/* Compass labels matching MissionControlScanDir enum order. */
static const char *scan_direction_labels[MISSION_CONTROL_DIR_COUNT] = {
    "NW", "N", "NE", "W", "E", "SW", "S", "SE"
};

/* Check cheaper cardinal scans (N, E, S, W) before diagonal scans. */
static const MissionControlScanDir scan_order[MISSION_CONTROL_DIR_COUNT] = {
    MISSION_CONTROL_DIR_NORTH,
    MISSION_CONTROL_DIR_EAST,
    MISSION_CONTROL_DIR_SOUTH,
    MISSION_CONTROL_DIR_WEST,
    MISSION_CONTROL_DIR_NORTHEAST,
    MISSION_CONTROL_DIR_NORTHWEST,
    MISSION_CONTROL_DIR_SOUTHEAST,
    MISSION_CONTROL_DIR_SOUTHWEST
};

/* Pairs a direction character ('N', 'E', 'S', 'W') with its (dx, dy) step. */
typedef struct {
    char direction;
    int dx;
    int dy;
} AutopilotMove;

static const AutopilotMove cardinal_moves[4] = {
    { 'N', 0, -1 },
    { 'E', 1, 0 },
    { 'S', 0, 1 },
    { 'W', -1, 0 },
};

/* Eight surrounding neighbour offsets at radius 1. */
static const int neighbor_dx[MISSION_CONTROL_DIR_COUNT] = { -1, 0, 1, -1, 1, -1, 0, 1 };
static const int neighbor_dy[MISSION_CONTROL_DIR_COUNT] = { -1, -1, -1, 0, 0, 1, 1, 1 };

static bool autopilot_is_end(char ch)
{
    return ch == AUTOPILOT_END;
}

static bool autopilot_is_danger(char ch)
{
    return ch == AUTOPILOT_DANGER;
}

/* Return true only if (x, y) has been scanned and is safe to walk onto. */
static bool autopilot_cell_is_passable(int x, int y)
{
    char ch;

    if (local_map_is_blocked(x, y)) {
        return false;
    }
    ch = local_map_peek_terrain(x, y);
    if (ch == LOCAL_MAP_TERRAIN_UNKNOWN) {
        return false;
    }
    if (ch == AUTOPILOT_WALL) {
        return false;
    }
    if (autopilot_is_danger(ch)) {
        return false;
    }
    return true;
}

/* Return true if (x, y) has not been scanned yet and is not marked blocked. */
static bool autopilot_unknown_at(int x, int y)
{
    return !local_map_is_blocked(x, y) &&
           local_map_peek_terrain(x, y) == LOCAL_MAP_TERRAIN_UNKNOWN;
}

/* Return true if any known non-wall cell still has an unscanned neighbour. */
static bool autopilot_unknown_cell_remains(void)
{
    int min_x;
    int max_x;
    int min_y;
    int max_y;
    int x;
    int y;
    int i;

    if (!local_map_has_grid()) {
        return true;
    }

    local_map_get_bounds(&min_x, &max_x, &min_y, &max_y);
    for (y = min_y; y <= max_y; y++) {
        for (x = min_x; x <= max_x; x++) {
            char ch = local_map_peek_terrain(x, y);
            bool known = local_map_is_explored(x, y) ||
                         (ch != LOCAL_MAP_TERRAIN_UNKNOWN);
            if (!known || local_map_is_blocked(x, y) || ch == AUTOPILOT_WALL) {
                continue;
            }
            for (i = 0; i < MISSION_CONTROL_DIR_COUNT; i++) {
                if (autopilot_unknown_at(x + neighbor_dx[i], y + neighbor_dy[i])) {
                    return true;
                }
            }
        }
    }
    return false;
}

/* Return true if any safe scanned cell has not been stepped on yet. */
static bool autopilot_has_unvisited_passable(void)
{
    int min_x;
    int max_x;
    int min_y;
    int max_y;
    int x;
    int y;

    if (!local_map_has_grid()) {
        return false;
    }

    local_map_get_bounds(&min_x, &max_x, &min_y, &max_y);
    for (y = min_y; y <= max_y; y++) {
        for (x = min_x; x <= max_x; x++) {
            if (autopilot_cell_is_passable(x, y) && !local_map_is_explored(x, y)) {
                return true;
            }
        }
    }
    return false;
}

/* Return true if at least one 'E' was found and every known 'E' was visited. */
static bool autopilot_all_known_ends_visited(void)
{
    int min_x;
    int max_x;
    int min_y;
    int max_y;
    int x;
    int y;
    bool any = false;

    if (!local_map_has_grid()) {
        return false;
    }

    local_map_get_bounds(&min_x, &max_x, &min_y, &max_y);
    for (y = min_y; y <= max_y; y++) {
        for (x = min_x; x <= max_x; x++) {
            if (!autopilot_is_end(local_map_peek_terrain(x, y))) {
                continue;
            }
            any = true;
            if (!local_map_is_explored(x, y)) {
                return false;
            }
        }
    }
    return any;
}

static unsigned autopilot_end_count(void)
{
    return local_map_count_scanned(AUTOPILOT_END);
}

/* Check whether the goal condition for mission_type (1-8) has been met. */
static bool autopilot_goal_met(int mission_type)
{
    char here;
    bool frontier;
    bool unvisited;

    here = local_map_peek_terrain(local_map_get_x(), local_map_get_y());
    frontier = autopilot_unknown_cell_remains();
    unvisited = autopilot_has_unvisited_passable();

    switch (mission_type) {
    case 1:
        return false;
    case 2:
        return autopilot_end_count() >= 1;
    case 3:
        return autopilot_is_end(here);
    case 4:
        return !frontier;
    case 5:
        return !frontier && autopilot_all_known_ends_visited();
    case 6:
        return !frontier;
    case 7:
        return !frontier && local_map_count_scanned(AUTOPILOT_WALL) > 0;
    case 8:
        return !frontier && !unvisited;
    default:
        return false;
    }
}

/* Check battery floor and move/scan action limits, printing the stop reason. */
static bool autopilot_stop_limit_hit(const VehDataType *vehicle,
                                     const AutopilotOptions *opt)
{
    StatsCounters s = stats_get();

    if (vehicle->currEnergy <= 0) {
        printf("Auto-pilot stopped: energy depleted.\n");
        return true;
    }
    if (vehicle->currEnergy < opt->energy_floor) {
        printf("Auto-pilot stopped: energy below limit.\n");
        return true;
    }
    if (opt->max_moves > 0 && s.moves_issued >= opt->max_moves) {
        printf("Auto-pilot stopped: move limit reached.\n");
        return true;
    }
    if (opt->max_scans > 0 && s.scans_issued >= opt->max_scans) {
        printf("Auto-pilot stopped: scan limit reached.\n");
        return true;
    }
    return false;
}

/* Initialize default auto-pilot options before user configuration. */
void autopilot_options_set_defaults(AutopilotOptions *opt)
{
    if (opt == NULL) {
        return;
    }
    memset(opt, 0, sizeof(*opt));
    opt->map_scan = true;
    opt->map_move = true;
    opt->energy_floor = 1;
}

/* Helper returning "y" or "n" for printing boolean option flags. */
static const char *yes_no_text(bool flag)
{
    if (flag) {
        return "y";
    }
    return "n";
}

/* Print a summary of the active auto-pilot configuration. */
void autopilot_print_options(const AutopilotOptions *opt)
{
    if (opt == NULL) {
        return;
    }

    printf("\n[ Auto-pilot options ]\n");
    printf("  Stats after every action : %s\n", yes_no_text(opt->stats_every));
    if (!opt->stats_every) {
        printf("  Stats after scan/move/shield : %s / %s / %s\n",
               yes_no_text(opt->stats_scan),
               yes_no_text(opt->stats_move),
               yes_no_text(opt->stats_shield));
    }
    printf("  Map after scan / move : %s / %s\n",
           yes_no_text(opt->map_scan),
           yes_no_text(opt->map_move));
    printf("  Delays t1..t7 (s) : %ld %ld %ld %ld %ld %ld %ld\n",
           opt->t1, opt->t2, opt->t3, opt->t4, opt->t5, opt->t6, opt->t7);
    printf("  Stop if energy < %ld, moves > %lu (0=off), scans > %lu (0=off)\n",
           opt->energy_floor, opt->max_moves, opt->max_scans);
}

/* Run the 'o' configuration prompts (applying Appendix A skip rules). */
bool autopilot_prompt_options(AutopilotOptions *opt)
{
    long entered_cap;

    if (opt == NULL) {
        return false;
    }

    autopilot_options_set_defaults(opt);

    printf("\n=== Configure Auto-Pilot ===\n");
    printf("(Press Enter to accept the default shown in [brackets].)\n");

    if (!util_prompt_yes_no_default(
            "1) After every action, show vehicle status / energy stats? "
            "(y/n) [n] : ",
            false,
            &opt->stats_every)) {
        return false;
    }

    /* Skip Q2-Q4 if Q1 already enabled stats after every action */
    if (!opt->stats_every) {
        if (!util_prompt_yes_no_default(
                "2) After every scan, show energy stats? (y/n) [n] : ",
                false,
                &opt->stats_scan) ||
            !util_prompt_yes_no_default(
                "3) After every move, show energy stats? (y/n) [n] : ",
                false,
                &opt->stats_move) ||
            !util_prompt_yes_no_default(
                "4) After every shield allocation, show energy stats? "
                "(y/n) [n] : ",
                false,
                &opt->stats_shield)) {
            return false;
        }
    }

    if (!util_prompt_yes_no_default(
            "5) After every scan, show scanned map? (y/n) [y] : ",
            true,
            &opt->map_scan) ||
        !util_prompt_yes_no_default(
            "6) After every move, show travelled map? (y/n) [y] : ",
            true,
            &opt->map_move)) {
        return false;
    }

    if (!util_prompt_long_default(
            "7) t1 secs delay after displaying energy stats [0] : ",
            0, 0, &opt->t1) ||
        !util_prompt_long_default(
            "8) t2 secs delay after displaying scanned map [0] : ",
            0, 0, &opt->t2) ||
        !util_prompt_long_default(
            "9) t3 secs delay after displaying travelled map [0] : ",
            0, 0, &opt->t3) ||
        !util_prompt_long_default(
            "10) t4 secs delay after any auto-pilot action [0] : ",
            0, 0, &opt->t4)) {
        return false;
    }

    /* Skip per-action delays t5-t7 if global delay t4 is non-zero */
    if (opt->t4 == 0) {
        if (!util_prompt_long_default(
                "11) t5 secs delay after each auto scan [0] : ",
                0, 0, &opt->t5) ||
            !util_prompt_long_default(
                "12) t6 secs delay after each auto move [0] : ",
                0, 0, &opt->t6) ||
            !util_prompt_long_default(
                "13) t7 secs delay after each auto shield [0] : ",
                0, 0, &opt->t7)) {
            return false;
        }
    }

    if (!util_prompt_long_default(
            "a) Pause when current energy falls below N [1] : ",
            0, 1, &opt->energy_floor)) {
        return false;
    }
    if (!util_prompt_long_default(
            "b) Pause when move actions exceed N (0 = no cap) [0] : ",
            0, 0, &entered_cap)) {
        return false;
    }
    opt->max_moves = (unsigned long)entered_cap;
    if (!util_prompt_long_default(
            "c) Pause when scan actions exceed N (0 = no cap) [0] : ",
            0, 0, &entered_cap)) {
        return false;
    }
    opt->max_scans = (unsigned long)entered_cap;

    opt->configured = true;
    autopilot_print_options(opt);
    return true;
}

/* Peek at stdin without blocking to see if the user entered 'p' or 'q'. */
static bool autopilot_poll_interrupt(char *cmd)
{
    fd_set rfds;
    struct timeval tv;
    int n;

    /* Ignore piped input so scripted commands after 'p' are not consumed early */
    if (!isatty(STDIN_FILENO)) {
        return false;
    }

    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    n = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
    if (n <= 0) {
        return false;
    }

    *cmd = util_read_command_char();
    return *cmd == 'p' || *cmd == 'q';
}

/* Sleep one second at a time while checking for 'p' or 'q' interrupts. */
static bool autopilot_sleep_secs(long secs, char *cmd)
{
    long i;

    if (autopilot_poll_interrupt(cmd)) {
        return true;
    }
    for (i = 0; i < secs; i++) {
        sleep(1);
        if (autopilot_poll_interrupt(cmd)) {
            return true;
        }
    }
    return false;
}

/* Apply configured post-display and post-action delays (t1..t7). */
static bool autopilot_show_action_result(const AutopilotOptions *opt,
                                         char kind,
                                         bool printed_stats,
                                         bool printed_scan_map,
                                         bool printed_move_map,
                                         char *cmd)
{
    if (printed_stats && autopilot_sleep_secs(opt->t1, cmd)) {
        return true;
    }
    if (printed_scan_map && autopilot_sleep_secs(opt->t2, cmd)) {
        return true;
    }
    if (printed_move_map && autopilot_sleep_secs(opt->t3, cmd)) {
        return true;
    }
    if (opt->t4 > 0) {
        return autopilot_sleep_secs(opt->t4, cmd);
    }
    if (kind == 's') {
        return autopilot_sleep_secs(opt->t5, cmd);
    }
    if (kind == 'm') {
        return autopilot_sleep_secs(opt->t6, cmd);
    }
    if (kind == 'i') {
        return autopilot_sleep_secs(opt->t7, cmd);
    }
    return autopilot_poll_interrupt(cmd);
}

typedef struct {
    int x;
    int y;
} AutopilotPoint;

/* Find the shortest safe path to a goal cell using Breadth-First Search (BFS)
 * and output only the immediate first step (out_dx, out_dy). */
static bool autopilot_bfs_step(bool any_frontier,
                               bool any_unvisited,
                               bool any_unvisited_end,
                               bool any_end,
                               int *out_dx,
                               int *out_dy)
{
    int min_x;
    int max_x;
    int min_y;
    int max_y;
    int w;
    int h;
    int sx;
    int sy;
    int cap;
    int head;
    int tail;
    int i;
    AutopilotPoint *queue;
    int *came_from;
    bool *visited;
    bool found = false;
    int fx = 0;
    int fy = 0;

    if (!local_map_has_grid()) {
        return false;
    }

    /* Build a search window covering the known map plus a 1-cell margin */
    local_map_get_bounds(&min_x, &max_x, &min_y, &max_y);
    min_x--;
    min_y--;
    max_x++;
    max_y++;
    w = max_x - min_x + 1;
    h = max_y - min_y + 1;
    cap = w * h;
    if (cap <= 0 || cap > 20000) {
        return false;
    }

    /* Allocate BFS queue, parent breadcrumb table, and visited flags */
    queue = malloc((size_t)cap * sizeof(*queue));
    came_from = malloc((size_t)cap * sizeof(*came_from));
    visited = calloc((size_t)cap, sizeof(*visited));
    if (queue == NULL || came_from == NULL || visited == NULL) {
        free(queue);
        free(came_from);
        free(visited);
        return false;
    }

    /* Enqueue vehicle's starting position (sx, sy) */
    sx = local_map_get_x();
    sy = local_map_get_y();
    head = 0;
    tail = 0;
    queue[tail].x = sx;
    queue[tail].y = sy;
    tail++;
    visited[(sy - min_y) * w + (sx - min_x)] = true;
    came_from[(sy - min_y) * w + (sx - min_x)] = -1;

    /* Expand outward layer-by-layer until a matching goal cell is reached */
    while (head < tail) {
        AutopilotPoint cur = queue[head++];
        int ci = (cur.y - min_y) * w + (cur.x - min_x);
        bool is_goal = false;

        if (any_unvisited_end) {
            is_goal = autopilot_is_end(local_map_peek_terrain(cur.x, cur.y)) &&
                      !local_map_is_explored(cur.x, cur.y);
        } else if (any_end) {
            is_goal = autopilot_is_end(local_map_peek_terrain(cur.x, cur.y)) &&
                      !(cur.x == sx && cur.y == sy);
        } else if (any_unvisited) {
            is_goal = autopilot_cell_is_passable(cur.x, cur.y) &&
                      !local_map_is_explored(cur.x, cur.y);
        } else if (any_frontier) {
            int d;
            for (d = 0; d < MISSION_CONTROL_DIR_COUNT; d++) {
                if (autopilot_unknown_at(cur.x + neighbor_dx[d], cur.y + neighbor_dy[d])) {
                    is_goal = true;
                    break;
                }
            }
        }

        if (is_goal && !(cur.x == sx && cur.y == sy)) {
            found = true;
            fx = cur.x;
            fy = cur.y;
            break;
        }

        /* Add safe, unvisited cardinal neighbours to the back of the queue */
        for (i = 0; i < 4; i++) {
            int nx = cur.x + cardinal_moves[i].dx;
            int ny = cur.y + cardinal_moves[i].dy;
            int ni;

            if (nx < min_x || nx > max_x || ny < min_y || ny > max_y) {
                continue;
            }
            if (!autopilot_cell_is_passable(nx, ny) && !(nx == sx && ny == sy)) {
                continue;
            }
            ni = (ny - min_y) * w + (nx - min_x);
            if (visited[ni]) {
                continue;
            }
            visited[ni] = true;
            came_from[ni] = ci;
            queue[tail].x = nx;
            queue[tail].y = ny;
            tail++;
        }
    }

    /* Trace breadcrumbs back from goal to find the first step from (sx, sy) */
    if (found) {
        int gi = (fy - min_y) * w + (fx - min_x);
        int si = (sy - min_y) * w + (sx - min_x);
        int step_index = gi;

        while (came_from[step_index] != si && came_from[step_index] != -1) {
            step_index = came_from[step_index];
        }
        if (came_from[step_index] == si) {
            int px = min_x + (step_index % w);
            int py = min_y + (step_index / w);
            *out_dx = px - sx;
            *out_dy = py - sy;
            free(queue);
            free(came_from);
            free(visited);
            return true;
        }
    }

    free(queue);
    free(came_from);
    free(visited);
    return false;
}

/* Pick an unscanned cell within radius 1 or 2 (cardinal directions first). */
static bool autopilot_pick_scan(MissionControlScanDir *dir, int *radius)
{
    int r;
    int i;

    for (r = 1; r <= 2; r++) {
        mission_control_set_scan_radius(r);
        for (i = 0; i < MISSION_CONTROL_DIR_COUNT; i++) {
            MissionControlScanDir d = scan_order[i];
            int dx;
            int dy;
            int x;
            int y;

            mission_control_scan_offset(d, &dx, &dy);
            x = local_map_get_x() + dx;
            y = local_map_get_y() + dy;
            if (autopilot_unknown_at(x, y)) {
                *dir = d;
                *radius = r;
                return true;
            }
        }
    }
    return false;
}

/* Choose the next cardinal move toward the mission goal or open frontier. */
static bool autopilot_pick_move(int mission_type, int *move_i)
{
    int dx = 0;
    int dy = 0;
    bool step_found = false;
    int i;

    if (mission_type == 3) {
        step_found = autopilot_bfs_step(false, false, false, true, &dx, &dy);
    } else if (mission_type == 5) {
        step_found = autopilot_bfs_step(false, false, true, false, &dx, &dy);
    } else if (mission_type == 8) {
        step_found = autopilot_bfs_step(false, true, false, false, &dx, &dy);
    }

    /* Fall back to approaching the nearest unscanned frontier */
    if (!step_found) {
        step_found = autopilot_bfs_step(true, false, false, false, &dx, &dy);
    }
    if (!step_found) {
        return false;
    }

    /* Match (dx, dy) to its entry in cardinal_moves[0..3] */
    for (i = 0; i < 4; i++) {
        if (cardinal_moves[i].dx == dx && cardinal_moves[i].dy == dy) {
            *move_i = i;
            return true;
        }
    }
    return false;
}

/* Calculate how much battery energy to divert when shields run low. */
static long autopilot_shield_amount(const VehDataType *vehicle)
{
    long chunk = vehicle->currEnergy / 10;

    if (chunk < 1000) {
        chunk = vehicle->currEnergy / 2;
    }
    if (chunk >= vehicle->currEnergy) {
        chunk = vehicle->currEnergy / 2;
    }
    if (chunk < 1) {
        return 0;
    }
    return chunk;
}

/* Print stop message and return matching result when interrupted by 'p' or 'q'. */
static AutopilotRunResult stop_on_interrupt(char cmd)
{
    if (cmd == 'q') {
        printf("Auto-pilot stopped: quit.\n");
        return AUTOPILOT_RUN_QUIT;
    }
    printf("Auto-pilot stopped: toggled off.\n");
    return AUTOPILOT_RUN_PAUSED;
}

/* Main auto-pilot loop: check stop rules, maintain shields, scan, or BFS move. */
AutopilotRunResult autopilot_run(VehDataType *vehicle,
                                 int mission_type,
                                 AutopilotOptions *opt)
{
    printf("\nAuto-pilot ON (mission type %d). Press p to pause, q to quit.\n",
           mission_type);

    while (true) {
        char cmd = '\0';
        MissionControlScanDir sdir;
        int radius;
        int move_i;
        bool print_stats;
        bool print_map;
        long chunk;

        /* 1. Check whether the user typed 'p' or 'q' */
        if (autopilot_poll_interrupt(&cmd)) {
            return stop_on_interrupt(cmd);
        }

        /* 2. Check mission completion, destruction, battery floor, and caps */
        if (mission_control_is_mission_over()) {
            printf("Auto-pilot stopped: mission over.\n");
            return AUTOPILOT_RUN_PAUSED;
        }
        if (autopilot_goal_met(mission_type)) {
            printf("Auto-pilot stopped: mission objective achieved.\n");
            return AUTOPILOT_RUN_PAUSED;
        }
        if (autopilot_stop_limit_hit(vehicle, opt)) {
            return AUTOPILOT_RUN_PAUSED;
        }

        /* 3. Top up shields if below reserve threshold */
        if (vehicle->currShieldEnergy < AUTOPILOT_SHIELD_RESERVE &&
            vehicle->currEnergy > AUTOPILOT_SHIELD_RESERVE) {
            chunk = autopilot_shield_amount(vehicle);
            if (chunk > 0) {
                print_stats = opt->stats_every || opt->stats_shield;
                printf("Auto-pilot: allocate %ld to shields.\n", chunk);
                veh_action_allocate(vehicle, chunk, print_stats);
                if (autopilot_show_action_result(opt, 'i', print_stats, false, false, &cmd)) {
                    return stop_on_interrupt(cmd);
                }
                continue;
            }
        }

        /* 4. Scan any nearby unknown cell at radius 1 or 2 */
        if (autopilot_pick_scan(&sdir, &radius)) {
            StatsCounters before = stats_get();

            if (opt->max_scans > 0 && before.scans_issued >= opt->max_scans) {
                printf("Auto-pilot stopped: scan limit reached.\n");
                return AUTOPILOT_RUN_PAUSED;
            }
            mission_control_set_scan_radius(radius);
            print_stats = opt->stats_every || opt->stats_scan;
            print_map = opt->map_scan;
            printf("Auto-pilot: scan %s radius %d.\n",
                   scan_direction_labels[sdir],
                   radius);
            veh_action_scan(vehicle, sdir, scan_direction_labels[sdir], print_stats, print_map);
            if (autopilot_show_action_result(opt, 's', print_stats, print_map, false, &cmd)) {
                return stop_on_interrupt(cmd);
            }
            continue;
        }

        /* 5. Step along shortest safe BFS path toward goal or frontier */
        if (autopilot_pick_move(mission_type, &move_i)) {
            StatsCounters before = stats_get();

            if (opt->max_moves > 0 && before.moves_issued >= opt->max_moves) {
                printf("Auto-pilot stopped: move limit reached.\n");
                return AUTOPILOT_RUN_PAUSED;
            }
            print_stats = opt->stats_every || opt->stats_move;
            print_map = opt->map_move;
            printf("Auto-pilot: move dx=%d dy=%d.\n",
                   cardinal_moves[move_i].dx,
                   cardinal_moves[move_i].dy);
            veh_action_move(vehicle,
                            cardinal_moves[move_i].direction,
                            cardinal_moves[move_i].dx,
                            cardinal_moves[move_i].dy,
                            print_stats,
                            print_map);
            if (autopilot_show_action_result(opt, 'm', print_stats, false, print_map, &cmd)) {
                return stop_on_interrupt(cmd);
            }
            continue;
        }

        /* 6. Nothing left to scan or reach */
        printf("Auto-pilot stopped: stuck / map exhausted.\n");
        return AUTOPILOT_RUN_PAUSED;
    }
}

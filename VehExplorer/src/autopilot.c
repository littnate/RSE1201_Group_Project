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

#define AP_SHIELD_RESERVE 2500L
#define AP_END 'E'
#define AP_DANGER 'X'
#define AP_WALL '#'

static const char *k_scan_name[MC_DIR_COUNT] = {
    "NW", "N", "NE", "W", "E", "SW", "S", "SE"
};

/* Prefer cheaper cardinal scans first. */
static const McScanDir k_scan_order[MC_DIR_COUNT] = {
    MC_DIR_N, MC_DIR_E, MC_DIR_S, MC_DIR_W,
    MC_DIR_NE, MC_DIR_NW, MC_DIR_SE, MC_DIR_SW
};

typedef struct {
    bool (*fn)(VehDataType *);
    int dx;
    int dy;
} ApMove;

static const ApMove k_moves[4] = {
    { mc_move_north, 0, -1 },
    { mc_move_east, 1, 0 },
    { mc_move_south, 0, 1 },
    { mc_move_west, -1, 0 },
};

/* McScanDir order, radius 1. */
static const int k8dx[MC_DIR_COUNT] = { -1, 0, 1, -1, 1, -1, 0, 1 };
static const int k8dy[MC_DIR_COUNT] = { -1, -1, -1, 0, 0, 1, 1, 1 };

static bool ap_is_end(char ch)
{
    return ch == AP_END;
}

static bool ap_is_danger(char ch)
{
    return ch == AP_DANGER;
}

static bool ap_walkable(int x, int y)
{
    char ch;

    if (local_map_is_blocked(x, y)) {
        return false;
    }
    ch = local_map_peek_terrain(x, y);
    if (ch == LM_TERRAIN_UNKNOWN) {
        return false;
    }
    if (ch == AP_WALL) {
        return false;
    }
    if (ap_is_danger(ch)) {
        return false;
    }
    return true;
}

static bool ap_unknown_at(int x, int y)
{
    return !local_map_is_blocked(x, y) &&
           local_map_peek_terrain(x, y) == LM_TERRAIN_UNKNOWN;
}

static bool ap_frontier_open(void)
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
                         (ch != LM_TERRAIN_UNKNOWN);
            if (!known || local_map_is_blocked(x, y) || ch == AP_WALL) {
                continue;
            }
            for (i = 0; i < MC_DIR_COUNT; i++) {
                if (ap_unknown_at(x + k8dx[i], y + k8dy[i])) {
                    return true;
                }
            }
        }
    }
    return false;
}

static bool ap_has_unvisited_passable(void)
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
            if (ap_walkable(x, y) && !local_map_is_explored(x, y)) {
                return true;
            }
        }
    }
    return false;
}

static bool ap_all_known_ends_visited(void)
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
            if (!ap_is_end(local_map_peek_terrain(x, y))) {
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

static unsigned ap_end_count(void)
{
    return local_map_count_scanned(AP_END);
}

static bool ap_goal_met(int mission_type)
{
    char here;
    bool frontier;
    bool unvisited;

    here = local_map_peek_terrain(local_map_get_x(), local_map_get_y());
    frontier = ap_frontier_open();
    unvisited = ap_has_unvisited_passable();

    switch (mission_type) {
    case 1:
        return false;
    case 2:
        return ap_end_count() >= 1;
    case 3:
        return ap_is_end(here);
    case 4:
        return !frontier;
    case 5:
        return !frontier && ap_all_known_ends_visited();
    case 6:
        return !frontier;
    case 7:
        return !frontier && local_map_count_scanned(AP_WALL) > 0;
    case 8:
        return !frontier && !unvisited;
    default:
        return false;
    }
}

static bool ap_caps_hit(const ApOptions *opt, const char **why)
{
    StatsCounters s = stats_get();

    if (opt->max_moves > 0 && s.moves_issued >= opt->max_moves) {
        *why = "move limit reached.";
        return true;
    }
    if (opt->max_scans > 0 && s.scans_issued >= opt->max_scans) {
        *why = "scan limit reached.";
        return true;
    }
    return false;
}

static bool ap_energy_hit(const VehDataType *vdt,
                          const ApOptions *opt,
                          const char **why)
{
    if (vdt->currEnergy <= 0) {
        *why = "energy depleted.";
        return true;
    }
    if (vdt->currEnergy < opt->energy_floor) {
        *why = "energy below limit.";
        return true;
    }
    return false;
}

void ap_options_set_defaults(ApOptions *opt)
{
    if (opt == NULL) {
        return;
    }
    memset(opt, 0, sizeof *opt);
    opt->map_scan = true;
    opt->map_move = true;
    opt->energy_floor = 1;
}

void ap_print_options(const ApOptions *opt)
{
    if (opt == NULL) {
        return;
    }

    printf("\n[ Auto-pilot options ]\n");
    printf("  Stats after every action : %s\n", opt->stats_every ? "y" : "n");
    if (!opt->stats_every) {
        printf("  Stats after scan/move/shield : %s / %s / %s\n",
               opt->stats_scan ? "y" : "n",
               opt->stats_move ? "y" : "n",
               opt->stats_shield ? "y" : "n");
    }
    printf("  Map after scan / move : %s / %s\n",
           opt->map_scan ? "y" : "n",
           opt->map_move ? "y" : "n");
    printf("  Delays t1..t7 (s) : %ld %ld %ld %ld %ld %ld %ld\n",
           opt->t1, opt->t2, opt->t3, opt->t4, opt->t5, opt->t6, opt->t7);
    printf("  Stop if energy < %ld, moves > %lu (0=off), scans > %lu (0=off)\n",
           opt->energy_floor, opt->max_moves, opt->max_scans);
}

bool ap_prompt_options(ApOptions *opt)
{
    long tmp;

    if (opt == NULL) {
        return false;
    }

    ap_options_set_defaults(opt);

    printf("\n=== Configure Auto-Pilot ===\n");
    printf("(Press Enter to accept the default shown in [brackets].)\n");

    if (!util_prompt_yes_no_default(
            "1) After every action, show vehicle status / energy stats? "
            "(y/n) [n] : ",
            false,
            &opt->stats_every)) {
        return false;
    }

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
            0, 0, &tmp)) {
        return false;
    }
    opt->max_moves = (unsigned long)tmp;
    if (!util_prompt_long_default(
            "c) Pause when scan actions exceed N (0 = no cap) [0] : ",
            0, 0, &tmp)) {
        return false;
    }
    opt->max_scans = (unsigned long)tmp;

    opt->configured = true;
    ap_print_options(opt);
    return true;
}

static bool ap_poll_interrupt(char *cmd)
{
    fd_set rfds;
    struct timeval tv;
    int n;

    /* Piped sessions feed q/o after `p`; do not steal those lines. */
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

static bool ap_sleep_secs(long secs, char *cmd)
{
    long i;

    if (ap_poll_interrupt(cmd)) {
        return true;
    }
    for (i = 0; i < secs; i++) {
        sleep(1);
        if (ap_poll_interrupt(cmd)) {
            return true;
        }
    }
    return false;
}

static bool ap_after_action(const ApOptions *opt,
                            char kind,
                            bool printed_stats,
                            bool printed_scan_map,
                            bool printed_move_map,
                            char *cmd)
{
    if (printed_stats && ap_sleep_secs(opt->t1, cmd)) {
        return true;
    }
    if (printed_scan_map && ap_sleep_secs(opt->t2, cmd)) {
        return true;
    }
    if (printed_move_map && ap_sleep_secs(opt->t3, cmd)) {
        return true;
    }
    if (opt->t4 > 0) {
        return ap_sleep_secs(opt->t4, cmd);
    }
    if (kind == 's') {
        return ap_sleep_secs(opt->t5, cmd);
    }
    if (kind == 'm') {
        return ap_sleep_secs(opt->t6, cmd);
    }
    if (kind == 'i') {
        return ap_sleep_secs(opt->t7, cmd);
    }
    return ap_poll_interrupt(cmd);
}

typedef struct {
    int x;
    int y;
} ApPt;

static bool ap_bfs_step(int goal_x,
                        int goal_y,
                        bool any_frontier,
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
    ApPt *q;
    int *came;
    bool *seen;
    bool found = false;
    int fx = 0;
    int fy = 0;

    (void)goal_x;
    (void)goal_y;

    if (!local_map_has_grid()) {
        return false;
    }

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

    q = malloc((size_t)cap * sizeof *q);
    came = malloc((size_t)cap * sizeof *came);
    seen = calloc((size_t)cap, sizeof *seen);
    if (q == NULL || came == NULL || seen == NULL) {
        free(q);
        free(came);
        free(seen);
        return false;
    }

    sx = local_map_get_x();
    sy = local_map_get_y();
    head = 0;
    tail = 0;
    q[tail].x = sx;
    q[tail].y = sy;
    tail++;
    seen[(sy - min_y) * w + (sx - min_x)] = true;
    came[(sy - min_y) * w + (sx - min_x)] = -1;

    while (head < tail) {
        ApPt cur = q[head++];
        int ci = (cur.y - min_y) * w + (cur.x - min_x);
        bool is_goal = false;

        if (any_unvisited_end) {
            is_goal = ap_is_end(local_map_peek_terrain(cur.x, cur.y)) &&
                      !local_map_is_explored(cur.x, cur.y);
        } else if (any_end) {
            is_goal = ap_is_end(local_map_peek_terrain(cur.x, cur.y)) &&
                      !(cur.x == sx && cur.y == sy);
        } else if (any_unvisited) {
            is_goal = ap_walkable(cur.x, cur.y) &&
                      !local_map_is_explored(cur.x, cur.y);
        } else if (any_frontier) {
            int d;
            for (d = 0; d < MC_DIR_COUNT; d++) {
                if (ap_unknown_at(cur.x + k8dx[d], cur.y + k8dy[d])) {
                    is_goal = true;
                    break;
                }
            }
        } else {
            is_goal = (cur.x == goal_x && cur.y == goal_y);
        }

        if (is_goal && !(cur.x == sx && cur.y == sy)) {
            found = true;
            fx = cur.x;
            fy = cur.y;
            (void)ci;
            break;
        }

        for (i = 0; i < 4; i++) {
            int nx = cur.x + k_moves[i].dx;
            int ny = cur.y + k_moves[i].dy;
            int ni;

            if (nx < min_x || nx > max_x || ny < min_y || ny > max_y) {
                continue;
            }
            if (!ap_walkable(nx, ny) && !(nx == sx && ny == sy)) {
                continue;
            }
            ni = (ny - min_y) * w + (nx - min_x);
            if (seen[ni]) {
                continue;
            }
            seen[ni] = true;
            came[ni] = ci;
            q[tail].x = nx;
            q[tail].y = ny;
            tail++;
        }
    }

    if (found) {
        int gi = (fy - min_y) * w + (fx - min_x);
        int si = (sy - min_y) * w + (sx - min_x);
        int prev = gi;

        while (came[prev] != si && came[prev] != -1) {
            prev = came[prev];
        }
        if (came[prev] == si) {
            int px = min_x + (prev % w);
            int py = min_y + (prev / w);
            *out_dx = px - sx;
            *out_dy = py - sy;
            free(q);
            free(came);
            free(seen);
            return true;
        }
    }

    free(q);
    free(came);
    free(seen);
    return false;
}

static bool ap_pick_scan(McScanDir *dir, int *radius)
{
    int r;
    int i;

    for (r = 1; r <= 2; r++) {
        mc_set_scan_radius(r);
        for (i = 0; i < MC_DIR_COUNT; i++) {
            McScanDir d = k_scan_order[i];
            int dx;
            int dy;
            int x;
            int y;

            mc_scan_offset(d, &dx, &dy);
            x = local_map_get_x() + dx;
            y = local_map_get_y() + dy;
            if (ap_unknown_at(x, y)) {
                *dir = d;
                *radius = r;
                return true;
            }
        }
    }
    return false;
}

static bool ap_pick_move(int mission_type, int *move_i)
{
    int dx = 0;
    int dy = 0;
    int i;

    if (mission_type == 3) {
        if (ap_bfs_step(0, 0, false, false, false, true, &dx, &dy)) {
            goto found;
        }
    }
    if (mission_type == 5) {
        if (ap_bfs_step(0, 0, false, false, true, false, &dx, &dy)) {
            goto found;
        }
    }
    if (mission_type == 8) {
        if (ap_bfs_step(0, 0, false, true, false, false, &dx, &dy)) {
            goto found;
        }
    }
    if (ap_bfs_step(0, 0, true, false, false, false, &dx, &dy)) {
        goto found;
    }
    return false;

found:
    for (i = 0; i < 4; i++) {
        if (k_moves[i].dx == dx && k_moves[i].dy == dy) {
            *move_i = i;
            return true;
        }
    }
    return false;
}

static long ap_shield_chunk(const VehDataType *vdt)
{
    long chunk = vdt->currEnergy / 10;

    if (chunk < 1000) {
        chunk = vdt->currEnergy / 2;
    }
    if (chunk >= vdt->currEnergy) {
        chunk = vdt->currEnergy / 2;
    }
    if (chunk < 1) {
        return 0;
    }
    return chunk;
}

ApRunResult ap_run(VehDataType *vdt, int mission_type, ApOptions *opt)
{
    printf("\nAuto-pilot ON (mission type %d). Press p to pause, q to quit.\n",
           mission_type);

    for (;;) {
        const char *why = NULL;
        char cmd = '\0';
        McScanDir sdir;
        int radius;
        int move_i;
        bool print_stats;
        bool print_map;
        long chunk;

        if (ap_poll_interrupt(&cmd)) {
            if (cmd == 'q') {
                printf("Auto-pilot stopped: quit.\n");
                return AP_RUN_QUIT;
            }
            printf("Auto-pilot stopped: toggled off.\n");
            return AP_RUN_PAUSED;
        }

        if (mc_is_mission_over()) {
            printf("Auto-pilot stopped: mission over.\n");
            return AP_RUN_PAUSED;
        }
        if (ap_goal_met(mission_type)) {
            printf("Auto-pilot stopped: mission objective achieved.\n");
            return AP_RUN_PAUSED;
        }
        if (ap_energy_hit(vdt, opt, &why) || ap_caps_hit(opt, &why)) {
            printf("Auto-pilot stopped: %s\n", why);
            return AP_RUN_PAUSED;
        }

        if (vdt->currShieldEnergy < AP_SHIELD_RESERVE &&
            vdt->currEnergy > AP_SHIELD_RESERVE) {
            chunk = ap_shield_chunk(vdt);
            if (chunk > 0) {
                print_stats = opt->stats_every || opt->stats_shield;
                printf("Auto-pilot: allocate %ld to shields.\n", chunk);
                (void)veh_action_allocate(vdt, chunk, print_stats);
                if (ap_after_action(opt, 'i', print_stats, false, false, &cmd)) {
                    goto interrupted;
                }
                continue;
            }
        }

        if (ap_pick_scan(&sdir, &radius)) {
            StatsCounters before = stats_get();

            if (opt->max_scans > 0 && before.scans_issued >= opt->max_scans) {
                printf("Auto-pilot stopped: scan limit reached.\n");
                return AP_RUN_PAUSED;
            }
            mc_set_scan_radius(radius);
            print_stats = opt->stats_every || opt->stats_scan;
            print_map = opt->map_scan;
            printf("Auto-pilot: scan %s radius %d.\n",
                   k_scan_name[sdir],
                   radius);
            veh_action_scan(vdt, sdir, k_scan_name[sdir], print_stats, print_map);
            if (ap_after_action(opt, 's', print_stats, print_map, false, &cmd)) {
                goto interrupted;
            }
            continue;
        }

        if (ap_pick_move(mission_type, &move_i)) {
            StatsCounters before = stats_get();

            if (opt->max_moves > 0 && before.moves_issued >= opt->max_moves) {
                printf("Auto-pilot stopped: move limit reached.\n");
                return AP_RUN_PAUSED;
            }
            print_stats = opt->stats_every || opt->stats_move;
            print_map = opt->map_move;
            printf("Auto-pilot: move dx=%d dy=%d.\n",
                   k_moves[move_i].dx,
                   k_moves[move_i].dy);
            veh_action_move(vdt,
                            k_moves[move_i].fn,
                            k_moves[move_i].dx,
                            k_moves[move_i].dy,
                            print_stats,
                            print_map);
            if (ap_after_action(opt, 'm', print_stats, false, print_map, &cmd)) {
                goto interrupted;
            }
            continue;
        }

        printf("Auto-pilot stopped: stuck / map exhausted.\n");
        return AP_RUN_PAUSED;

interrupted:
        if (cmd == 'q') {
            printf("Auto-pilot stopped: quit.\n");
            return AP_RUN_QUIT;
        }
        printf("Auto-pilot stopped: toggled off.\n");
        return AP_RUN_PAUSED;
    }
}

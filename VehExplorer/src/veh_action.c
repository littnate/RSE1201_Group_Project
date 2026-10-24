#include "veh_action.h"

#include "local_map.h"
#include "stats.h"
#include "ui_cli.h"

#include <stdio.h>

void veh_action_move(VehDataType *vdt,
                     bool (*move_fn)(VehDataType *),
                     int dx,
                     int dy,
                     bool print_stats,
                     bool print_explored)
{
    VehDataType before;
    char dest_terrain;
    bool ok;

    if (vdt == NULL || move_fn == NULL) {
        return;
    }

    before = *vdt;
    dest_terrain = local_map_peek_terrain_at_offset(dx, dy);
    ok = move_fn(vdt);
    stats_note_move(ok);

    if (mc_is_mission_over()) {
        return;
    }

    if (ok) {
        long move_e = vdt->totEnergyUsed - before.totEnergyUsed;
        long shld_e = vdt->totShieldEnergyUsed - before.totShieldEnergyUsed;

        stats_note_move_energy(dest_terrain, move_e, shld_e);
        local_map_move_success(dx, dy);
    } else if (dest_terrain == LM_TERRAIN_UNKNOWN) {
        local_map_mark_blocked_at_offset(dx, dy);
    }

    if (print_stats) {
        ui_print_veh_data(NULL, vdt);
    }
    if (print_explored) {
        local_map_print_explored();
    }
}

void veh_action_scan(VehDataType *vdt,
                     McScanDir dir,
                     const char *dir_name,
                     bool print_stats,
                     bool print_scanned)
{
    VehDataType before;
    char terrain;
    int dx;
    int dy;
    int radius;
    bool ok;
    long scan_e;
    long shld_e;

    if (vdt == NULL) {
        return;
    }

    before = *vdt;
    radius = mc_get_scan_radius();
    terrain = mc_scan(vdt, dir);
    ok = (terrain != '\0');
    stats_note_scan(ok);

    scan_e = vdt->totEnergyUsed - before.totEnergyUsed;
    shld_e = vdt->totShieldEnergyUsed - before.totShieldEnergyUsed;
    if (scan_e != 0 || shld_e != 0) {
        stats_note_scan_energy(dir, radius, scan_e, shld_e);
    }

    if (!ok) {
        mc_scan_offset(dir, &dx, &dy);
        local_map_mark_blocked_at_offset(dx, dy);
        printf("Scan %s (radius %d) failed.\n", dir_name, radius);
        if (print_stats) {
            ui_print_veh_data(NULL, vdt);
        }
        return;
    }

    mc_scan_offset(dir, &dx, &dy);
    local_map_record_scan(dx, dy, terrain);

    printf("Returned (scanned) terrain symbol is : '%c'\n", terrain);
    if (print_stats) {
        ui_print_veh_data(NULL, vdt);
    }
    if (print_scanned) {
        local_map_print_scanned();
    }
}

bool veh_action_allocate(VehDataType *vdt, long amount, bool print_stats)
{
    bool ok;

    if (vdt == NULL) {
        return false;
    }

    ok = mc_allocate_shield(vdt, amount);
    if (ok && print_stats) {
        ui_print_veh_data(NULL, vdt);
    }
    return ok;
}

#include "veh_action.h"

#include "local_map.h"
#include "stats.h"
#include "ui_cli.h"

#include <stdio.h>

void veh_action_move(VehDataType *vehicle,
                     char direction,
                     int dx,
                     int dy,
                     bool print_stats,
                     bool print_travelled_map)
{
    VehDataType before;
    char dest_terrain;
    bool ok = false;

    if (vehicle == NULL) {
        return;
    }

    before = *vehicle;

    dest_terrain = local_map_peek_terrain_at_offset(dx, dy);

    switch (direction) {
    case 'N':
    case 'n':
        ok = mission_control_move_north(vehicle);
        break;
    case 'S':
    case 's':
        ok = mission_control_move_south(vehicle);
        break;
    case 'E':
    case 'e':
        ok = mission_control_move_east(vehicle);
        break;
    case 'W':
    case 'w':
        ok = mission_control_move_west(vehicle);
        break;
    default:
        ok = false;
        break;
    }

    stats_count_move(ok);

    if (mission_control_is_mission_over()) {
        return;
    }

    if (ok) {
        long move_e = vehicle->totEnergyUsed - before.totEnergyUsed;
        long shld_e = vehicle->totShieldEnergyUsed - before.totShieldEnergyUsed;

        stats_add_move_energy(dest_terrain, move_e, shld_e);
        local_map_move_success(dx, dy);
    } else if (dest_terrain == LOCAL_MAP_TERRAIN_UNKNOWN) {
        local_map_mark_blocked_at_offset(dx, dy);
    }

    if (print_stats) {
        ui_print_veh_data(vehicle);
    }
    if (print_travelled_map) {
        local_map_print_explored();
    }
}

void veh_action_scan(VehDataType *vehicle,
                     MissionControlScanDir dir,
                     const char *scan_direction_name,
                     bool print_stats,
                     bool print_scanned_map)
{
    VehDataType before;
    char terrain;
    int dx;
    int dy;
    int radius;
    bool ok;
    long scan_e;
    long shld_e;

    if (vehicle == NULL) {
        return;
    }

    before = *vehicle;
    radius = mission_control_get_scan_radius();
    terrain = mission_control_scan(vehicle, dir);
    ok = (terrain != '\0');
    stats_count_scan(ok);

    scan_e = vehicle->totEnergyUsed - before.totEnergyUsed;
    shld_e = vehicle->totShieldEnergyUsed - before.totShieldEnergyUsed;
    if (scan_e != 0 || shld_e != 0) {
        stats_add_scan_energy(dir, radius, scan_e, shld_e);
    }

    if (!ok) {
        mission_control_scan_offset(dir, &dx, &dy);
        local_map_mark_blocked_at_offset(dx, dy);
        printf("Scan %s (radius %d) failed.\n", scan_direction_name, radius);
        if (print_stats) {
            ui_print_veh_data(vehicle);
        }
        return;
    }

    mission_control_scan_offset(dir, &dx, &dy);
    local_map_record_scan(dx, dy, terrain);

    printf("Returned (scanned) terrain symbol is : '%c'\n", terrain);
    if (print_stats) {
        ui_print_veh_data(vehicle);
    }
    if (print_scanned_map) {
        local_map_print_scanned();
    }
}

bool veh_action_allocate(VehDataType *vehicle, long amount, bool print_stats)
{
    bool ok;

    if (vehicle == NULL) {
        return false;
    }

    ok = mission_control_allocate_shield(vehicle, amount);
    if (ok && print_stats) {
        ui_print_veh_data(vehicle);
    }
    return ok;
}

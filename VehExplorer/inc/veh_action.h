#ifndef VEH_ACTION_H
#define VEH_ACTION_H

#include "SatComRelay.h"
#include "mission_control.h"

#include <stdbool.h>

/* Shared move/scan/shield used by manual keys and auto-pilot.
 * Print flags let auto-pilot skip the status line and the map. */

void veh_action_move(VehDataType *vehicle, /* was vdt */
                     bool (*move_fn)(VehDataType *),
                     int dx,
                     int dy,
                     bool print_stats,
                     bool print_travelled_map); /* was print_explored */

void veh_action_scan(VehDataType *vehicle, /* was vdt */
                     MissionControlScanDir dir,
                     const char *scan_direction_name, /* was dir_name */
                     bool print_stats,
                     bool print_scanned_map); /* was print_scanned */

bool veh_action_allocate(VehDataType *vehicle, long amount, bool print_stats); /* was vdt */

#endif /* VEH_ACTION_H */

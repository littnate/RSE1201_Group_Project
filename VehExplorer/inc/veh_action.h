#ifndef VEH_ACTION_H
#define VEH_ACTION_H

#include "SatComRelay.h"
#include "mission_control.h"

#include <stdbool.h>

/* Shared move/scan/shield used by manual keys and auto-pilot.
 * Print flags suppress tutor maps/stats for auto-pilot option gating. */

void veh_action_move(VehDataType *vdt,
                     bool (*move_fn)(VehDataType *),
                     int dx,
                     int dy,
                     bool print_stats,
                     bool print_explored);

void veh_action_scan(VehDataType *vdt,
                     McScanDir dir,
                     const char *dir_name,
                     bool print_stats,
                     bool print_scanned);

bool veh_action_allocate(VehDataType *vdt, long amount, bool print_stats);

#endif /* VEH_ACTION_H */

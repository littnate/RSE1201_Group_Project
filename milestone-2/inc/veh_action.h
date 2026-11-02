#ifndef VEH_ACTION_H
#define VEH_ACTION_H

#include "SatComRelay.h"
#include "mission_control.h"

#include <stdbool.h>

void veh_action_move(VehDataType *vehicle,
                     char direction,
                     int dx,
                     int dy,
                     bool print_stats,
                     bool print_travelled_map);

void veh_action_scan(VehDataType *vehicle,
                     MissionControlScanDir dir,
                     const char *scan_direction_name,
                     bool print_stats,
                     bool print_scanned_map);

bool veh_action_allocate(VehDataType *vehicle, long amount, bool print_stats);

#endif

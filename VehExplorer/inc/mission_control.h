#ifndef MISSION_CONTROL_H
#define MISSION_CONTROL_H

#include "SatComRelay.h"

#include <stdbool.h>

/* Eight compass directions matching the numpad scan layout. */
typedef enum {
    MISSION_CONTROL_DIR_NORTHWEST = 0,
    MISSION_CONTROL_DIR_NORTH,
    MISSION_CONTROL_DIR_NORTHEAST,
    MISSION_CONTROL_DIR_WEST,
    MISSION_CONTROL_DIR_EAST,
    MISSION_CONTROL_DIR_SOUTHWEST,
    MISSION_CONTROL_DIR_SOUTH,
    MISSION_CONTROL_DIR_SOUTHEAST,
    MISSION_CONTROL_DIR_COUNT
} MissionControlScanDir;

/* Move the vehicle in one cardinal direction and update *vehicle.
 * Returns true only when SatComRelay reports "Vehicle moved". */
bool mission_control_move_north(VehDataType *vehicle);
bool mission_control_move_south(VehDataType *vehicle);
bool mission_control_move_east(VehDataType *vehicle);
bool mission_control_move_west(VehDataType *vehicle);

/* Returns true once SatComRelay reports DESTROYED or MISSION OVER. */
bool mission_control_is_mission_over(void);

/* Check that 1 <= amount <= currEnergy, then transfer energy to shields. */
bool mission_control_allocate_shield(VehDataType *vehicle, long amount);

/* Read, toggle (1 <-> 2), or set the active scan radius. */
int mission_control_get_scan_radius(void);
int mission_control_toggle_scan_radius(void);
int mission_control_set_scan_radius(int radius);

/* Scan in direction dir at the active scan radius; returns '\0' on failure. */
char mission_control_scan(VehDataType *vehicle, MissionControlScanDir dir);

/* Compute relative grid offset (dx, dy) for dir at the active scan radius. */
void mission_control_scan_offset(MissionControlScanDir dir, int *dx, int *dy);

#endif /* MISSION_CONTROL_H */

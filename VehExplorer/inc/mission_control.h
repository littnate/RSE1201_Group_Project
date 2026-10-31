#ifndef MISSION_CONTROL_H
#define MISSION_CONTROL_H

#include "SatComRelay.h"

#include <stdbool.h>

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

/* Call the matching Move API and refresh *vehicle.
 * Returns true if the library reported a successful move
 * ("Vehicle moved …"); false on INVALID / IMPASSABLE / destroy (or I/O error).
 * Library stdout is captured then re-printed so the operator still sees it.
 * On vehicle destruction the library frees its state — call mission_control_is_mission_over()
 * and skip further vehicle APIs / Close. */
bool mission_control_move_north(VehDataType *vehicle); /* was vdt */
bool mission_control_move_south(VehDataType *vehicle); /* was vdt */
bool mission_control_move_east(VehDataType *vehicle); /* was vdt */
bool mission_control_move_west(VehDataType *vehicle); /* was vdt */

/* True after the library reported DESTROYED / MISSION OVER (do not Close). */
bool mission_control_is_mission_over(void);

/* Validate amount locally, then AllocateToShield and refresh *vehicle.
 * Returns false if rejected before calling the API (≤0 or > currEnergy). */
bool mission_control_allocate_shield(VehDataType *vehicle, long amount); /* was vdt */

/* Scan radius mode: starts at 1; toggle flips 1 <-> 2. */
int mission_control_get_scan_radius(void);
int mission_control_toggle_scan_radius(void);
int mission_control_set_scan_radius(int radius);

/* Call Scan* or Scan*2 for dir based on current radius; refresh *vehicle.
 * Returns the terrain char, or '\0' if the scan failed. */
char mission_control_scan(VehDataType *vehicle, MissionControlScanDir dir); /* was vdt */

/* Relative (dx, dy) for dir at the current scan radius (1 or 2).
 * North decreases y; East increases x. */
void mission_control_scan_offset(MissionControlScanDir dir, int *dx, int *dy);

#endif /* MISSION_CONTROL_H */

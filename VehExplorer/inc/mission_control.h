#ifndef MISSION_CONTROL_H
#define MISSION_CONTROL_H

#include "SatComRelay.h"

#include <stdbool.h>

typedef enum {
    MC_DIR_NW = 0,
    MC_DIR_N,
    MC_DIR_NE,
    MC_DIR_W,
    MC_DIR_E,
    MC_DIR_SW,
    MC_DIR_S,
    MC_DIR_SE,
    MC_DIR_COUNT
} McScanDir;

/* Call the matching Move API and refresh *vdt.
 * Returns true if the library reported a successful move
 * ("Vehicle moved …"); false on INVALID / IMPASSABLE / destroy (or I/O error).
 * Library stdout is captured then re-printed so the operator still sees it.
 * On vehicle destruction the library frees its state — call mc_is_mission_over()
 * and skip further vehicle APIs / Close. */
bool mc_move_north(VehDataType *vdt);
bool mc_move_south(VehDataType *vdt);
bool mc_move_east(VehDataType *vdt);
bool mc_move_west(VehDataType *vdt);

/* True after the library reported DESTROYED / MISSION OVER (do not Close). */
bool mc_is_mission_over(void);

/* Validate amount locally, then AllocateToShield and refresh *vdt.
 * Returns false if rejected before calling the API (≤0 or > currEnergy). */
bool mc_allocate_shield(VehDataType *vdt, long amount);

/* Scan radius mode: starts at 1; toggle flips 1 <-> 2. */
int mc_get_scan_radius(void);
int mc_toggle_scan_radius(void);

/* Call Scan* or Scan*2 for dir based on current radius; refresh *vdt.
 * Returns the terrain char, or '\0' if the scan failed. */
char mc_scan(VehDataType *vdt, McScanDir dir);

/* Relative (dx, dy) for dir at the current scan radius (1 or 2).
 * North decreases y; East increases x. */
void mc_scan_offset(McScanDir dir, int *dx, int *dy);

#endif /* MISSION_CONTROL_H */

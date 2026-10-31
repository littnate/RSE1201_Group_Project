#define _DEFAULT_SOURCE

#include "mission_control.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int scan_radius = 1; /* was g_scan_radius */
static bool vehicle_destroyed = false; /* was g_mission_over */

bool mission_control_is_mission_over(void)
{
    return vehicle_destroyed;
}

static bool library_text_says_destroyed(const char *library_text) /* was msg */
{
    return library_text != NULL
        && (strstr(library_text, "DESTROYED") != NULL
            || strstr(library_text, "MISSION OVER") != NULL);
}

/*
 * Capture Move API stdout into a temp file (not a pipe — the library's
 * destroy path can fault when stdout is a pipe), detect success vs
 * destruction, then re-print captured text to the real terminal.
 *
 * Success is ONLY "Vehicle moved" in the capture. Failed bumps may still
 * burn energy/shield, so energy deltas are not a reliable success signal.
 */
static bool capture_move_and_reprint(VehDataType *vehicle, VehDataType (*move_fn)(void)) /* was vdt */
{
    int saved_stdout = -1;
    FILE *captured_output = NULL; /* was tmp */
    char read_chunk[512]; /* was buf */
    size_t n;
    char library_text[4096]; /* was msg */
    size_t total = 0;

    if (vehicle == NULL || move_fn == NULL) {
        return false;
    }

    if (vehicle_destroyed) {
        printf("Mission already over — move ignored.\n");
        return false;
    }

    /*
     * Moving with shields down onto damaging terrain makes the library
     * DESTROY the vehicle and free its state; further API/Close calls
     * segfault. Refuse the call so the session stays stable (z/x/c/q).
     */
    if (vehicle->currShieldEnergy <= 0) {
        printf("Cannot move: shields are down (allocate with 'i' first). "
               "Moving onto damaging terrain would destroy the vehicle.\n");
        return false;
    }

    /* Totally depleted energy → library ends the mission and frees state. */
    if (vehicle->currEnergy <= 0) {
        printf("Cannot move: energy reserves are depleted. "
               "Status/maps/quit still available.\n");
        return false;
    }

    library_text[0] = '\0';
    captured_output = tmpfile();
    if (captured_output == NULL) {
        *vehicle = move_fn();
        /* Without a capture we cannot reliably tell success from a paid bump. */
        return false;
    }

    fflush(stdout);
    saved_stdout = dup(STDOUT_FILENO);
    if (saved_stdout < 0) {
        fclose(captured_output);
        *vehicle = move_fn();
        return false;
    }

    if (dup2(fileno(captured_output), STDOUT_FILENO) < 0) {
        close(saved_stdout);
        fclose(captured_output);
        *vehicle = move_fn();
        return false;
    }

    *vehicle = move_fn();
    fflush(stdout);
    fflush(captured_output);

    if (dup2(saved_stdout, STDOUT_FILENO) < 0) {
        /* Best-effort restore; still drain the temp capture. */
    }
    close(saved_stdout);

    rewind(captured_output);
    while ((n = fread(read_chunk, 1, sizeof read_chunk, captured_output)) > 0) {
        if (total + n < sizeof library_text) {
            memcpy(library_text + total, read_chunk, n);
            total += n;
            library_text[total] = '\0';
        }
    }
    fclose(captured_output);

    if (total > 0) {
        fputs(library_text, stdout);
        fflush(stdout);
    }

    if (library_text_says_destroyed(library_text)) {
        vehicle_destroyed = true;
        return false;
    }

    return strstr(library_text, "Vehicle moved") != NULL;
}

bool mission_control_move_north(VehDataType *vehicle) /* was vdt */
{
    return capture_move_and_reprint(vehicle, SCRelay_Signal_Veh_MoveUpNorth);
}

bool mission_control_move_south(VehDataType *vehicle) /* was vdt */
{
    return capture_move_and_reprint(vehicle, SCRelay_Signal_Veh_MoveDownSouth);
}

bool mission_control_move_east(VehDataType *vehicle) /* was vdt */
{
    return capture_move_and_reprint(vehicle, SCRelay_Signal_Veh_MoveRightEast);
}

bool mission_control_move_west(VehDataType *vehicle) /* was vdt */
{
    return capture_move_and_reprint(vehicle, SCRelay_Signal_Veh_MoveLeftWest);
}

bool mission_control_allocate_shield(VehDataType *vehicle, long amount) /* was vdt */
{
    if (vehicle == NULL) {
        return false;
    }

    if (vehicle_destroyed) {
        printf("Mission already over — shield allocate ignored.\n");
        return false;
    }

    if (amount <= 0) {
        printf("Error: Invalid energy qty '%ld' for shield allocation!\n",
               amount);
        return false;
    }

    if (amount > vehicle->currEnergy) {
        printf("Error: Insufficient energy reserves. Try reducing the "
               "quantity allocated to shield!\n");
        return false;
    }

    *vehicle = SCRelay_Signal_Veh_AllocateToShield(amount);
    return true;
}

int mission_control_get_scan_radius(void)
{
    return scan_radius;
}

int mission_control_toggle_scan_radius(void)
{
    scan_radius = (scan_radius == 1) ? 2 : 1;
    return scan_radius;
}

int mission_control_set_scan_radius(int radius)
{
    if (radius == 2) {
        scan_radius = 2;
    } else {
        scan_radius = 1;
    }
    return scan_radius;
}

char mission_control_scan(VehDataType *vehicle, MissionControlScanDir dir) /* was vdt */
{
    static const SCRelay_Signal_Veh_SCAN_Fn_Ptr scan_r1[MISSION_CONTROL_DIR_COUNT] = {
        SCRelay_Signal_Veh_ScanNorthWest,
        SCRelay_Signal_Veh_ScanNorth,
        SCRelay_Signal_Veh_ScanNorthEast,
        SCRelay_Signal_Veh_ScanWest,
        SCRelay_Signal_Veh_ScanEast,
        SCRelay_Signal_Veh_ScanSouthWest,
        SCRelay_Signal_Veh_ScanSouth,
        SCRelay_Signal_Veh_ScanSouthEast,
    };
    static const SCRelay_Signal_Veh_SCAN_Fn_Ptr scan_r2[MISSION_CONTROL_DIR_COUNT] = {
        SCRelay_Signal_Veh_ScanNorthWest2,
        SCRelay_Signal_Veh_ScanNorth2,
        SCRelay_Signal_Veh_ScanNorthEast2,
        SCRelay_Signal_Veh_ScanWest2,
        SCRelay_Signal_Veh_ScanEast2,
        SCRelay_Signal_Veh_ScanSouthWest2,
        SCRelay_Signal_Veh_ScanSouth2,
        SCRelay_Signal_Veh_ScanSouthEast2,
    };

    SCRelay_Signal_Veh_SCAN_Fn_Ptr scan_fn;

    if (vehicle == NULL || dir < 0 || dir >= MISSION_CONTROL_DIR_COUNT) {
        return '\0';
    }

    if (vehicle_destroyed) {
        printf("Mission already over — scan ignored.\n");
        return '\0';
    }

    if (vehicle->currEnergy <= 0) {
        printf("Cannot scan: energy reserves are depleted. "
               "Status/maps/quit still available.\n");
        return '\0';
    }

    scan_fn = (scan_radius == 2) ? scan_r2[dir] : scan_r1[dir];
    return scan_fn(vehicle);
}

void mission_control_scan_offset(MissionControlScanDir dir, int *dx, int *dy)
{
    static const int base_dx[MISSION_CONTROL_DIR_COUNT] = {
        -1, 0, 1, -1, 1, -1, 0, 1,
    };
    static const int base_dy[MISSION_CONTROL_DIR_COUNT] = {
        -1, -1, -1, 0, 0, 1, 1, 1,
    };
    int r = scan_radius;

    if (dx == NULL || dy == NULL || dir < 0 || dir >= MISSION_CONTROL_DIR_COUNT) {
        return;
    }

    *dx = base_dx[dir] * r;
    *dy = base_dy[dir] * r;
}

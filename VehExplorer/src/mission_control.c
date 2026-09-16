#define _DEFAULT_SOURCE

#include "mission_control.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_scan_radius = 1;
static bool g_mission_over = false;

bool mc_is_mission_over(void)
{
    return g_mission_over;
}

static bool message_means_destroyed(const char *msg)
{
    return msg != NULL
        && (strstr(msg, "DESTROYED") != NULL
            || strstr(msg, "MISSION OVER") != NULL);
}

/*
 * Capture Move API stdout into a temp file (not a pipe — the library's
 * destroy path can fault when stdout is a pipe), detect success vs
 * destruction, then re-print captured text to the real terminal.
 *
 * Success is ONLY "Vehicle moved" in the capture. Failed bumps may still
 * burn energy/shield, so energy deltas are not a reliable success signal.
 */
static bool apply_move(VehDataType *vdt, VehDataType (*move_fn)(void))
{
    int saved_stdout = -1;
    FILE *tmp = NULL;
    char buf[512];
    size_t n;
    char msg[4096];
    size_t total = 0;

    if (vdt == NULL || move_fn == NULL) {
        return false;
    }

    if (g_mission_over) {
        printf("Mission already over — move ignored.\n");
        return false;
    }

    /*
     * Moving with shields down onto damaging terrain makes the library
     * DESTROY the vehicle and free its state; further API/Close calls
     * segfault. Refuse the call so the session stays stable (z/x/c/q).
     */
    if (vdt->currShieldEnergy <= 0) {
        printf("Cannot move: shields are down (allocate with 'i' first). "
               "Moving onto damaging terrain would destroy the vehicle.\n");
        return false;
    }

    /* Totally depleted energy → library ends the mission and frees state. */
    if (vdt->currEnergy <= 0) {
        printf("Cannot move: energy reserves are depleted. "
               "Status/maps/quit still available.\n");
        return false;
    }

    msg[0] = '\0';
    tmp = tmpfile();
    if (tmp == NULL) {
        *vdt = move_fn();
        /* Without a capture we cannot reliably tell success from a paid bump. */
        return false;
    }

    fflush(stdout);
    saved_stdout = dup(STDOUT_FILENO);
    if (saved_stdout < 0) {
        fclose(tmp);
        *vdt = move_fn();
        return false;
    }

    if (dup2(fileno(tmp), STDOUT_FILENO) < 0) {
        close(saved_stdout);
        fclose(tmp);
        *vdt = move_fn();
        return false;
    }

    *vdt = move_fn();
    fflush(stdout);
    fflush(tmp);

    if (dup2(saved_stdout, STDOUT_FILENO) < 0) {
        /* Best-effort restore; still drain the temp capture. */
    }
    close(saved_stdout);

    rewind(tmp);
    while ((n = fread(buf, 1, sizeof buf, tmp)) > 0) {
        if (total + n < sizeof msg) {
            memcpy(msg + total, buf, n);
            total += n;
            msg[total] = '\0';
        }
    }
    fclose(tmp);

    if (total > 0) {
        fputs(msg, stdout);
        fflush(stdout);
    }

    if (message_means_destroyed(msg)) {
        g_mission_over = true;
        return false;
    }

    return strstr(msg, "Vehicle moved") != NULL;
}

bool mc_move_north(VehDataType *vdt)
{
    return apply_move(vdt, SCRelay_Signal_Veh_MoveUpNorth);
}

bool mc_move_south(VehDataType *vdt)
{
    return apply_move(vdt, SCRelay_Signal_Veh_MoveDownSouth);
}

bool mc_move_east(VehDataType *vdt)
{
    return apply_move(vdt, SCRelay_Signal_Veh_MoveRightEast);
}

bool mc_move_west(VehDataType *vdt)
{
    return apply_move(vdt, SCRelay_Signal_Veh_MoveLeftWest);
}

bool mc_allocate_shield(VehDataType *vdt, long amount)
{
    if (vdt == NULL) {
        return false;
    }

    if (g_mission_over) {
        printf("Mission already over — shield allocate ignored.\n");
        return false;
    }

    if (amount <= 0) {
        printf("Error: Invalid energy qty '%ld' for shield allocation!\n",
               amount);
        return false;
    }

    if (amount > vdt->currEnergy) {
        printf("Error: Insufficient energy reserves. Try reducing the "
               "quantity allocated to shield!\n");
        return false;
    }

    *vdt = SCRelay_Signal_Veh_AllocateToShield(amount);
    return true;
}

int mc_get_scan_radius(void)
{
    return g_scan_radius;
}

int mc_toggle_scan_radius(void)
{
    g_scan_radius = (g_scan_radius == 1) ? 2 : 1;
    return g_scan_radius;
}

char mc_scan(VehDataType *vdt, McScanDir dir)
{
    static const SCRelay_Signal_Veh_SCAN_Fn_Ptr scan_r1[MC_DIR_COUNT] = {
        SCRelay_Signal_Veh_ScanNorthWest,
        SCRelay_Signal_Veh_ScanNorth,
        SCRelay_Signal_Veh_ScanNorthEast,
        SCRelay_Signal_Veh_ScanWest,
        SCRelay_Signal_Veh_ScanEast,
        SCRelay_Signal_Veh_ScanSouthWest,
        SCRelay_Signal_Veh_ScanSouth,
        SCRelay_Signal_Veh_ScanSouthEast,
    };
    static const SCRelay_Signal_Veh_SCAN_Fn_Ptr scan_r2[MC_DIR_COUNT] = {
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

    if (vdt == NULL || dir < 0 || dir >= MC_DIR_COUNT) {
        return '\0';
    }

    if (g_mission_over) {
        printf("Mission already over — scan ignored.\n");
        return '\0';
    }

    if (vdt->currEnergy <= 0) {
        printf("Cannot scan: energy reserves are depleted. "
               "Status/maps/quit still available.\n");
        return '\0';
    }

    scan_fn = (g_scan_radius == 2) ? scan_r2[dir] : scan_r1[dir];
    return scan_fn(vdt);
}

void mc_scan_offset(McScanDir dir, int *dx, int *dy)
{
    static const int base_dx[MC_DIR_COUNT] = {
        -1, 0, 1, -1, 1, -1, 0, 1,
    };
    static const int base_dy[MC_DIR_COUNT] = {
        -1, -1, -1, 0, 0, 1, 1, 1,
    };
    int r = g_scan_radius;

    if (dx == NULL || dy == NULL || dir < 0 || dir >= MC_DIR_COUNT) {
        return;
    }

    *dx = base_dx[dir] * r;
    *dy = base_dy[dir] * r;
}

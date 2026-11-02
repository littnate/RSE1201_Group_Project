#define _DEFAULT_SOURCE

#include "mission_control.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int scan_radius = 1;
static bool vehicle_destroyed = false;

bool mission_control_is_mission_over(void)
{
    return vehicle_destroyed;
}

static bool library_text_says_destroyed(const char *library_text)
{
    if (library_text == NULL) {
        return false;
    }
    if (strstr(library_text, "DESTROYED") != NULL) {
        return true;
    }
    if (strstr(library_text, "MISSION OVER") != NULL) {
        return true;
    }
    return false;
}

static VehDataType call_relay_move(char direction, VehDataType fallback)
{
    switch (direction) {
    case 'N':
        return SCRelay_Signal_Veh_MoveUpNorth();
    case 'S':
        return SCRelay_Signal_Veh_MoveDownSouth();
    case 'E':
        return SCRelay_Signal_Veh_MoveRightEast();
    case 'W':
        return SCRelay_Signal_Veh_MoveLeftWest();
    default:
        return fallback;
    }
}

static bool capture_move_and_reprint(VehDataType *vehicle, char direction)
{
    int saved_stdout = -1;
    FILE *captured_output = NULL;
    char read_chunk[512];
    size_t n;
    char library_text[4096];
    size_t captured_len = 0;

    if (vehicle == NULL) {
        return false;
    }

    if (vehicle_destroyed) {
        printf("Mission already over — move ignored.\n");
        return false;
    }

    if (vehicle->currShieldEnergy <= 0) {
        printf("Cannot move: shields are down (allocate with 'i' first). "
               "Moving onto damaging terrain would destroy the vehicle.\n");
        return false;
    }

    if (vehicle->currEnergy <= 0) {
        printf("Cannot move: energy reserves are depleted. "
               "Status/maps/quit still available.\n");
        return false;
    }

    library_text[0] = '\0';
    captured_output = tmpfile();
    if (captured_output == NULL) {
        *vehicle = call_relay_move(direction, *vehicle);
        return false;
    }

    fflush(stdout);
    saved_stdout = dup(STDOUT_FILENO);
    if (saved_stdout < 0) {
        fclose(captured_output);
        *vehicle = call_relay_move(direction, *vehicle);
        return false;
    }

    if (dup2(fileno(captured_output), STDOUT_FILENO) < 0) {
        close(saved_stdout);
        fclose(captured_output);
        *vehicle = call_relay_move(direction, *vehicle);
        return false;
    }

    *vehicle = call_relay_move(direction, *vehicle);
    fflush(stdout);
    fflush(captured_output);

    if (dup2(saved_stdout, STDOUT_FILENO) < 0) {

    }
    close(saved_stdout);

    rewind(captured_output);
    while ((n = fread(read_chunk, 1, sizeof(read_chunk), captured_output)) > 0) {
        if (captured_len + n < sizeof(library_text)) {
            memcpy(library_text + captured_len, read_chunk, n);
            captured_len += n;
            library_text[captured_len] = '\0';
        }
    }
    fclose(captured_output);

    if (captured_len > 0) {
        fputs(library_text, stdout);
        fflush(stdout);
    }

    if (library_text_says_destroyed(library_text)) {
        vehicle_destroyed = true;
        return false;
    }

    return strstr(library_text, "Vehicle moved") != NULL;
}

bool mission_control_move_north(VehDataType *vehicle)
{
    return capture_move_and_reprint(vehicle, 'N');
}

bool mission_control_move_south(VehDataType *vehicle)
{
    return capture_move_and_reprint(vehicle, 'S');
}

bool mission_control_move_east(VehDataType *vehicle)
{
    return capture_move_and_reprint(vehicle, 'E');
}

bool mission_control_move_west(VehDataType *vehicle)
{
    return capture_move_and_reprint(vehicle, 'W');
}

bool mission_control_allocate_shield(VehDataType *vehicle, long amount)
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
    if (scan_radius == 1) {
        scan_radius = 2;
    } else {
        scan_radius = 1;
    }
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

char mission_control_scan(VehDataType *vehicle, MissionControlScanDir dir)
{
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

    if (scan_radius == 2) {
        switch (dir) {
        case MISSION_CONTROL_DIR_NORTHWEST:
            return SCRelay_Signal_Veh_ScanNorthWest2(vehicle);
        case MISSION_CONTROL_DIR_NORTH:
            return SCRelay_Signal_Veh_ScanNorth2(vehicle);
        case MISSION_CONTROL_DIR_NORTHEAST:
            return SCRelay_Signal_Veh_ScanNorthEast2(vehicle);
        case MISSION_CONTROL_DIR_WEST:
            return SCRelay_Signal_Veh_ScanWest2(vehicle);
        case MISSION_CONTROL_DIR_EAST:
            return SCRelay_Signal_Veh_ScanEast2(vehicle);
        case MISSION_CONTROL_DIR_SOUTHWEST:
            return SCRelay_Signal_Veh_ScanSouthWest2(vehicle);
        case MISSION_CONTROL_DIR_SOUTH:
            return SCRelay_Signal_Veh_ScanSouth2(vehicle);
        case MISSION_CONTROL_DIR_SOUTHEAST:
            return SCRelay_Signal_Veh_ScanSouthEast2(vehicle);
        default:
            return '\0';
        }
    } else {
        switch (dir) {
        case MISSION_CONTROL_DIR_NORTHWEST:
            return SCRelay_Signal_Veh_ScanNorthWest(vehicle);
        case MISSION_CONTROL_DIR_NORTH:
            return SCRelay_Signal_Veh_ScanNorth(vehicle);
        case MISSION_CONTROL_DIR_NORTHEAST:
            return SCRelay_Signal_Veh_ScanNorthEast(vehicle);
        case MISSION_CONTROL_DIR_WEST:
            return SCRelay_Signal_Veh_ScanWest(vehicle);
        case MISSION_CONTROL_DIR_EAST:
            return SCRelay_Signal_Veh_ScanEast(vehicle);
        case MISSION_CONTROL_DIR_SOUTHWEST:
            return SCRelay_Signal_Veh_ScanSouthWest(vehicle);
        case MISSION_CONTROL_DIR_SOUTH:
            return SCRelay_Signal_Veh_ScanSouth(vehicle);
        case MISSION_CONTROL_DIR_SOUTHEAST:
            return SCRelay_Signal_Veh_ScanSouthEast(vehicle);
        default:
            return '\0';
        }
    }
}

void mission_control_scan_offset(MissionControlScanDir dir, int *dx, int *dy)
{
    static const int base_dx[MISSION_CONTROL_DIR_COUNT] = {
        -1, 0, 1, -1, 1, -1, 0, 1,
    };
    static const int base_dy[MISSION_CONTROL_DIR_COUNT] = {
        -1, -1, -1, 0, 0, 1, 1, 1,
    };
    int radius = scan_radius;

    if (dx == NULL || dy == NULL || dir < 0 || dir >= MISSION_CONTROL_DIR_COUNT) {
        return;
    }

    *dx = base_dx[dir] * radius;
    *dy = base_dy[dir] * radius;
}

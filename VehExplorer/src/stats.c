#include "stats.h"

#include "local_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* One row in the per-terrain movement energy table. */
typedef struct {
    char terrain;
    long move_energy;
    long shield_energy;
    unsigned long samples;
} TerrainEnergyRow;

/* One row in the per-direction/radius scan energy table. */
typedef struct {
    MissionControlScanDir dir;
    int radius;
    long scan_energy;
    long shield_energy;
    unsigned long samples;
} ScanEnergyRow;

/* Module state: command counters and dynamic energy tables. */
static StatsCounters move_scan_counts;
static TerrainEnergyRow *terrain_energy_rows;
static size_t terrain_energy_count;
static size_t terrain_energy_capacity;
static ScanEnergyRow *scan_energy_rows;
static size_t scan_energy_count;
static size_t scan_energy_capacity;

/* Convert a scan direction enum into its short compass label. */
static const char *scan_direction_name(MissionControlScanDir dir)
{
    static const char *names[MISSION_CONTROL_DIR_COUNT] = {
        "NW", "N", "NE", "W", "E", "SW", "S", "SE",
    };

    if (dir < 0 || dir >= MISSION_CONTROL_DIR_COUNT) {
        return "?";
    }
    return names[dir];
}

/* Free both dynamically allocated energy tables and reset their sizes. */
static void free_energy_tables(void)
{
    free(terrain_energy_rows);
    free(scan_energy_rows);
    terrain_energy_rows = NULL;
    scan_energy_rows = NULL;
    terrain_energy_count = 0;
    terrain_energy_capacity = 0;
    scan_energy_count = 0;
    scan_energy_capacity = 0;
}

/* Clear all counters and energy tables at session start. */
void stats_reset(void)
{
    free_energy_tables();
    move_scan_counts.moves_issued = 0;
    move_scan_counts.moves_succeeded = 0;
    move_scan_counts.scans_issued = 0;
    move_scan_counts.scans_succeeded = 0;
}

/* Increment move counters after a move attempt. */
void stats_count_move(bool success)
{
    move_scan_counts.moves_issued++;
    if (success) {
        move_scan_counts.moves_succeeded++;
    }
}

/* Increment scan counters after a scan attempt. */
void stats_count_scan(bool success)
{
    move_scan_counts.scans_issued++;
    if (success) {
        move_scan_counts.scans_succeeded++;
    }
}

/* Add observed movement energy cost to the matching terrain row (or append one). */
void stats_add_move_energy(char terrain, long move_energy, long shield_energy)
{
    size_t i;
    TerrainEnergyRow *row;

    if (terrain == '\0') {
        terrain = LOCAL_MAP_TERRAIN_UNKNOWN;
    }

    /* Update existing row if we have already stepped on this terrain symbol */
    for (i = 0; i < terrain_energy_count; i++) {
        if (terrain_energy_rows[i].terrain == terrain) {
            terrain_energy_rows[i].move_energy += move_energy;
            terrain_energy_rows[i].shield_energy += shield_energy;
            terrain_energy_rows[i].samples++;
            return;
        }
    }

    /* Grow table capacity when full */
    if (terrain_energy_count >= terrain_energy_capacity) {
        size_t new_cap;
        TerrainEnergyRow *grown;

        if (terrain_energy_capacity == 0) {
            new_cap = 8;
        } else {
            new_cap = terrain_energy_capacity * 2;
        }

        grown = realloc(terrain_energy_rows, new_cap * sizeof(*grown));
        if (grown == NULL) {
            fprintf(stderr, "stats: out of memory\n");
            return;
        }
        terrain_energy_rows = grown;
        terrain_energy_capacity = new_cap;
    }

    /* Append a new row for this newly seen terrain symbol */
    row = &terrain_energy_rows[terrain_energy_count++];
    row->terrain = terrain;
    row->move_energy = move_energy;
    row->shield_energy = shield_energy;
    row->samples = 1;
}

/* Add observed scan energy cost to the matching (dir, radius) row (or append one). */
void stats_add_scan_energy(MissionControlScanDir dir,
                           int radius,
                           long scan_energy,
                           long shield_energy)
{
    size_t i;
    ScanEnergyRow *row;

    if (dir < 0 || dir >= MISSION_CONTROL_DIR_COUNT) {
        return;
    }
    if (radius != 1 && radius != 2) {
        return;
    }

    /* Update existing row if this (dir, radius) pair was already recorded */
    for (i = 0; i < scan_energy_count; i++) {
        if (scan_energy_rows[i].dir == dir && scan_energy_rows[i].radius == radius) {
            scan_energy_rows[i].scan_energy += scan_energy;
            scan_energy_rows[i].shield_energy += shield_energy;
            scan_energy_rows[i].samples++;
            return;
        }
    }

    /* Grow table capacity when full */
    if (scan_energy_count >= scan_energy_capacity) {
        size_t new_cap;
        ScanEnergyRow *grown;

        if (scan_energy_capacity == 0) {
            new_cap = 8;
        } else {
            new_cap = scan_energy_capacity * 2;
        }

        grown = realloc(scan_energy_rows, new_cap * sizeof(*grown));
        if (grown == NULL) {
            fprintf(stderr, "stats: out of memory\n");
            return;
        }
        scan_energy_rows = grown;
        scan_energy_capacity = new_cap;
    }

    /* Append a new row for this (dir, radius) pair */
    row = &scan_energy_rows[scan_energy_count++];
    row->dir = dir;
    row->radius = radius;
    row->scan_energy = scan_energy;
    row->shield_energy = shield_energy;
    row->samples = 1;
}

/* Return a copy of the current move/scan counters. */
StatsCounters stats_get(void)
{
    return move_scan_counts;
}

/* Print issued vs successful move and scan counts. */
static void print_command_stats(void)
{
    printf("Moves issued : %lu, Successful moves : %lu\n",
           move_scan_counts.moves_issued,
           move_scan_counts.moves_succeeded);
    printf("Scans issued : %lu, Successful scans : %lu\n",
           move_scan_counts.scans_issued,
           move_scan_counts.scans_succeeded);
}

/* Print how many cells of each terrain symbol were scanned and explored. */
static void print_terrain_counts(void)
{
    LocalMapTerrainCount counts[64];
    unsigned max_entries = (unsigned)(sizeof(counts) / sizeof(counts[0]));
    unsigned n;
    unsigned i;

    printf("\n[ Terrain Types Scanned / Explored ]\n");
    n = local_map_terrain_counts(counts, max_entries);
    if (n == 0) {
        printf("(none recorded)\n");
        return;
    }

    printf("%-8s %10s %10s\n", "Terrain", "Scanned", "Explored");
    for (i = 0; i < n; i++) {
        printf("  '%c'    %10u %10u\n",
               counts[i].terrain,
               counts[i].scanned,
               counts[i].explored);
    }
}

/* Print observed movement and shield energy totals grouped by terrain. */
static void print_move_energy_by_terrain(void)
{
    size_t i;

    printf("\n[ Move / Shield Energy by Destination Terrain ]\n");
    if (terrain_energy_count == 0) {
        printf("(none recorded — need successful moves)\n");
        return;
    }

    printf("%-8s %12s %12s %8s\n",
           "Terrain",
           "MoveEnrg",
           "ShldEnrg",
           "Samples");
    for (i = 0; i < terrain_energy_count; i++) {
        printf("  '%c'    %12ld %12ld %8lu\n",
               terrain_energy_rows[i].terrain,
               terrain_energy_rows[i].move_energy,
               terrain_energy_rows[i].shield_energy,
               terrain_energy_rows[i].samples);
    }
}

/* Print observed scan and shield energy totals grouped by direction and radius. */
static void print_scan_energy(void)
{
    size_t i;

    printf("\n[ Scan Energy by Direction / Radius ]\n");
    if (scan_energy_count == 0) {
        printf("(none recorded — need scans that consumed energy)\n");
        return;
    }

    printf("%-4s %6s %12s %12s %8s\n",
           "Dir",
           "Radius",
           "ScanEnrg",
           "ShldEnrg",
           "Samples");
    for (i = 0; i < scan_energy_count; i++) {
        printf("%-4s %6d %12ld %12ld %8lu\n",
               scan_direction_name(scan_energy_rows[i].dir),
               scan_energy_rows[i].radius,
               scan_energy_rows[i].scan_energy,
               scan_energy_rows[i].shield_energy,
               scan_energy_rows[i].samples);
    }
}

/* Print the full Stage 3 Exploration Report at the end of the session. */
void stats_print_summary(const VehDataType *vehicle)
{
    printf("\n");
    printf("-------------------------------------\n");
    printf("-- Exploration Report --\n");
    printf("-------------------------------------\n");

    printf("\nTotal no. of scans taken : %lu\n", move_scan_counts.scans_succeeded);
    printf("Total no. of movements (steps) taken : %lu\n", move_scan_counts.moves_succeeded);

    if (vehicle != NULL) {
        printf("\n[ Energy Report ]\n");
        printf("Initial Energy : %ld\n", vehicle->initialEnergy);
        printf("Total Energy Used : %ld\n", vehicle->totEnergyUsed);
        printf("Total Shield Energy Used : %ld\n", vehicle->totShieldEnergyUsed);
        printf("Energy Left : %ld\n", vehicle->currEnergy);
        printf("Shield Energy Left : %ld\n", vehicle->currShieldEnergy);
    }

    printf("\n[ Command Counters ]\n");
    print_command_stats();

    print_terrain_counts();
    print_move_energy_by_terrain();
    print_scan_energy();

    printf("\n-------------------------------------\n");
    printf("-- End of Exploration Report --\n");
    printf("-------------------------------------\n");
}

#include "stats.h"

#include "local_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char terrain;
    long move_energy;
    long shield_energy;
    unsigned long samples;
} TerrainEnergyRow;

typedef struct {
    McScanDir dir;
    int radius;
    long scan_energy;
    long shield_energy;
    unsigned long samples;
} ScanEnergyRow;

static StatsCounters g_stats;
static TerrainEnergyRow *g_terrain_energy;
static size_t g_terrain_energy_n;
static size_t g_terrain_energy_cap;
static ScanEnergyRow *g_scan_energy;
static size_t g_scan_energy_n;
static size_t g_scan_energy_cap;

static const char *dir_name(McScanDir dir)
{
    static const char *names[MC_DIR_COUNT] = {
        "NW", "N", "NE", "W", "E", "SW", "S", "SE",
    };

    if (dir < 0 || dir >= MC_DIR_COUNT) {
        return "?";
    }
    return names[dir];
}

static void free_tables(void)
{
    free(g_terrain_energy);
    free(g_scan_energy);
    g_terrain_energy = NULL;
    g_scan_energy = NULL;
    g_terrain_energy_n = 0;
    g_terrain_energy_cap = 0;
    g_scan_energy_n = 0;
    g_scan_energy_cap = 0;
}

void stats_reset(void)
{
    free_tables();
    g_stats.moves_issued = 0;
    g_stats.moves_ok = 0;
    g_stats.scans_issued = 0;
    g_stats.scans_ok = 0;
}

void stats_note_move(bool success)
{
    g_stats.moves_issued++;
    if (success) {
        g_stats.moves_ok++;
    }
}

void stats_note_scan(bool success)
{
    g_stats.scans_issued++;
    if (success) {
        g_stats.scans_ok++;
    }
}

void stats_note_move_energy(char terrain, long move_energy, long shield_energy)
{
    size_t i;
    TerrainEnergyRow *row;

    if (terrain == '\0') {
        terrain = LM_TERRAIN_UNKNOWN;
    }

    for (i = 0; i < g_terrain_energy_n; i++) {
        if (g_terrain_energy[i].terrain == terrain) {
            g_terrain_energy[i].move_energy += move_energy;
            g_terrain_energy[i].shield_energy += shield_energy;
            g_terrain_energy[i].samples++;
            return;
        }
    }

    if (g_terrain_energy_n >= g_terrain_energy_cap) {
        size_t new_cap = (g_terrain_energy_cap == 0) ? 8 : g_terrain_energy_cap * 2;
        TerrainEnergyRow *grown =
            realloc(g_terrain_energy, new_cap * sizeof *grown);
        if (grown == NULL) {
            fprintf(stderr, "stats: out of memory\n");
            return;
        }
        g_terrain_energy = grown;
        g_terrain_energy_cap = new_cap;
    }

    row = &g_terrain_energy[g_terrain_energy_n++];
    row->terrain = terrain;
    row->move_energy = move_energy;
    row->shield_energy = shield_energy;
    row->samples = 1;
}

void stats_note_scan_energy(McScanDir dir,
                            int radius,
                            long scan_energy,
                            long shield_energy)
{
    size_t i;
    ScanEnergyRow *row;

    if (dir < 0 || dir >= MC_DIR_COUNT) {
        return;
    }
    if (radius != 1 && radius != 2) {
        return;
    }

    for (i = 0; i < g_scan_energy_n; i++) {
        if (g_scan_energy[i].dir == dir && g_scan_energy[i].radius == radius) {
            g_scan_energy[i].scan_energy += scan_energy;
            g_scan_energy[i].shield_energy += shield_energy;
            g_scan_energy[i].samples++;
            return;
        }
    }

    if (g_scan_energy_n >= g_scan_energy_cap) {
        size_t new_cap = (g_scan_energy_cap == 0) ? 8 : g_scan_energy_cap * 2;
        ScanEnergyRow *grown = realloc(g_scan_energy, new_cap * sizeof *grown);
        if (grown == NULL) {
            fprintf(stderr, "stats: out of memory\n");
            return;
        }
        g_scan_energy = grown;
        g_scan_energy_cap = new_cap;
    }

    row = &g_scan_energy[g_scan_energy_n++];
    row->dir = dir;
    row->radius = radius;
    row->scan_energy = scan_energy;
    row->shield_energy = shield_energy;
    row->samples = 1;
}

StatsCounters stats_get(void)
{
    return g_stats;
}

static void print_command_stats(void)
{
    printf("Moves issued : %lu, Successful moves : %lu\n",
           g_stats.moves_issued,
           g_stats.moves_ok);
    printf("Scans issued : %lu, Successful scans : %lu\n",
           g_stats.scans_issued,
           g_stats.scans_ok);
}

static void print_terrain_counts(void)
{
    LocalMapTerrainCount counts[64];
    unsigned n;
    unsigned i;

    printf("\n[ Terrain Types Scanned / Explored ]\n");
    n = local_map_terrain_counts(counts,
                                (unsigned)(sizeof counts / sizeof counts[0]));
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

static void print_move_energy_by_terrain(void)
{
    size_t i;

    printf("\n[ Move / Shield Energy by Destination Terrain ]\n");
    if (g_terrain_energy_n == 0) {
        printf("(none recorded — need successful moves)\n");
        return;
    }

    printf("%-8s %12s %12s %8s\n",
           "Terrain",
           "MoveEnrg",
           "ShldEnrg",
           "Samples");
    for (i = 0; i < g_terrain_energy_n; i++) {
        printf("  '%c'    %12ld %12ld %8lu\n",
               g_terrain_energy[i].terrain,
               g_terrain_energy[i].move_energy,
               g_terrain_energy[i].shield_energy,
               g_terrain_energy[i].samples);
    }
}

static void print_scan_energy(void)
{
    size_t i;

    printf("\n[ Scan Energy by Direction / Radius ]\n");
    if (g_scan_energy_n == 0) {
        printf("(none recorded — need scans that consumed energy)\n");
        return;
    }

    printf("%-4s %6s %12s %12s %8s\n",
           "Dir",
           "Radius",
           "ScanEnrg",
           "ShldEnrg",
           "Samples");
    for (i = 0; i < g_scan_energy_n; i++) {
        printf("%-4s %6d %12ld %12ld %8lu\n",
               dir_name(g_scan_energy[i].dir),
               g_scan_energy[i].radius,
               g_scan_energy[i].scan_energy,
               g_scan_energy[i].shield_energy,
               g_scan_energy[i].samples);
    }
}

void stats_print_summary(const VehDataType *vdt)
{
    printf("\n");
    printf("-------------------------------------\n");
    printf("-- Exploration Report --\n");
    printf("-------------------------------------\n");

    printf("\nTotal no. of scans taken : %lu\n", g_stats.scans_ok);
    printf("Total no. of movements (steps) taken : %lu\n", g_stats.moves_ok);

    if (vdt != NULL) {
        printf("\n[ Energy Report ]\n");
        printf("Initial Energy : %ld\n", vdt->initialEnergy);
        printf("Total Energy Used : %ld\n", vdt->totEnergyUsed);
        printf("Total Shield Energy Used : %ld\n", vdt->totShieldEnergyUsed);
        printf("Energy Left : %ld\n", vdt->currEnergy);
        printf("Shield Energy Left : %ld\n", vdt->currShieldEnergy);
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

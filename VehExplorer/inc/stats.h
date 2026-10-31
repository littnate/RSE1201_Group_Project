#ifndef STATS_H
#define STATS_H

#include "SatComRelay.h"
#include "mission_control.h"

#include <stdbool.h>

typedef struct {
    unsigned long moves_issued;
    unsigned long moves_succeeded; /* was moves_ok */
    unsigned long scans_issued;
    unsigned long scans_succeeded; /* was scans_ok */
} StatsCounters;

void stats_reset(void);

void stats_count_move(bool success);
void stats_count_scan(bool success);

/*
 * Attribute observed energy deltas from a successful move to destination
 * terrain (use LOCAL_MAP_TERRAIN_UNKNOWN / '?' when the cell was never scanned).
 */
void stats_add_move_energy(char terrain,
                            long move_energy,
                            long shield_energy);

/*
 * Attribute observed energy deltas from a scan attempt to (dir, radius).
 * Call for any attempt that consumed energy (success or fail).
 */
void stats_add_scan_energy(MissionControlScanDir dir,
                            int radius,
                            long scan_energy,
                            long shield_energy);

StatsCounters stats_get(void);

/* Stage 3 summary: final veh stats, per-terrain tallies, observed costs. */
void stats_print_summary(const VehDataType *vehicle); /* was vdt */

#endif /* STATS_H */

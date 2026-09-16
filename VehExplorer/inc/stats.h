#ifndef STATS_H
#define STATS_H

#include "SatComRelay.h"
#include "mission_control.h"

#include <stdbool.h>

typedef struct {
    unsigned long moves_issued;
    unsigned long moves_ok;
    unsigned long scans_issued;
    unsigned long scans_ok;
} StatsCounters;

void stats_reset(void);

void stats_note_move(bool success);
void stats_note_scan(bool success);

/*
 * Attribute observed energy deltas from a successful move to destination
 * terrain (use LM_TERRAIN_UNKNOWN / '?' when the cell was never scanned).
 */
void stats_note_move_energy(char terrain,
                            long move_energy,
                            long shield_energy);

/*
 * Attribute observed energy deltas from a scan attempt to (dir, radius).
 * Call for any attempt that consumed energy (success or fail).
 */
void stats_note_scan_energy(McScanDir dir,
                            int radius,
                            long scan_energy,
                            long shield_energy);

StatsCounters stats_get(void);

/* Stage 3 summary: final veh stats, per-terrain tallies, observed costs. */
void stats_print_summary(const VehDataType *vdt);

#endif /* STATS_H */

#ifndef STATS_H
#define STATS_H

#include "SatComRelay.h"
#include "mission_control.h"

#include <stdbool.h>

/* Running totals of move and scan commands attempted vs succeeded. */
typedef struct {
    unsigned long moves_issued;
    unsigned long moves_succeeded;
    unsigned long scans_issued;
    unsigned long scans_succeeded;
} StatsCounters;

void stats_reset(void);

void stats_count_move(bool success);
void stats_count_scan(bool success);

/* Record energy and shield deltas from a successful move onto terrain. */
void stats_add_move_energy(char terrain,
                           long move_energy,
                           long shield_energy);

/* Record energy and shield deltas from a scan attempt at (dir, radius). */
void stats_add_scan_energy(MissionControlScanDir dir,
                           int radius,
                           long scan_energy,
                           long shield_energy);

StatsCounters stats_get(void);

/* Stage 3 summary: final vehicle reserves, terrain counts, and energy tables. */
void stats_print_summary(const VehDataType *vehicle);

#endif /* STATS_H */

#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include "SatComRelay.h"

#include <stdbool.h>

typedef struct {
    bool configured;
    bool stats_every;
    bool stats_scan;
    bool stats_move;
    bool stats_shield;
    bool map_scan;
    bool map_move;
    long t1;
    long t2;
    long t3;
    long t4;
    long t5;
    long t6;
    long t7;
    long energy_floor;
    unsigned long max_moves; /* 0 = disabled */
    unsigned long max_scans; /* 0 = disabled */
} AutopilotOptions;

typedef enum {
    AUTOPILOT_RUN_PAUSED = 0, /* back to manual loop */ /* was AP_RUN_PAUSED */
    AUTOPILOT_RUN_QUIT        /* leave Stage 2 */ /* was AP_RUN_QUIT */
} AutopilotRunResult;

void autopilot_options_set_defaults(AutopilotOptions *opt);
bool autopilot_prompt_options(AutopilotOptions *opt);
void autopilot_print_options(const AutopilotOptions *opt);

/* Runs until pause/stop, `p`, `q`, destroy, or stuck. */
AutopilotRunResult autopilot_run(VehDataType *vehicle, int mission_type, AutopilotOptions *opt); /* was vdt */

#endif /* AUTOPILOT_H */

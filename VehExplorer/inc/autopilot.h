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
} ApOptions;

typedef enum {
    AP_RUN_PAUSED = 0, /* back to manual loop */
    AP_RUN_QUIT        /* leave Stage 2 */
} ApRunResult;

void ap_options_set_defaults(ApOptions *opt);
bool ap_prompt_options(ApOptions *opt);
void ap_print_options(const ApOptions *opt);

/* Runs until pause/stop, `p`, `q`, destroy, or stuck. */
ApRunResult ap_run(VehDataType *vdt, int mission_type, ApOptions *opt);

#endif /* AUTOPILOT_H */

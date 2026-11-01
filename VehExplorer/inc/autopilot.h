#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include "SatComRelay.h"

#include <stdbool.h>

/* User-configurable display, delay, and limit settings from the 'o' menu. */
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

/* Tells the Stage 2 command loop whether auto-pilot paused or quit the game. */
typedef enum {
    AUTOPILOT_RUN_PAUSED = 0, /* return to manual command loop */
    AUTOPILOT_RUN_QUIT        /* leave Stage 2 and print final report */
} AutopilotRunResult;

void autopilot_options_set_defaults(AutopilotOptions *opt);
bool autopilot_prompt_options(AutopilotOptions *opt);
void autopilot_print_options(const AutopilotOptions *opt);

/* Run autonomous exploration until paused, quit, goal met, or stuck. */
AutopilotRunResult autopilot_run(VehDataType *vehicle,
                                 int mission_type,
                                 AutopilotOptions *opt);

#endif /* AUTOPILOT_H */

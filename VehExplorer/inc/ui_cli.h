#ifndef UI_CLI_H
#define UI_CLI_H

#include "SatComRelay.h"

#include <stdbool.h>

#define UI_SCENARIO_PATH_MAX 512

typedef struct {
    char scenario_path[UI_SCENARIO_PATH_MAX];
    bool file_needs_decryption;
    bool randomize_start;
    int mission_type;
} UiStartupConfig;

/* Stage 1 prompts. Returns false on EOF / empty path abandoned. */
bool ui_prompt_startup(UiStartupConfig *cfg);

void ui_print_veh_data(const char *label, const VehDataType *vdt);

/* Stage 2: print keybinding help. */
void ui_print_help(void);

/* Stage 2 loop: v/z/x/c/i/t/q + movement + 8-direction scans (radius 1/2).
 * *vdt is refreshed on moves, shield alloc, and scans.
 * Local explored/scanned maps update on successful moves/scans.
 * Move/scan issued vs success counters update on every move/scan attempt.
 * Observed energy deltas feed the Stage 3 summary report. */
void ui_run_command_loop(VehDataType *vdt);

#endif /* UI_CLI_H */

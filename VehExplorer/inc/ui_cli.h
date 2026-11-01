#ifndef UI_CLI_H
#define UI_CLI_H

#include "SatComRelay.h"

#include <stdbool.h>

#define UI_SCENARIO_PATH_MAX 512

/* Stores Stage 1 setup choices entered by the user at startup. */
typedef struct {
    char scenario_path[UI_SCENARIO_PATH_MAX];
    bool file_needs_decryption;
    bool randomize_start;
    int mission_type;
} UiStartupConfig;

/* Stage 1 prompts. Returns false on EOF before setup finishes. */
bool ui_prompt_startup(UiStartupConfig *cfg);

/* Print one-line vehicle energy summary. */
void ui_print_veh_data(const VehDataType *vehicle);

/* Print Stage 2 control instructions. */
void ui_print_help(void);

/* Stage 2 main command loop (manual controls and auto-pilot). */
void ui_run_command_loop(VehDataType *vehicle, int mission_type);

#endif /* UI_CLI_H */

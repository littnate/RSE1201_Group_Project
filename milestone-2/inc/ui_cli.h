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

bool ui_prompt_startup(UiStartupConfig *cfg);

void ui_print_veh_data(const VehDataType *vehicle);

void ui_print_help(void);

void ui_run_command_loop(VehDataType *vehicle, int mission_type);

#endif

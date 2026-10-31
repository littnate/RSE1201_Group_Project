#include "SatComRelay.h"
#include "local_map.h"
#include "mission_control.h"
#include "stats.h"
#include "ui_cli.h"

#include <stdio.h>

/* Heuristic: Init failure typically yields all-zero / non-positive energy. */
static bool vehicle_init_has_energy(const VehDataType *vehicle) /* was vdt */
{
    return vehicle->initialEnergy > 0 || vehicle->currEnergy > 0;
}

/* Library can segfault after printing an open error; refuse missing paths first. */
static bool scenario_file_readable(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        return false;
    }
    fclose(fp);
    return true;
}

int main(void)
{
    UiStartupConfig cfg;
    VehDataType vehicle; /* was vdt */

    if (!ui_prompt_startup(&cfg)) {
        return 1;
    }

    if (!scenario_file_readable(cfg.scenario_path)) {
        fprintf(stderr, "Cannot open scenario file: \"%s\"\n", cfg.scenario_path);
        return 1;
    }

    vehicle = SCRelay_Signal_Veh_Init(
        cfg.scenario_path,
        cfg.file_needs_decryption,
        cfg.randomize_start,
        cfg.mission_type);

    if (!vehicle_init_has_energy(&vehicle)) {
        fprintf(stderr,
                "Init failed — check scenario path and encryption flag "
                "(\"%s\", encrypted=%s).\n",
                cfg.scenario_path,
                cfg.file_needs_decryption ? "y" : "n");
        return 1;
    }

    /* Library already printed init + energy one-liner. */
    local_map_init();
    local_map_mark_explored_here();
    stats_reset();

    ui_run_command_loop(&vehicle, cfg.mission_type);

    stats_print_summary(&vehicle);

    /* Destroy path already freed library state — Close would segfault. */
    if (mission_control_is_mission_over()) {
        printf("Vehicle already shut down by mission-over. Goodbye.\n");
    } else {
        SCRelay_Signal_Veh_Close();
        printf("Vehicle closed. Goodbye.\n");
    }

    return 0;
}

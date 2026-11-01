#include "SatComRelay.h"
#include "local_map.h"
#include "mission_control.h"
#include "stats.h"
#include "ui_cli.h"

#include <stdio.h>

/* Check that vehicle initialization gave positive starting energy. */
static bool vehicle_init_has_energy(const VehDataType *vehicle)
{
    return vehicle->initialEnergy > 0 || vehicle->currEnergy > 0;
}

/* Check that the scenario file exists before passing it to SatComRelay. */
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
    VehDataType vehicle;

    /* Stage 1: Ask the user for scenario path, flags, and mission type */
    if (!ui_prompt_startup(&cfg)) {
        return 1;
    }

    /* Verify the scenario file can be opened before calling the library */
    if (!scenario_file_readable(cfg.scenario_path)) {
        fprintf(stderr, "Cannot open scenario file: \"%s\"\n", cfg.scenario_path);
        return 1;
    }

    /* Initialize the exploration vehicle through SatComRelay */
    vehicle = SCRelay_Signal_Veh_Init(
        cfg.scenario_path,
        cfg.file_needs_decryption,
        cfg.randomize_start,
        cfg.mission_type);

    /* Stop early if initialization failed (for example, wrong encryption flag) */
    if (!vehicle_init_has_energy(&vehicle)) {
        const char *enc_flag = "n";
        if (cfg.file_needs_decryption) {
            enc_flag = "y";
        }
        fprintf(stderr,
                "Init failed — check scenario path and encryption flag "
                "(\"%s\", encrypted=%s).\n",
                cfg.scenario_path,
                enc_flag);
        return 1;
    }

    /* Set up the local map at spawn (0, 0) and clear statistics */
    local_map_init();
    local_map_mark_explored_here();
    stats_reset();

    /* Stage 2: Run the interactive command loop (manual + auto-pilot) */
    ui_run_command_loop(&vehicle, cfg.mission_type);

    /* Stage 3: Print final exploration report */
    stats_print_summary(&vehicle);

    /* Shut down the vehicle session if it was not already ended by destruction */
    if (mission_control_is_mission_over()) {
        printf("Vehicle already shut down by mission-over. Goodbye.\n");
    } else {
        SCRelay_Signal_Veh_Close();
        printf("Vehicle closed. Goodbye.\n");
    }

    return 0;
}

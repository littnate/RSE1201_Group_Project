#include "ui_cli.h"

#include "autopilot.h"
#include "local_map.h"
#include "mission_control.h"
#include "stats.h"
#include "util_input.h"
#include "veh_action.h"

#include <stdio.h>
#include <string.h>

bool ui_prompt_startup(UiStartupConfig *cfg)
{
    if (cfg == NULL) {
        return false;
    }

    memset(cfg, 0, sizeof *cfg);

    for (;;) {
        printf("Please enter the scenario file's name : ");
        fflush(stdout);

        if (!util_read_line(cfg->scenario_path, sizeof cfg->scenario_path)) {
            if (feof(stdin)) {
                fprintf(stderr, "No scenario path provided.\n");
                return false;
            }
            printf("Path cannot be empty.\n");
            continue;
        }
        break;
    }

    cfg->file_needs_decryption =
        util_prompt_yes_no("Is the scenario file encrypted? (y/n) : ");
    cfg->randomize_start =
        util_prompt_yes_no("Do you wish to randomize start position? (y/n) : ");
    cfg->mission_type = util_prompt_int_range(
        "Please select Mission Type :\n"
        "  1) Casual Explore\n"
        "  2) Find 1 End Point\n"
        "  3) Reach 1 End Point\n"
        "  4) Find ALL End Points\n"
        "  5) Reach ALL End Points\n"
        "  6) Find ALL Danger Points\n"
        "  7) Find Map Boundaries\n"
        "  8) Explore Everything\n"
        "Enter 1-8 : ",
        1,
        8);

    printf("fileName : %s, isFileEncrypted : %d, randomizeStartPosition : %d\n",
           cfg->scenario_path,
           cfg->file_needs_decryption ? 1 : 0,
           cfg->randomize_start ? 1 : 0);

    return true;
}

void ui_print_veh_data(const char *label, const VehDataType *vdt)
{
    if (vdt == NULL) {
        return;
    }

    /* Tutor one-liner (VehController_Print1LinerEnrgConsumption). */
    (void)label;
    printf("\nInitial Enrg : %ld, Tot Enrg Used : %ld, Tot Shld Enrg Used : %ld, "
           "Engy Left : %ld, Shld Enrg Left : %ld\n",
           vdt->initialEnergy,
           vdt->totEnergyUsed,
           vdt->totShieldEnergyUsed,
           vdt->currEnergy,
           vdt->currShieldEnergy);
}

static void ui_print_command_stats(void)
{
    StatsCounters s = stats_get();

    printf("Moves issued : %lu, Successful moves : %lu\n",
           s.moves_issued,
           s.moves_ok);
    printf("Scans issued : %lu, Successful scans : %lu\n",
           s.scans_issued,
           s.scans_ok);
}

void ui_print_help(void)
{
    printf("\n*** Welcome to VehExplorer ! ***\n\n");
    printf("[ Usage Instructions ]\n");
    printf("-------------------------------------\n");
    printf("Enter 'q' to Quit\n");
    printf("Enter 'v' to show this help\n");
    printf("'w', 's', 'a', 'd' keys to MOVE vehicle North, South, West & East\n");
    printf("'t' key = Toggle Scan Radius (betw. 1-2)\n");
    printf("NumPad keys to SCAN : SW (1), S (2), SE (3), W (4), E (6), "
           "NW (7), N (8), NE (9) respectively\n");
    printf("'i' = Increase / divert energy to shield\n");
    printf("'z' key = Info on Vehicle Status\n");
    printf("'c' key = Map of terrain scanned by Vehicle\n");
    printf("'x' key = Map of terrain travelled by Vehicle\n");
    printf("'o' key = Configure auto-pilot options\n");
    printf("'p' key = Toggle auto-pilot on / off\n");
    printf("In map display, '_' underline indicates curr. veh. location!\n");
}

static void ui_wait_continue(void)
{
    for (;;) {
        printf("\nPress 'y' key and hit Enter to continue! ");
        fflush(stdout);

        if (util_read_command_char() == 'y') {
            return;
        }
        if (feof(stdin)) {
            return;
        }
        printf("Please press 'y' then Enter.\n");
    }
}

static void handle_allocate_shield(VehDataType *vdt)
{
    long amount;
    char prompt[128];

    if (vdt == NULL) {
        return;
    }

    if (vdt->currEnergy < 1) {
        printf("Error: Insufficient energy reserves. Try reducing the "
               "quantity allocated to shield!\n");
        return;
    }

    snprintf(prompt,
             sizeof prompt,
             "Enter amt. of energy to increase / allocate to shields "
             "(1 - %ld) : ",
             vdt->currEnergy);

    if (!util_prompt_long(prompt, &amount)) {
        printf("Shield allocation cancelled (need a whole number).\n");
        return;
    }

    (void)mc_allocate_shield(vdt, amount);
}

static void handle_toggle_scan_radius(void)
{
    int radius = mc_toggle_scan_radius();
    printf("Toggle Scan Radius => now set to %d terrain units!\n", radius);
}

void ui_run_command_loop(VehDataType *vdt, int mission_type)
{
    ApOptions ap_opt;

    ap_options_set_defaults(&ap_opt);

    ui_print_help();
    ui_wait_continue();
    putchar('\n');

    for (;;) {
        char cmd;

        printf("Enter your command (q to quit) : ");
        fflush(stdout);

        cmd = util_read_command_char();
        if (cmd == '\0') {
            if (feof(stdin)) {
                printf("\nEOF — leaving command loop.\n");
                return;
            }
            continue;
        }

        switch (cmd) {
        case 'v':
            ui_print_help();
            break;
        case 'z':
            printf("\nVehicle Status / Statistics :\n");
            ui_print_veh_data(NULL, vdt);
            ui_print_command_stats();
            break;
        case 'x':
            printf("\nCurrently Explored Terrain Layout :\n");
            local_map_print_explored();
            break;
        case 'c':
            printf("\nCurrently Scanned Terrain Layout :\n");
            local_map_print_scanned();
            break;
        case 'i':
            handle_allocate_shield(vdt);
            break;
        case 't':
            handle_toggle_scan_radius();
            break;
        case 'o':
            (void)ap_prompt_options(&ap_opt);
            break;
        case 'p':
            if (!ap_opt.configured) {
                if (!ap_prompt_options(&ap_opt)) {
                    break;
                }
            }
            if (ap_run(vdt, mission_type, &ap_opt) == AP_RUN_QUIT) {
                return;
            }
            break;
        case 'w':
            veh_action_move(vdt, mc_move_north, 0, -1, true, true);
            break;
        case 's':
            veh_action_move(vdt, mc_move_south, 0, 1, true, true);
            break;
        case 'a':
            veh_action_move(vdt, mc_move_west, -1, 0, true, true);
            break;
        case 'd':
            veh_action_move(vdt, mc_move_east, 1, 0, true, true);
            break;
        case '7':
            veh_action_scan(vdt, MC_DIR_NW, "NW", true, true);
            break;
        case '8':
            veh_action_scan(vdt, MC_DIR_N, "N", true, true);
            break;
        case '9':
            veh_action_scan(vdt, MC_DIR_NE, "NE", true, true);
            break;
        case '4':
            veh_action_scan(vdt, MC_DIR_W, "W", true, true);
            break;
        case '6':
            veh_action_scan(vdt, MC_DIR_E, "E", true, true);
            break;
        case '1':
            veh_action_scan(vdt, MC_DIR_SW, "SW", true, true);
            break;
        case '2':
            veh_action_scan(vdt, MC_DIR_S, "S", true, true);
            break;
        case '3':
            veh_action_scan(vdt, MC_DIR_SE, "SE", true, true);
            break;
        case '5':
            printf("Invalid Command : '5', please try again!\n");
            break;
        case 'q':
            return;
        default:
            printf("Invalid Command : '%c', please try again!\n", cmd);
            break;
        }

        if (mc_is_mission_over()) {
            printf("\nLeaving manual control after mission end.\n");
            return;
        }

        putchar('\n');
    }
}

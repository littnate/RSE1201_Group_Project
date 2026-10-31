cd VehExplorer
./bin/VehExplorer.run

input-files/New_2026_Basic_Scenario_MS_1.dat
y
n
1

Rebuild from VehExplorer/ after a source change. bin/ already exists:

gcc -std=c11 -Wall -Wextra -Iinc -I../MS_1_files src/autopilot.c src/local_map.c src/main.c src/mission_control.c src/stats.c src/ui_cli.c src/util_input.c src/veh_action.c ./libsatcom.so -o bin/VehExplorer.run -Wl,-rpath=.

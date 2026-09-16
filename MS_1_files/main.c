#include <stdio.h>
#include "SatComRelay.h"

int main () {  

    char fileName [50];
    char ans1, ans2, ans3, ans4; /// *** to change variable names

    printf("Please enter the scenario file's name          : ");
    scanf(" %s", fileName);
    printf("\nIs the scenario file encrypted?          (y/n) : ");
    scanf(" %c", &ans1); //if no, show prompt and end program else, error msg;
    printf("\nDo you wish to randomize start position? (y/n) : ");
    scanf(" %c", &ans2); //if no, show prompt and end program else, error msg;
    printf("\nPlease select mission type below\n\n"
        " 1) Casual Explore\n"
        " 2) Find 1 End Point\n"
        " 3) Reach 1 End Point\n"
        " 4) Find ALL End Points\n"
        " 5) Reach ALL End Points\n"
        " 6) Find ALL Danger Points\n"
        " 7) Find Map Boundries\n"
        " 8) Explore Everything\n\n");
    printf("Enter your choice (1-8) : ");
    scanf(" %c", &ans3); //if no, show prompt and end program else, error msg;

    if (ans1 == 'y') { //ans1 - change y to 1 and n to 0
        ans1 = '1';
    }
    else if (ans1 == 'n') {
        ans1 = '0';
    }

    if (ans2 == 'y') { // ans - change y to 1 and n to 0
        ans2 = '1';
    }
    else if (ans2 == 'n') {
        ans2 = '0';
    }
    
    printf("\nfilename : %s, isFileEncrypted : %c, randomizeStartPosition : %c\n\n", fileName,ans1,ans2); //*** to change values to 1 and 0 */
    printf("Initilizing Vehicle Controller ... done!\n\n");
    //----------- initial energy status, dont use hardcoded value-----------]
    //printf("Initial Enrg : %d, Tot Erg Used ; %d, Tot Shld Enrg Used : %d, Engy Left : %d, Shld Enrg Left : %d\n\n", initE, usedE,usedShE, initE-usedE, ***-usedShE);
    printf( "************************************************\n"
        "*** Welcome to Mr Tian's Vehicle Simulator ! ***\n"
        "************************************************\n\n\n"
        "[ Usage Instrutions ]\n\n"
        "--------------------------------------------------------------------------\n\n"//*** check on lengt
        "Enter 'q' to Quit\n\n"
        "Enter 'v' to show this usage instructions (again ..)\n\n"
        "'w', 's', 'a', 'd' keys to MOVE vehicle North, South, West & East\n\n"
        "Enable 'Numlock, (right key pad), keys 1-9 (except 5) to SCAN terrain :\n"
        "SW (1), S(2), SE (3), W(4), E (6), NW (7), N (8), NE (9) respectively\n\n"
        "'t' key = Toggle Scan Radius (betw. 1-2)\n\n"
        "'i' key = increase / divert energy to shield\n\n"
        "'z' key = info on Vehicle Status\n"
        "'c' key = Map of terrain scanned by vehicle\n"
        "'x' key = Map of terrain travelled by vehicle\n\n"
        "Note : In map display, '_' underline indicates curr. veh. location:\n\n"
        "-----------------------------------------------------------------------------\n\n"
        "Press 'y' key and hit Enter to continue! ");
    scanf(" %c",ans4); //why underlined
    printf("Enter your command (q to quit) s: ");
}
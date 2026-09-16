#include <stdio.h>
#include "SatComRelay.h"

int main () {  

    char fileName [50];
    char ans1, ans3;
    char ans2;
    printf("Please enter the scenario file's name   : ");
    scanf(" %s\n\n", &fileName); //***do we have to read it?;
    printf("Is the scenario file encrypted?     (y/n) : ");
    scanf(" %c\n\n", &ans1); //if no, show prompt and end program else, error msg;
    printf("Do you wish to randomize start position? (y/n) : ");
    scanf(" %c\n\n", &ans1); //if no, show prompt and end program else, error msg;
    printf("Please select mission type below\n\n 1) Casual Explore\n 2) Find 1 End Point\n 3) Reach 1 End Point\n 4) Find ALL End Points\n 5) Reach ALL End Points\n 6) Find ALL Danger Points\n 7) Find Map Boundries\n 8) Explore Everything");
    printf("Enter your choice (1-8) : ");
    scanf(" %c\n\n", &ans3); //if no, show prompt and end program else, error msg;
    

    printf("filename : %s, isFileEncrypted : ***, randomizeStartPosition : ***\n\n"); //*** to change values to 1 and 0 */
    printf("Initilizing Vehicle Controller ... done!");
    //----------- initial energy status, dont use hardcoded value-----------]
    //printf("Initial Enrg : %d, Tot Erg Used ; %d, Tot Shld Enrg Used : %d, Engy Left : %d, Shld Enrg Left : %d", initE, usedE,usedShE, initE-usedE, ***-usedShE);
    printf("***********************************\n");
    printf("*** Welcome to Mr Tian's Vehicle Simulator ! ***\n");
    printf("***********************************\n\n\n");
    printf("[ Usage Instrutions ]\n");
    printf("------------------------------");
}
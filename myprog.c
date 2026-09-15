
#include <stdio.h>

#include "timefn.h"

int main()
{
    printf ("\n");
    printf("Welcome to our Sys Time Display program! \n");
    printf("---------------------------------------- \n");

    // edit below line to print your team id # !!
    printf("Team ID # : \n");  
    
    // edit below line to print the NAME of 1st team member ...
    printf("Participating Team Member 1 : \n");  

    // edit below line to print the NAME of 2nd team member ...
    printf("Participating Team Member 2 : \n");  

    // edit below line to print the NAME of "Nth" team member ...
    printf("Participating Team Member XX : \n");  

    printf("---------------------------------------- \n\n");

    printf("1)  Display in 24 HR   \n");
    printf("2)  Display in AM / PM \n");
    printf("---------------------------------------- \n\n");
    printf("Enter preferred display format : ");

    int usersChoice = 0;

    scanf("%d", &usersChoice);

    if (usersChoice == 1)
        printCurrSysTime_24HR();
    else if (usersChoice == 2)

/* 
Below function is commented because it is NOT WORKING YET
  
Research on how to make it work such that no compilation errors will arise when you un-comment the line below that invokes the function "printCurrSysTime_AMPM()"
*/

//        printCurrSysTime_AMPM();
        printf("\nNot working yet! \n");

    else
        printf("\nInvalid Choice! \n");
        
    printf ("\nGoodbye and have a nice day! \n\n");

    return 0;
}

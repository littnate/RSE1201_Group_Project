
#ifndef SATELLITE_COMMS_RELAY_H
#define SATELLITE_COMMS_RELAY_H

#include <stdbool.h>

// -----------------------------------------------------------

struct VehData
{
    long initialEnergy;
    long currEnergy, currShieldEnergy;
    long totEnergyUsed, totShieldEnergyUsed;
};

typedef struct VehData VehDataType;

// -----------------------------------------------------------

typedef VehDataType (*SCRelay_Signal_Veh_MOVT_Fn_Ptr)();
typedef char (*SCRelay_Signal_Veh_SCAN_Fn_Ptr)(VehDataType *);

// -----------------------------------------------------------
// Admin 
// -----------------------------------------------------------
// Note : For SCRelay_Signal_Veh_Init(), the 4th argument 'missionType', if mission is : 
//        "Casual Explore",             set missionType = 1
//        "Find 1 End Point",           set missionType = 2
//        "Reach 1 End Point",          set missionType = 3
//        "Find All End Points",        set missionType = 4
//        "Reach ALL End Points",       set missionType = 5
//        "Find ALL Danger Points",     set missionType = 6
//        "Find Map Boundaries",        set missionType = 7
//        "Map Everything",             set missionType = 8
VehDataType SCRelay_Signal_Veh_Init  (char * srcFilename, bool fileNeedsDecryption, bool randomizeStartPosition, int missionType);
void SCRelay_Signal_Veh_Close ();

VehDataType SCRelay_Signal_Veh_AllocateToShield (long amountOfEnergy);

// -----------------------------------------------------------
// Movement 
// -----------------------------------------------------------
VehDataType SCRelay_Signal_Veh_MoveUpNorth   ();
VehDataType SCRelay_Signal_Veh_MoveDownSouth ();
VehDataType SCRelay_Signal_Veh_MoveRightEast ();
VehDataType SCRelay_Signal_Veh_MoveLeftWest  ();

// -----------------------------------------------------------
// Scanning Terrain
// -----------------------------------------------------------
// Returns '\0' if unsuccessful ...
// -----------------------------------------------------------
char SCRelay_Signal_Veh_ScanNorth (VehDataType * vdt);
char SCRelay_Signal_Veh_ScanNorth2 (VehDataType * vdt);

char SCRelay_Signal_Veh_ScanNorthEast (VehDataType * vdt);
char SCRelay_Signal_Veh_ScanNorthEast2 (VehDataType * vdt);

char SCRelay_Signal_Veh_ScanEast  (VehDataType * vdt);
char SCRelay_Signal_Veh_ScanEast2  (VehDataType * vdt);

char SCRelay_Signal_Veh_ScanSouthEast (VehDataType * vdt);
char SCRelay_Signal_Veh_ScanSouthEast2 (VehDataType * vdt);

char SCRelay_Signal_Veh_ScanSouth (VehDataType * vdt);
char SCRelay_Signal_Veh_ScanSouth2 (VehDataType * vdt);

char SCRelay_Signal_Veh_ScanSouthWest (VehDataType * vdt);
char SCRelay_Signal_Veh_ScanSouthWest2 (VehDataType * vdt);

char SCRelay_Signal_Veh_ScanWest  (VehDataType * vdt);
char SCRelay_Signal_Veh_ScanWest2  (VehDataType * vdt);

char SCRelay_Signal_Veh_ScanNorthWest (VehDataType * vdt);
char SCRelay_Signal_Veh_ScanNorthWest2 (VehDataType * vdt);


// -----------------------------------------------------------
// Below functions for testing / verification purposes. Do not use!
// -----------------------------------------------------------
void SCRelay_Signal_Veh_PrintEntireMap (char password [], bool underlineVehLocation);
void SCRelay_Signal_Veh_PrintScannedMap (char password [], bool underlineVehLocation);
void SCRelay_Signal_Veh_PrintExploredMap (char password [], bool underlineVehLocation);
void SCRelay_Signal_Veh_PrintReport (char password []);


#endif // SATELLITE_COMMS_RELAY_H


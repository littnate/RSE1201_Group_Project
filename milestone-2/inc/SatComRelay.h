#ifndef SATELLITE_COMMS_RELAY_H
#define SATELLITE_COMMS_RELAY_H

#include <stdbool.h>

struct VehData
{
    long initialEnergy;
    long currEnergy, currShieldEnergy;
    long totEnergyUsed, totShieldEnergyUsed;
};

typedef struct VehData VehDataType;

typedef VehDataType (*SCRelay_Signal_Veh_MOVT_Fn_Ptr)();
typedef char (*SCRelay_Signal_Veh_SCAN_Fn_Ptr)(VehDataType *);

VehDataType SCRelay_Signal_Veh_Init  (char * srcFilename, bool fileNeedsDecryption, bool randomizeStartPosition, int missionType);
void SCRelay_Signal_Veh_Close ();

VehDataType SCRelay_Signal_Veh_AllocateToShield (long amountOfEnergy);

VehDataType SCRelay_Signal_Veh_MoveUpNorth   ();
VehDataType SCRelay_Signal_Veh_MoveDownSouth ();
VehDataType SCRelay_Signal_Veh_MoveRightEast ();
VehDataType SCRelay_Signal_Veh_MoveLeftWest  ();

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

void SCRelay_Signal_Veh_PrintEntireMap (char password [], bool underlineVehLocation);
void SCRelay_Signal_Veh_PrintScannedMap (char password [], bool underlineVehLocation);
void SCRelay_Signal_Veh_PrintExploredMap (char password [], bool underlineVehLocation);
void SCRelay_Signal_Veh_PrintReport (char password []);

#endif

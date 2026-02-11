#ifndef HARDWARE_SERVICE_H
#define HARDWARE_SERVICE_H

#include "models/DeviceConfig.h"
#include "models/HardwareEvent.h"

class HardwareService {
public:
    HardwareService();
    ~HardwareService();
    
    char* getRAMUsage();
    char* getCPUUsage();
    char *getUptime();
    char *getFreeHeap();
    char *getDataAsJson();

};


#endif // HARDWARE_SERVICE_H
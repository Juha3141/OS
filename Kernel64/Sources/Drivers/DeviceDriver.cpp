#include <Drivers/DeviceDriver.hpp>

void Kernel::Drivers::DriverSystemManager::Initialize(void) {
    int i;
    SystemList = (unsigned long *)Kernel::MemoryManagement::Allocate(DRIVERSYSTEM_MAXCOUNT*sizeof(unsigned long));
    SystemCount = 0;
    for(i = 0; i < DRIVERSYSTEM_MAXCOUNT; i++) {
        SystemList[i] = 0xFFFFFFFFFFFFFFFF;
    }
}

unsigned long Kernel::Drivers::DriverSystemManager::Register(unsigned long System)  {
    unsigned long i;
    if(SystemCount >= DRIVERSYSTEM_MAXCOUNT) {
        return DRIVERSYSTEM_INVALIDID;
    }/*
    Kernel::printf("this : 0x%X\n" , this);
    for(i = 0; i < DRIVERSYSTEM_MAXCOUNT; i++) {
        Kernel::printf("0x%X " , SystemList[i]);
    }
    Kernel::Keyboard::GetASCIIData();*/
    for(i = 0; i < DRIVERSYSTEM_MAXCOUNT; i++) {
        if(SystemList[i] == 0xFFFFFFFFFFFFFFFF) {
            break;
        }
    }
    if(i >= DRIVERSYSTEM_MAXCOUNT) {
        return DRIVERSYSTEM_INVALIDID;
    }
    SystemList[i] = System;
    SystemCount++;
    return i;
}

unsigned long Kernel::Drivers::DriverSystemManager::GetSystem(unsigned long ID) {
    return SystemList[ID];
}

unsigned long Kernel::Drivers::DriverSystemManager::Deregister(unsigned long ID)  {
    if(SystemCount == 0) {
        return 0xFFFFFFFFFFFFFFFF;
    }
    SystemList[ID] = 0xFFFFFFFFFFFFFFFF;
    SystemCount--;
    return SystemList[ID];
}
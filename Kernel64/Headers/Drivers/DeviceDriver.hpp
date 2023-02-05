#ifndef _DEVICEDRIVER_HPP_
#define _DEVICEDRIVER_HPP_

#include <Kernel.hpp>

#define DRIVERSYSTEM_MAXCOUNT  512
#define DRIVERSYSTEM_INVALIDID 0xFFFFFFFFFFFFFFFF

// check
// https://ryanclaire.blogspot.com/2020/11/linux-device-driver-basics.html

namespace Kernel {
    namespace Drivers {
        template <typename T> class DriverSystemManager { // structure that manages driver
            public:
                void Initialize(void){
                    int i;
                    SystemList = (struct SystemListStruct *)Kernel::MemoryManagement::Allocate(DRIVERSYSTEM_MAXCOUNT*sizeof(SystemListStruct));
                    SystemCount = 0;
                    for(i = 0; i < DRIVERSYSTEM_MAXCOUNT; i++) {
                        SystemList[i].Using = false;
                    }
                }
                unsigned long Register(T *System) {
                    int i;
                    if(SystemCount >= DRIVERSYSTEM_MAXCOUNT) {
                        return DRIVERSYSTEM_INVALIDID;
                    }
                    for(i = 0; i < DRIVERSYSTEM_MAXCOUNT; i++) {
                        if(SystemList[i].Using == false) {
                            SystemList[i].Using = true;
                            break;
                        }
                    }
                    if(i >= DRIVERSYSTEM_MAXCOUNT) {
                        return DRIVERSYSTEM_INVALIDID;
                    }
                    SystemList[i].System = System;
                    SystemCount++;
                    return i;
                }
                T *GetSystem(unsigned long ID) {
                    return SystemList[ID].System;
                }
                T *Deregister(unsigned long ID) {
                    SystemList[ID].Using = false;
                    SystemCount--;
                    return SystemList[ID].System;
                }
                struct SystemListStruct {
                    T *System;
                    bool Using = false;
                }*SystemList;
                unsigned long SystemCount;
        };
    }
}

#endif
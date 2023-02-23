#ifndef _DEVICEDRIVER_HPP_
#define _DEVICEDRIVER_HPP_

#include <Kernel.hpp>

#define DRIVERSYSTEM_MAXCOUNT  512
#define DRIVERSYSTEM_INVALIDID 0xFFFFFFFFFFFFFFFF

// check
// https://ryanclaire.blogspot.com/2020/11/linux-device-driver-basics.html

namespace Kernel {
    namespace Drivers {
        class DriverSystemManager { // structure that manages driver
            public:
                void Initialize(void);
                unsigned long Register(unsigned long System);
                unsigned long GetSystem(unsigned long ID);
                unsigned long Deregister(unsigned long ID);
                unsigned long *SystemList;
                unsigned long SystemCount;
        };
    }
}

#endif
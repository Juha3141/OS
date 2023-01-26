///////////////////////////////////////////////////////////////////
// File "KernelSystemStructure.hpp"                              //
// Started Date : 2022.09.04                                     //
// Fixed major bug : 2022.11.08                                  //
// Description : Header file of file "KernelSystemStructure.hpp" //
///////////////////////////////////////////////////////////////////

#ifndef _KERNEL_SYSTEM_STRUCTURE_HPP_
#define _KERNEL_SYSTEM_STRUCTURE_HPP_

#define KERNELSYSTEMSTRUCTURE_LOCATION 0x500000           // The location of memory space
#define KERNELSYSTEMSTRUCTURE_SIZE     0x100000           // Total Size : 0x70000(Approx. 448KB)

namespace Kernel {
    namespace SystemStructure {
        struct Manager {
            unsigned long CurrentAddress; // CurrentAddress : Address to allocate
        };
        void Initialize(void);
        unsigned long Allocate(unsigned int Size);
    }
}

#endif
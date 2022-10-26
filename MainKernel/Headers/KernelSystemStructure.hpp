///////////////////////////////////////////////////////////////////
// File "KernelSystemStructure.hpp"                              //
// Started Date : 2022.09.04                                     //
// Description : Header file of file "KernelSystemStructure.hpp" //
///////////////////////////////////////////////////////////////////

#ifndef _KERNEL_SYSTEM_STRUCTURE_HPP_
#define _KERNEL_SYSTEM_STRUCTURE_HPP_

#define KERNELSYSTEMSTRUCTURE_LOCATION 0x500000           // The location of memory space
#define KERNELSYSTEMSTRUCTURE_SIZE     0x100000           // Total Size : 0x70000(Approx. 448KB)

namespace Kernel {
    namespace SystemStructure {
        class Manager {
            public:
                void Initialize(void) { // CurrentAddress is set as the description of Kernel::SystemStructure::Initialize function.
                    CurrentAddress = KERNELSYSTEMSTRUCTURE_LOCATION+sizeof(Kernel::SystemStructure::Manager);
                    // CurrentAddress = 0x500008;
                }
                inline void IncreaseCurrentAddress(unsigned long ChunkSize) {
                    CurrentAddress += ChunkSize;
                    // Add ChunkSize to CurrentAddress to make a empty space
                }
                inline unsigned long GetCurrentAddress(void) { // Just return CurrentAddress
                    return CurrentAddress;
                }
            private:
                unsigned long CurrentAddress; // CurrentAddress : Address to allocate
        };
        void Initialize(void);
        unsigned long Allocate(unsigned int Size , unsigned int *ID);
        inline unsigned long FindAddress(unsigned int ID) {
            // Low 16 bit of ID  : Offset - which equals to (ID & 0xFFFF0000) >> 16
            // High 16 bit of ID : Segment - which equals to ID & 0xFFFF 
            // Method to convert ID to real address : (Segment*256)+Offset
            // Note : The method used here is very similar to 16-bit segmentation of intel processor
            // - Except that the multiplied value is different.
            return (((ID & 0xFFFF0000) >> 16)*256)+(ID & 0xFFFF);
        }
    }
}

#endif
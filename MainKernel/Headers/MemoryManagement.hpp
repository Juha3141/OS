///////////////////////////////////////////////////////////////////////////////
// File "MemoryManagement.hpp"                                               //
// Written by   : Juha Cho                                                   //
// Started Date : 2022.10.18                                                 //
// Description  : Header file of MemoryManagement.hpp, contains structure of //
// Memory Management System and allocation & disallocation function.         //
///////////////////////////////////////////////////////////////////////////////

#ifndef _MEMORYMANAGEMENT_H_
#define _MEMORYMANAGEMENT_H_

#define MEMORYMANAGEMENT_MEMORY_STARTADDRESS 0x1720000
#define MEMORYMANAGEMENT_E820_ADDRESS        0xA000
#define MEMORYMANAGEMENT_SIGNATURE           0x3141

#include <Kernel.hpp>

namespace Kernel {
    namespace MemoryManagement {
        struct Node {
            unsigned long NextNode;
            unsigned long PreviousNode;
            unsigned char Using:1;
            unsigned short Signature:15; // 0x3141
        }; // Total 18 bytes
        class NodeManager {
            public:
                void Initialize(unsigned long StartAddress , unsigned long TotalUsableMemory);
                unsigned long SearchReasonableNode(unsigned long Size);
                unsigned long SearchNewNodeLocation(void);

                void MapNode(void);

                unsigned long StartAddress;
                unsigned long CurrentAddress; // Next address of lastly allocated node
                unsigned long LastFreedAddress;
                unsigned long TotalUsableMemory;
        };
        struct QuerySystemAddressMap {
            unsigned long Address;
            unsigned long Length;
            unsigned long Type;
        };
        void Initialize(void);
        void *Allocate(unsigned long Size);
        void Free(void *Address);
    }
}

#endif
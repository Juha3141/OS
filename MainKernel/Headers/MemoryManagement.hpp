///////////////////////////////////////////////////////////////////////////////
// File "MemoryManagement.hpp"                                               //
// Written by   : Juha Cho                                                   //
// Started Date : 2022.10.18                                                 //
// Description  : Header file of MemoryManagement.hpp, contains structure of //
// Memory Management System and allocation & disallocation function.         //
///////////////////////////////////////////////////////////////////////////////

#ifndef _MEMORYMANAGEMENT_H_
#define _MEMORYMANAGEMENT_H_

#define MEMORYMANAGEMENT_MEMORY_STARTADDRESS 0xBF700000 // 0x1720000
#define MEMORYMANAGEMENT_E820_ADDRESS        0xA000
#define MEMORYMANAGEMENT_E820_USABLE         0x01
#define MEMORYMANAGEMENT_E820_RESERVED       0x02
#define MEMORYMANAGEMENT_E820_ACPI_RECLAIMED 0x03
#define MEMORYMANAGEMENT_E820_ACPI_NVS       0x04
#define MEMORYMANAGEMENT_E820_BAD_MEMORY     0x05
#define MEMORYMANAGEMENT_SIGNATURE           0x3141

namespace Kernel {
    namespace MemoryManagement {
        struct Node {
            unsigned long NextNode;
            unsigned long PreviousNode;
            unsigned long Size;
            unsigned char Using:1;
            unsigned short Signature:15; // 0x3141
        }; // Total 18 bytes
        struct QuerySystemAddressMap {
            unsigned long Address;
            unsigned long Length;
            unsigned long Type;
        };
        enum ALIGNMENT {
            NO_ALIGN = 0 , 
            ALIGN_4K = 4096 , 
            ALIGN_8K = 8192
        };
        class NodeManager {
            public:
                void Initialize(unsigned long StartAddress , unsigned long TotalUsableMemory , QuerySystemAddressMap *E820 , int E820EntryCount);
                unsigned long SearchReasonableNode(unsigned long Size);
                unsigned long SearchAlignedNode(unsigned long Size , ALIGNMENT Alignment);
                unsigned long SearchNewNodeLocation(void);
                
                struct Node *CreateNewNode(unsigned long Size , ALIGNMENT Alignment);
                void WriteNodeData(struct Node *Node , unsigned char Using , unsigned long Size , unsigned long NextNode=0xFFFFFFFFFFFFFFFF , unsigned long PreviousNode=0xFFFFFFFFFFFFFFFF);
                
                struct Kernel::MemoryManagement::Node *AdjustNode(struct Node *Node); // If node violated reserved memory, adjust it.
                
                int IsNodeInUnusableMemory(struct Node *Node , QuerySystemAddressMap *ViolatedMemory);
                void AddUnusableMemory(unsigned long StartAddress , unsigned long MemorySize);
                
                void MapNode(void);
                
                unsigned long StartAddress;
                unsigned long CurrentAddress; // Next address of lastly allocated node
                unsigned long LastFreedAddress;
                unsigned long TotalUsableMemory;
            private:
                
                QuerySystemAddressMap UnusableMemories[512];
                int UnusableMemoryEntryCount;
        };
        extern "C" unsigned long GetUsableMemory(unsigned long E820Address , unsigned long MemoryPoolAddress);
        
        struct Node *AlignNode(struct Node *Node , ALIGNMENT Alignment); // Align newly created node
        unsigned long AlignAddress(unsigned long Address , ALIGNMENT Alignment);
        
        unsigned long GetNodeSize(struct Node *Node);
        bool IsMemoryInside(unsigned long Source , unsigned long SourceLength , unsigned long Target , unsigned long TargetLength);

        void Initialize(void);
        void *Allocate(unsigned long Size , ALIGNMENT Alignment = NO_ALIGN);
        void Free(void *Address);

        // Protect memory from being allocated by kernel.
        void ProtectMemory(unsigned long StartAddress , unsigned long MemorySize);
    }
}

#endif
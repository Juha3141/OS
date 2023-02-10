#ifndef _MP_FLOATING_TABLE_HPP_
#define _MP_FLOATING_TABLE_HPP_

#include <KernelSystemStructure.hpp>
#include <MemoryManagement.hpp>
#include <ACPI.hpp>

namespace Kernel {
    namespace MPFloatingTable {
        struct MPFloatingPointer {
            unsigned char Identifier[4]; // "_MP_"
            unsigned int PhysicalAddressPointer;
            unsigned char Length;
            unsigned char VersionNumber;
            unsigned char Checksum;
            unsigned char Reserved[5];
        };
        struct MPFloatingTableHeader {
            unsigned char Identifier[4]; // "PCMP"
            unsigned short BaseTableLength;
            unsigned char VersionNumber;
            unsigned char Checksum;
            unsigned char OEMIDString[8];
            unsigned char ProductString[12];
            
            unsigned int OEMTableAddress;
            unsigned short OEMTableSize;
            unsigned short EntryCount;

            unsigned int LocalAPICAddress;
            
            unsigned short ExtendedTableLength;
            unsigned char ExtendedTableChecksum;
            unsigned char Reserved;
        };
        namespace Entries {
            struct Processor {
                unsigned char EntryType;        // Entry Type : 0
                unsigned char LocalAPICID;
                unsigned char LocalAPICVersion;
                unsigned char CPUFlags;
                unsigned char CPUSignature[4];
                unsigned int FeatureFlags;
                unsigned int Reserved1;
                unsigned int Reserved2;
            };
            struct BUS {
                unsigned char EntryType;        // Entry Type : 1
                unsigned char BusID;
                unsigned char BusTypeString[6];
            };
            struct IOAPIC {
                unsigned char EntryType;        // Entry Type : 2
                unsigned char IOAPICID;
                unsigned char IOAPICVersion;
                unsigned char IOAPICFlags;
                unsigned int IOAPICRegisterAddress;
            };
            struct IOInterruptAssignment {
                unsigned char EntryType;        // Entry Type : 3
                unsigned char InterruptType;
                unsigned short IOInterruptFlags;
                unsigned char SourceBusID;
                unsigned char SourceBusIRQ;
                unsigned char DestinationIOAPICID;
                unsigned char DestinationIOAPICINTIN;
            };
            struct LocalInterrupt {             // Entry Type : 4
                unsigned char EntryType;
                unsigned char InterruptType;
                unsigned short LocalInterruptFlags;
                unsigned char SourceBusID;
                unsigned char SourcesBusIRQ;
                unsigned char DestinationLocalAPICID;
                unsigned char DestinationLocalAPICINTIN;
            };
        }
        unsigned long FindMPFloatingTable(void);
        bool SaveCoresInformation(void);
    }
}

#endif
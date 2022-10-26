#ifndef _ACPI_HPP_
#define _ACPI_HPP_

#define RSDP_ADDRESS_1 0xE0000
#define RSDP_ADDRESS_2 0x40E

namespace Kernel {
    struct CPUProcessorsInformation {
        int CoreCount;
        unsigned int *LocalAPICProcessorID;
        unsigned int *LocalAPICID;
        unsigned int IOAPICID;
            
        unsigned long IOAPICAddress;
        unsigned long LocalAPICAddress;

        bool ACPIUsed;
    };
    namespace ACPI {
        struct RSDP_10 {
            char Signature[8];
            unsigned char Checksum;
            char OEMID[6];
            unsigned char Revision;
            unsigned int RSDTAddress;
        };
        struct RSDP_20 {
            struct RSDP_10 FirstPath;
            unsigned int Length;
            unsigned long XSDTAddress;
            unsigned char ExtendedChecksum;
            unsigned char Reserved[3];
        };
        struct APICSDTHeader {
            char Signature[4];
            unsigned int Length;
            unsigned char Revision;
            unsigned char Checksum;
            char OEMID[6];
            char OEMTableID[8];
            unsigned int OEMRevision;
            unsigned int CreatorID;
            unsigned int CreatorRevision;
        };
        struct FADT {
            struct APICSDTHeader Header;
            unsigned int FirmwareControl;
            unsigned int Dsdt;
            unsigned char Reserved;
            
            unsigned char PreferredPowerManagementProfile;
            unsigned short SCIInterrupt;
            unsigned int SMICommandPort;
            unsigned char ACPIEnable;
            unsigned char ACPIDisable;
            unsigned char S4BIOSREQ;
            unsigned char PSTATEControl;
            unsigned int PM1aEventBlock;
            unsigned int PM1bEventBlock;
            unsigned int PM1aControlBlock;
            unsigned int PM1bControlBlock;
            unsigned int PM2ControlBlock;
            unsigned int PMTimerBlock;
            unsigned int GPE0Block;
            unsigned int GPE1Block;
            unsigned char PM1EventLength;
            unsigned char PM1ControlLength;
            unsigned char PM2ControlLength;
            unsigned char PMTimerLength;
            unsigned char GPE0Length;
            unsigned char GPE1Length;
            unsigned char GPE1Base;
            unsigned char CStateControl;
            unsigned short WorstC2Latency;
            unsigned short WorstC3Latency;
            unsigned short FlushSize;
            unsigned short FlushStride;
            unsigned char DutyOffset;
            unsigned char DutyWidth;
            unsigned char DayAlarm;
            unsigned char MonthAlarm;
            unsigned char Century;
        };
        struct RSDT {
            struct APICSDTHeader Header;
        };

        unsigned long GetRSDPAddress(void);
        unsigned long GetMADTAddress(void);
        bool SaveCoresInformation(void);
        bool CheckRSDPChecksum(unsigned long Address);

        CPUProcessorsInformation *GetCoresInformation(void);
    }
}

#endif
#ifndef _ACPI_HPP_
#define _ACPI_HPP_

#include <KernelSystemStructure.hpp>
#include <EssentialLibrary.hpp>

#define RSDP_ADDRESS_1 0xE0000
#define RSDP_ADDRESS_2 0x40E

#define RSDT_TYPE_UNIDENTIFIED 0x00
#define RSDT_TYPE_APIC         0x01
#define RSDT_TYPE_BERT         0x02
#define RSDT_TYPE_CPEP         0x03
#define RSDT_TYPE_DSDT         0x04
#define RSDT_TYPE_ECDT         0x05
#define RSDT_TYPE_EINJ         0x06
#define RSDT_TYPE_ERST         0x07
#define RSDT_TYPE_FACP         0x08
#define RSDT_TYPE_FACS         0x09
#define RSDT_TYPE_HEST         0x0A
#define RSDT_TYPE_MSCT         0x0B
#define RSDT_TYPE_MPST         0x0C
#define RSDT_TYPE_OEMx         0x0D
#define RSDT_TYPE_PMTT         0x0E
#define RSDT_TYPE_PSDT         0x0F
#define RSDT_TYPE_RASF         0x10
#define RSDT_TYPE_RSDT         0x11
#define RSDT_TYPE_SBST         0x12
#define RSDT_TYPE_SLIT         0x13
#define RSDT_TYPE_SRAT         0x14
#define RSDT_TYPE_SSDT         0x15
#define RSDT_TYPE_XSDT         0x16

struct CoreInformation {
    int CoreCount;
    unsigned int *LocalAPICProcessorID;
    unsigned int *LocalAPICID;
    unsigned int IOAPICID;
        
    unsigned long IOAPICAddress;
    unsigned long LocalAPICAddress;

    bool ACPIUsed;
    bool MPUsed;

    static struct CoreInformation *GetInstance(void) {
        static struct CoreInformation *Instance;
        if(Instance == 0x00) {
            Instance = (struct CoreInformation *)SystemStructure::Allocate(sizeof(struct CoreInformation));
        }
        return Instance;
    }
    static void SetInstance(struct CoreInformation *NewInstance) {
        struct CoreInformation *OldInstance = CoreInformation::GetInstance();
        memcpy(OldInstance , NewInstance , sizeof(struct CoreInformation));
    }
};
namespace ACPI {
    struct RSDP {
        char Signature[8];
        unsigned char Checksum;
        char OEMID[6];
        unsigned char Revision;
        unsigned int RSDTAddress;
    };
    struct ExtendedRSDP {
        struct RSDP RSDP;
        unsigned int Length;
        unsigned long XSDTAddress;
        unsigned char ExtendedChecksum;
        unsigned char Reserved[3];
    };
    struct SDTHeader {
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
        struct SDTHeader Header; // All Descriptor Tables need header table.
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
        struct SDTHeader Header;
    };
        
    struct ACPIEntry {
        struct RSDP *RSDP;
        struct ExtendedRSDP *RSDP20;
        struct RSDT *RSDT;
        unsigned long RSDTEntryCount;
        
        bool FailedInitialization;
        
        static struct ACPIEntry *GetInstance(void) {
            static class ACPIEntry *Instance = 0x00;
            if(Instance == 0x00) {
                Instance = (struct ACPIEntry *)SystemStructure::Allocate(sizeof(struct ACPIEntry));
            }
            return Instance;
        }
    };
        
    bool Initialize(void);

    unsigned long GetDescriptorTable(const char Signature[4]);
    bool SaveCoresInformation(void);
    bool CheckRSDPChecksum(unsigned long Address);
    unsigned int IdentifySDT(unsigned long Address);
}
struct CPUInformation *GetCoresInformation(void);
void WriteCoresInformation(CPUInformation *NewCoreInformation);

#endif
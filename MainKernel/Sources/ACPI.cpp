#include <ACPI.hpp>
#include <Kernel.hpp>

static struct Kernel::CPUProcessorsInformation *CoreInformation;

bool Kernel::ACPI::SaveCoresInformation(void) {
    int i = 0;
    int j = 0;
    int CoreCount = 0;
    unsigned int EAX;
    unsigned int EDX;
    unsigned int ID;
    unsigned long MADTAddress = GetMADTAddress();
    APICSDTHeader *MADTHeader = (APICSDTHeader *)MADTAddress;
    unsigned char *MADT = (unsigned char *)(MADTAddress+sizeof(APICSDTHeader));
    if(MADTAddress == 0) {
        CoreInformation->ACPIUsed = 0;
        return 0;
    }
    MADT += 8;
    CoreInformation = (struct Kernel::CPUProcessorsInformation *)SystemStructure::Allocate(sizeof(struct Kernel::CPUProcessorsInformation) , &(ID));
    
    __asm__ ("mov rcx , 27");
    __asm__ ("rdmsr");
    __asm__ ("mov %0 , eax":"=r"(EAX));
    __asm__ ("mov %0 , edx":"=r"(EDX));
    CoreInformation->LocalAPICAddress = (EDX << 31)|EAX;
    CoreInformation->LocalAPICAddress ^= (CoreInformation->LocalAPICAddress & 0b111111111111);
#ifdef DEBUG
    Kernel::printf("Local APIC Address : 0x%X\n" , CoreInformation->LocalAPICAddress);
#endif
    while(1) {
        if(MADT[i] == 0) {
            CoreCount++;
        }
        i += MADT[i+1];
        if(i >= (MADTHeader->Length-sizeof(APICSDTHeader)-8)) {
            i = 0;
            break;
        }
    }
    CoreInformation->LocalAPICID = (unsigned int *)SystemStructure::Allocate(CoreCount*sizeof(unsigned int) , &(ID));
    CoreInformation->LocalAPICProcessorID = (unsigned int *)SystemStructure::Allocate(CoreCount*sizeof(unsigned int) , &(ID));
    Kernel::printf("CoreInformation->LocalAPICID : 0x%X(%d)\n" , CoreInformation->LocalAPICID , CoreCount*sizeof(unsigned int));
    Kernel::printf("CoreInformation->LocalAPICProcessorID : 0x%X(%d)\n" , CoreInformation->LocalAPICProcessorID , CoreCount*sizeof(unsigned int));
    while(1) {
        if(MADT[i] == 0) {
            CoreInformation->LocalAPICProcessorID[j] = (unsigned int)((unsigned char)MADT[i+2]);
            CoreInformation->LocalAPICID[j] = (unsigned int)((unsigned char)MADT[i+3]);
            j++;
        }
        else if(MADT[i] == 1) {
            CoreInformation->IOAPICAddress = ((MADT[i+4])|(MADT[i+5] << 8)|(MADT[i+6] << 16)|(MADT[i+7] << 24));
        }
        i += MADT[i+1];
        if(i >= (MADTHeader->Length-sizeof(APICSDTHeader)-8)) {
            break;
        }
    }
    CoreInformation->CoreCount = CoreCount;
#ifdef DEBUG
    Kernel::printf("Core Count : %d\n" , CoreCount);
#endif
    return 1;
}

Kernel::CPUProcessorsInformation *Kernel::ACPI::GetCoresInformation(void) {
    return CoreInformation;
}

unsigned long Kernel::ACPI::GetRSDPAddress(void) {
    if(CheckRSDPChecksum(RSDP_ADDRESS_1) == 1) {
        return RSDP_ADDRESS_1;
    }
    if(CheckRSDPChecksum(RSDP_ADDRESS_2) == 1) {
        return RSDP_ADDRESS_2;
    }
    return 0;
}

unsigned long Kernel::ACPI::GetMADTAddress(void) {
    int i = 0;
    int EntriesCount;
    unsigned int *NextAddress;
    unsigned long *NextAddressX;
    RSDP_10 *RSDP = (RSDP_10 *)GetRSDPAddress();
    RSDP_20 *RSDP20;
    struct RSDT *RSDT = (struct RSDT *)RSDP->RSDTAddress;
    if(RSDP == 0) {
        return 0;
    }
    if(RSDP->Revision == 2) {
        RSDP20 = (RSDP_20 *)((CheckRSDPChecksum(RSDP_ADDRESS_1) == 1) ? RSDP_ADDRESS_1 : RSDP_ADDRESS_2);
        RSDT = (struct RSDT *)RSDP20->XSDTAddress;
        Kernel::printf("Using XSDT\n");
    }
    EntriesCount = (RSDT->Header.Length-sizeof(APICSDTHeader))/((RSDP->Revision == 2) ? sizeof(unsigned long) : sizeof(unsigned int));

    Kernel::printf("RSDP Address  : 0x%X\n" , RSDP);
    Kernel::printf("RSDT Address  : 0x%X\n" , RSDT);
    Kernel::printf("Entries Count : 0x%X\n" , EntriesCount);

    NextAddress = (unsigned int *)(((unsigned long)RSDT)+sizeof(APICSDTHeader));
    NextAddressX = (unsigned long *)(((unsigned long)RSDT)+sizeof(APICSDTHeader));
    for(i = 0; i < EntriesCount; i++) {
        if(memcmp(RSDT->Header.Signature , "APIC" , 4) == 0) {
            return (unsigned long)RSDT;
        }
        RSDT = (struct RSDT *)((RSDP->Revision == 2) ? NextAddressX[i] : NextAddress[i]);
    }
    return 0;
}

bool Kernel::ACPI::CheckRSDPChecksum(unsigned long Address) {
    int i;
    unsigned char *RSDP = (unsigned char *)Address;
    unsigned char ChecksumString[8] = {'R' , 'S' , 'D' , ' ' , 'P' , 'T' , 'R' , ' '};
    for(i = 0; i < 8; i++) {
        if(RSDP[i] != ChecksumString[i]) {
            return 0;
        }
    }
    return 1;
}
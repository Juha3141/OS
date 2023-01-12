#include <ACPI.hpp>
#include <Kernel.hpp>

static struct Kernel::CPUInformation *CoreInformation;

#define DEBUG

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
    // MADT : Describes every interrupt controller in the system, this includes LocalAPIC and I/O APIC.
    // We're going to read the information of Local APIC and I/O APIC, and the number of Local APIC corresponds to 
    // number of the cores, so we're also going to find the number of cores by those informations.
    if(MADTAddress == 0) {  // If MADT was not found, use MP table.
        CoreInformation->ACPIUsed = 0;  // Didn't use ACPI to get the information
        return false;
    }
    MADT += 8;  // 
    // Allocate space to store core informations.
    CoreInformation = (struct Kernel::CPUInformation *)SystemStructure::Allocate(sizeof(struct Kernel::CPUInformation));
    
    // Get the address of Local APIC registers from MSR.
    // MSR 0x1B : APIC_BASE
    // Contains 32bits of Local APIC base and flags.
    __asm__ ("mov rcx , 27");
    __asm__ ("rdmsr");

    __asm__ ("mov %0 , eax":"=r"(EAX)); // EAX : Lower 32 bits
    __asm__ ("mov %0 , edx":"=r"(EDX)); // EDX : Higher 32 bits
    
    CoreInformation->LocalAPICAddress = (EDX << 31)|EAX;
    // Clear flag bits to only get the address, which has size of 12 bits.
    CoreInformation->LocalAPICAddress ^= (CoreInformation->LocalAPICAddress & 0b111111111111);
    Kernel::printf("Local APIC Address : 0x%X\n" , CoreInformation->LocalAPICAddress);
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
    CoreInformation->LocalAPICID = (unsigned int *)SystemStructure::Allocate(CoreCount*sizeof(unsigned int));
    CoreInformation->LocalAPICProcessorID = (unsigned int *)SystemStructure::Allocate(CoreCount*sizeof(unsigned int));
    Kernel::printf("CoreInformation->LocalAPICID : 0x%X(%d)\n" , CoreInformation->LocalAPICID , CoreCount*sizeof(unsigned int));
    Kernel::printf("CoreInformation->LocalAPICProcessorID : 0x%X(%d)\n" , CoreInformation->LocalAPICProcessorID , CoreCount*sizeof(unsigned int));
    while(i < (MADTHeader->Length-sizeof(APICSDTHeader)-8)) {
        if(MADT[i] == 0) {      // Entry Type 0 : Processor Local APIC
            /* Process Local APIC Entry : 
             * Offset 2(Size : 1) : ACPI Processor ID
             * Offset 3(Size : 1) : APIC ID
             * Offset 4(Size : 4) : Flags(0 = Processor Enable , 1 = Online Capable(Available for activation))
             */
            // Offset 2 : MADT[i+2]
            CoreInformation->LocalAPICProcessorID[j] = (unsigned int)((unsigned char)MADT[i+2]);
            // Offset 3 : MADT[i+3]
            CoreInformation->LocalAPICID[j] = (unsigned int)((unsigned char)MADT[i+3]);
            j++;
        }
        else if(MADT[i] == 1) { // Entry Type 1 : I/O APIC 
            CoreInformation->IOAPICAddress = (unsigned int)(((unsigned int)(MADT[i+4]))|((unsigned int)(MADT[i+5] << 8))|((unsigned int)(MADT[i+6] << 16))|((unsigned int)(MADT[i+7] << 24)));
        }
        i += MADT[i+1];
    }
    Kernel::printf("IO APIC Address : 0x%X\n" , CoreInformation->IOAPICAddress);
    Kernel::printf("Core count : %d\n" , CoreCount);
    CoreInformation->CoreCount = CoreCount;
    return true;
}

Kernel::CPUInformation *Kernel::GetCoresInformation(void) {
    return CoreInformation;
}

void Kernel::WriteCoresInformation(Kernel::CPUInformation *NewCoreInformation) {
    CoreInformation = NewCoreInformation;
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
        if(memcmp(RSDT->Header.Signature , "APIC" , 4) == 0) {  // MADT Signature : APIC
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
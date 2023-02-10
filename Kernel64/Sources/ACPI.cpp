#include <ACPI.hpp>
#include <Kernel.hpp>

static struct Kernel::CPUInformation *CoreInformation;
static struct Kernel::ACPI::ACPIEntry *UniversalACPIEntry;

#define DEBUG

bool Kernel::ACPI::Initialize(void) { // Gather information, and write it to ACPI Entry structure
    UniversalACPIEntry = (struct ACPIEntry *)Kernel::SystemStructure::Allocate(sizeof(struct ACPIEntry));
    UniversalACPIEntry->RSDP = (struct RSDP *)GetRSDPAddress();
    UniversalACPIEntry->RSDT = (struct RSDT *)UniversalACPIEntry->RSDP->RSDTAddress;
    if(UniversalACPIEntry->RSDP == 0) {
        return false;
    }
    if(UniversalACPIEntry->RSDP->Revision == 2) { // If revision is two, use XSDT.
        UniversalACPIEntry->RSDP20 = (struct ExtendedRSDP *)((CheckRSDPChecksum(RSDP_ADDRESS_1) == 1) ? RSDP_ADDRESS_1 : RSDP_ADDRESS_2);
        UniversalACPIEntry->RSDT = (struct RSDT *)UniversalACPIEntry->RSDP20->XSDTAddress;
        Kernel::printf("Using XSDT\n");
    }
    else {
        UniversalACPIEntry->RSDP20 = 0x00;
    }
    UniversalACPIEntry->RSDTEntryCount = (UniversalACPIEntry->RSDT->Header.Length-sizeof(SDTHeader))
                                         /((UniversalACPIEntry->RSDP->Revision == 2) ? sizeof(unsigned long) : sizeof(unsigned int));
    /*Kernel::printf("RSDP Address         : 0x%X\n" , UniversalACPIEntry->RSDP);
    Kernel::printf("RSDT Address         : 0x%X\n" , UniversalACPIEntry->RSDT);
    Kernel::printf("RSDT Entries Count   : 0x%X\n" , UniversalACPIEntry->RSDTEntryCount);
    */return true;
}

bool Kernel::ACPI::SaveCoresInformation(void) {
    int i = 0;
    int j = 0;
    int CoreCount = 0;
    unsigned int EAX;
    unsigned int EDX;
    unsigned int ID;
    unsigned long MADTAddress = GetDescriptorTable("APIC");
    SDTHeader *MADTHeader = (SDTHeader *)MADTAddress;
    unsigned char *MADT = (unsigned char *)(MADTAddress+sizeof(SDTHeader));
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
    //Kernel::printf("Local APIC Address : 0x%X\n" , CoreInformation->LocalAPICAddress);
    while(1) {
        if(MADT[i] == 0) {
            CoreCount++;
        }
        i += MADT[i+1];
        if(i >= (MADTHeader->Length-sizeof(SDTHeader)-8)) {
            i = 0;
            break;
        }
    }
    CoreInformation->LocalAPICID = (unsigned int *)SystemStructure::Allocate(CoreCount*sizeof(unsigned int));
    CoreInformation->LocalAPICProcessorID = (unsigned int *)SystemStructure::Allocate(CoreCount*sizeof(unsigned int));
    /*Kernel::printf("CoreInformation->LocalAPICID : 0x%X(%d)\n" , CoreInformation->LocalAPICID , CoreCount*sizeof(unsigned int));
    Kernel::printf("CoreInformation->LocalAPICProcessorID : 0x%X(%d)\n" , CoreInformation->LocalAPICProcessorID , CoreCount*sizeof(unsigned int));
    */while(i < (MADTHeader->Length-sizeof(SDTHeader)-8)) {
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
    /*Kernel::printf("IO APIC Address : 0x%X\n" , CoreInformation->IOAPICAddress);
    Kernel::printf("Core count : %d\n" , CoreCount);
    */CoreInformation->CoreCount = CoreCount;
    Kernel::MemoryManagement::ProtectMemory(CoreInformation->LocalAPICAddress , 0x100000);
    Kernel::MemoryManagement::ProtectMemory(CoreInformation->IOAPICAddress , 0x100000);
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

unsigned long Kernel::ACPI::GetDescriptorTable(const char Signature[4]) {
    int i = 0;
    unsigned int *NextAddress;
    unsigned long *NextAddressX; // Next Address for XSDT(Extended SDT)
    int RSDPRevision = UniversalACPIEntry->RSDP->Revision;
    struct RSDT *RSDT = UniversalACPIEntry->RSDT;
    NextAddress = (unsigned int *)(((unsigned long)RSDT)+sizeof(SDTHeader));
    NextAddressX = (unsigned long *)(((unsigned long)RSDT)+sizeof(SDTHeader));
    for(i = 0; i < UniversalACPIEntry->RSDTEntryCount; i++) {
        if(memcmp(RSDT->Header.Signature , Signature , 4) == 0) {
            return (unsigned long)RSDT;
        }
        RSDT = (struct RSDT *)((RSDPRevision == 2) ? NextAddressX[i] : NextAddress[i]);
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
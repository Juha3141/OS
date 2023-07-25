#include <ACPI.hpp>
#include <Kernel.hpp>

#define DEBUG

bool ACPI::Initialize(void) { // Gather information, and write it to ACPI Entry structures
    struct ACPIEntry *UniversalACPIEntry = ACPIEntry::GetInstance();
    UniversalACPIEntry->RSDT = (struct RSDT *)UniversalACPIEntry->RSDP->RSDTAddress;
    if(CheckRSDPChecksum(RSDP_ADDRESS_1) == true) {
        UniversalACPIEntry->RSDP = (RSDP *)RSDP_ADDRESS_1;
    }
    else if(CheckRSDPChecksum(RSDP_ADDRESS_2) == true) {
        UniversalACPIEntry->RSDP = (RSDP *)RSDP_ADDRESS_2;
    }
    else {
        UniversalACPIEntry->FailedInitialization = true;
        return false; // no acpi
    }
    if(UniversalACPIEntry->RSDP->Revision == 2) { // If revision is two, use XSDT.
        UniversalACPIEntry->RSDP20 = (struct ExtendedRSDP *)((CheckRSDPChecksum(RSDP_ADDRESS_1) == 1) ? RSDP_ADDRESS_1 : RSDP_ADDRESS_2);
        UniversalACPIEntry->RSDT = (struct RSDT *)UniversalACPIEntry->RSDP20->XSDTAddress;
        printf("Using XSDT\n");
    }
    else {
        UniversalACPIEntry->RSDP20 = 0x00;
    }
    UniversalACPIEntry->RSDTEntryCount = (UniversalACPIEntry->RSDT->Header.Length-sizeof(SDTHeader))
                                         /((UniversalACPIEntry->RSDP->Revision == 2) ? sizeof(unsigned long) : sizeof(unsigned int));
    /*printf("RSDP Address         : 0x%X\n" , UniversalACPIEntry->RSDP);
    printf("RSDT Address         : 0x%X\n" , UniversalACPIEntry->RSDT);
    printf("RSDT Entries Count   : 0x%X\n" , UniversalACPIEntry->RSDTEntryCount);
    */return true;
}

bool ACPI::SaveCoresInformation(void) {
    int i = 0;
    int j = 0;
    int CoreCount = 0;
    unsigned int EAX;
    unsigned int EDX;
    unsigned int ID;
    unsigned long MADTAddress = GetDescriptorTable("APIC");
    SDTHeader *MADTHeader = (SDTHeader *)MADTAddress;
    unsigned char *MADT = (unsigned char *)(MADTAddress+sizeof(SDTHeader));
    struct CoreInformation *CoreInformation = CoreInformation::GetInstance();
    if(ACPI::ACPIEntry::GetInstance()->FailedInitialization == true) {
        return false;
    }
    // MADT : Describes every interrupt controller in the system, this includes LocalAPIC and I/O APIC.
    // We're going to read the information of Local APIC and I/O APIC, and the number of Local APIC corresponds to 
    // number of the cores, so we're also going to find the number of cores by those informations.
    if(MADTAddress == 0) {  // If MADT was not found, use MP table.
        CoreInformation->ACPIUsed = 0;  // Didn't use ACPI to get the information
        return false;
    }
    MADT += 8;
    // Allocate space to store core informations.
    
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
    do {
        if(MADT[i] == 0) {
            CoreCount++;
        }
        i += MADT[i+1];
    }while(i < (MADTHeader->Length-sizeof(SDTHeader)-8));
    i = 0;
    CoreInformation->LocalAPICID = (unsigned int *)SystemStructure::Allocate(CoreCount*sizeof(unsigned int));
    CoreInformation->LocalAPICProcessorID = (unsigned int *)SystemStructure::Allocate(CoreCount*sizeof(unsigned int));
    printf("CoreInformation->LocalAPICID : 0x%X(%d)\n" , CoreInformation->LocalAPICID , CoreCount*sizeof(unsigned int));
    printf("CoreInformation->LocalAPICProcessorID : 0x%X(%d)\n" , CoreInformation->LocalAPICProcessorID , CoreCount*sizeof(unsigned int));
    while(i < (MADTHeader->Length-sizeof(SDTHeader)-8)) {
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
    /*printf("IO APIC Address : 0x%X\n" , CoreInformation->IOAPICAddress);
    printf("Core count : %d\n" , CoreCount);
    */CoreInformation->CoreCount = CoreCount;
    MemoryManagement::ProtectMemory(CoreInformation->LocalAPICAddress , 0x100000);
    MemoryManagement::ProtectMemory(CoreInformation->IOAPICAddress , 0x100000);
    return true;
}

unsigned long ACPI::GetDescriptorTable(const char Signature[4]) {
    int i = 0;
    unsigned int *NextAddress;
    unsigned long *NextAddressX; // Next Address for XSDT(Extended SDT)
    struct ACPIEntry *UniversalACPIEntry = ACPIEntry::GetInstance();
    if(UniversalACPIEntry->FailedInitialization == true) {
        return 0x00;
    }
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
    return 0x00;
}

bool ACPI::CheckRSDPChecksum(unsigned long Address) { // error
    int i;
    unsigned char *RSDP = (unsigned char *)Address;
    unsigned char ChecksumString[8] = {'R' , 'S' , 'D' , ' ' , 'P' , 'T' , 'R' , ' '};
    for(i = 0; i < 8; i++) {
        if(RSDP[i] != ChecksumString[i]) {
            return false;
        }
    }
    return true;
}

unsigned int ACPI::IdentifySDT(unsigned long Address) {
    int i;
    struct SDTHeader *Header = (struct SDTHeader *)Address;
    char RSDTSignatureList[22][5] = {
        "APIC" , "BERT" , "CPEP" , "DSDT" , 
        "ECDT" , "EINJ" , "ERST" , "FACP" , 
        "FACS" , "HEST" , "MSCT" , "MPST" , 
        "OEMx" , "PMTT" , "PSDT" , "RASF" , 
        "RSDT" , "SBST" , "SLIT" , "SRAT" , 
        "SSDT" , "XSDT" , 
    };
    for(i = 0; i < 22; i++) {
        if(memcpy((unsigned char *)Address , RSDTSignatureList[i] , 4) == 0) {
            return i+1;
        }
    }
    return 0x00;
}
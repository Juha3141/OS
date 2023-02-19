#include <MPFloatingTable.hpp>
#include <EssentialLibrary.hpp>
#include <TextScreen.hpp>

// To-do : comment

unsigned long Kernel::MPFloatingTable::FindMPFloatingTable(void) {
    int i;
    unsigned char *MPFloatingPointerLocation;
    Kernel::printf("Finding MP Floating Pointer : \n");
    for(MPFloatingPointerLocation = (unsigned char *)(0x40E*16); MPFloatingPointerLocation <= (unsigned char *)((0x40E*16)+1024); MPFloatingPointerLocation += 1) {
        if(memcmp(MPFloatingPointerLocation , "_MP_" , 4) == 0) {
            Kernel::printf("Found MP Floating Pointer : 0x%X\n" , MPFloatingPointerLocation);
            return (unsigned long)MPFloatingPointerLocation;
        }
    }
    for(MPFloatingPointerLocation = (unsigned char *)((0x413*1024)-1024); MPFloatingPointerLocation <= (unsigned char *)((0x403*1024)); MPFloatingPointerLocation += 1) {
        if(memcmp(MPFloatingPointerLocation , "_MP_" , 4) == 0) {
            Kernel::printf("Found MP Floating Pointer : 0x%X\n" , MPFloatingPointerLocation);
            return (unsigned long)MPFloatingPointerLocation;
        }
    }
    for(MPFloatingPointerLocation = (unsigned char *)0xF0000; MPFloatingPointerLocation <= (unsigned char *)0xFFFFF; MPFloatingPointerLocation += 1) {
        if(memcmp(MPFloatingPointerLocation , "_MP_" , 4) == 0) {
            Kernel::printf("Found MP Floating Pointer : 0x%X\n" , MPFloatingPointerLocation);
            return (unsigned long)MPFloatingPointerLocation;
        }
    }
    return 0x00;
}

bool Kernel::MPFloatingTable::SaveCoresInformation(void) {
    int i;
    int j = 0;
    int CoreCount = 0;
    unsigned int EAX;
    unsigned int EDX;
    unsigned long TableAddress;
    struct Kernel::CoreInformation *CoreInformation = (struct Kernel::CoreInformation *)Kernel::MemoryManagement::Allocate(sizeof(struct Kernel::CoreInformation));
    struct MPFloatingPointer *MPFloatingPointer = (struct MPFloatingPointer *)FindMPFloatingTable();
    struct MPFloatingTableHeader *TableHeader = (struct MPFloatingTableHeader *)MPFloatingPointer->PhysicalAddressPointer;
    
    struct Entries::Processor *ProcessorEntry;
    struct Entries::BUS *BUSEntry;
    struct Entries::IOAPIC *IOAPICEntry;
    struct Entries::IOInterruptAssignment *IOInterruptAssignmentEntry;
    struct Entries::LocalInterrupt *LocalInterruptEntry;
    
    Kernel::printf("MP Floating Pointer Address : 0x%X\n" , MPFloatingPointer);
    Kernel::printf("PhysicalAddressPointer      : 0x%X\n" , MPFloatingPointer->PhysicalAddressPointer);

    if(memcmp(TableHeader->Identifier , "PCMP" , 4) != 0) {
        return false;
    }
    Kernel::printf("MP Floating Table Entry Count : %d\n" , TableHeader->EntryCount);
    TableAddress = MPFloatingPointer->PhysicalAddressPointer+sizeof(struct MPFloatingTableHeader);
    CoreInformation->ACPIUsed = false;
    CoreInformation->MPUsed = true;
    for(i = 0; i < TableHeader->EntryCount; i++) {
        switch(*((unsigned char*)TableAddress)) {
            case 0:
                CoreCount++;
                TableAddress += sizeof(struct Entries::Processor);
                break;
            case 1:
                TableAddress += sizeof(struct Entries::BUS);
                break;
            default:
                TableAddress += 8;
                break;
        }
    }
    Kernel::printf("Core count : %d\n" , CoreCount);
    CoreInformation->LocalAPICID = (unsigned int *)Kernel::MemoryManagement::Allocate(CoreCount*sizeof(unsigned int));
    CoreInformation->LocalAPICProcessorID = (unsigned int *)Kernel::MemoryManagement::Allocate(CoreCount*sizeof(unsigned int));
    CoreInformation->CoreCount = CoreCount;
    TableAddress = MPFloatingPointer->PhysicalAddressPointer+sizeof(struct MPFloatingTableHeader);
    for(i = 0; i < TableHeader->EntryCount; i++) {
        switch(*((unsigned char*)TableAddress)) {
            case 0:
                ProcessorEntry = (struct Entries::Processor *)TableAddress;
                CoreInformation->LocalAPICID[j] = ProcessorEntry->LocalAPICID;
                CoreInformation->LocalAPICProcessorID[j] = ProcessorEntry->LocalAPICID;
                j++;
                TableAddress += sizeof(struct Entries::Processor);
                break;
            case 1:
                TableAddress += sizeof(struct Entries::BUS);
                break;
            case 2:
                IOAPICEntry = (struct Entries::IOAPIC *)TableAddress;
                Kernel::printf("IOAPIC ID               : 0x%X\n" , IOAPICEntry->IOAPICID);
                TableAddress += sizeof(struct Entries::IOAPIC);
                Kernel::printf("IOAPIC Register Address : 0x%X\n" , IOAPICEntry->IOAPICRegisterAddress);
                break;
            case 3:
                TableAddress += sizeof(struct Entries::IOInterruptAssignment);
                break;
            case 4:
                TableAddress += sizeof(struct Entries::LocalInterrupt);
                break;
            default:
                TableAddress += 8;
                break;
        };
    }
    
    // Get the address of Local APIC registers from MSR(As ACPI did)
    // (the comments are same as the acpi part)

    // MSR 0x1B : APIC_BASE
    // Contains 32bits of Local APIC base and flags.
    __asm__ ("mov rcx , 27");
    __asm__ ("rdmsr");

    __asm__ ("mov %0 , eax":"=r"(EAX)); // EAX : Lower 32 bits
    __asm__ ("mov %0 , edx":"=r"(EDX)); // EDX : Higher 32 bits
    
    CoreInformation->LocalAPICAddress = (EDX << 31)|EAX;
    // Clear flag bits to only get the address, which has size of 12 bits.
    CoreInformation->LocalAPICAddress ^= (CoreInformation->LocalAPICAddress & 0b111111111111);
    Kernel::printf("Local APIC Address from MSR : 0x%X\n" , CoreInformation->LocalAPICAddress);
    CoreInformation::SetInstance(CoreInformation);
    return true;
}
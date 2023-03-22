#include <APIC.hpp>
#include <PIT.hpp>
#include <TaskManagement.hpp>
#include <MutualExclusion.hpp>

void Kernel::LocalAPIC::WriteRegister(unsigned int RegisterAddress , unsigned int Data) {
    struct CoreInformation *CoreInformation = CoreInformation::GetInstance();
    *((unsigned int *)(CoreInformation->LocalAPICAddress+RegisterAddress)) = Data;
}

void Kernel::LocalAPIC::WriteRegister_L(unsigned int RegisterAddress , unsigned long Data) {
    struct CoreInformation *CoreInformation = CoreInformation::GetInstance();
    *((unsigned long *)(CoreInformation->LocalAPICAddress+RegisterAddress)) = Data;
}
    
unsigned int Kernel::LocalAPIC::ReadRegister(unsigned int RegisterAddress) {
    struct CoreInformation *CoreInformation = CoreInformation::GetInstance();
    return *((unsigned int *)(CoreInformation->LocalAPICAddress+RegisterAddress));
}

unsigned long Kernel::LocalAPIC::ReadRegister_L(unsigned int RegisterAddress) {
    struct CoreInformation *CoreInformation = CoreInformation::GetInstance();
    return *((unsigned long *)(CoreInformation->LocalAPICAddress+RegisterAddress));
}

void Kernel::IOAPIC::WriteRegister(unsigned char RegisterAddress , unsigned int Data) {
    struct CoreInformation *CoreInformation = CoreInformation::GetInstance();
    *((unsigned char *)(CoreInformation->IOAPICAddress+IOAPIC_REGISTER_SELECTOR)) = RegisterAddress;
    *((unsigned int *)(CoreInformation->IOAPICAddress+IOAPIC_REGISTER_WINDOW)) = Data;
}

unsigned int Kernel::IOAPIC::ReadRegister(unsigned char RegisterAddress) {
    struct CoreInformation *CoreInformation = CoreInformation::GetInstance();
    *((unsigned char *)(CoreInformation->IOAPICAddress+IOAPIC_REGISTER_SELECTOR)) = RegisterAddress;
    return *((unsigned int *)(CoreInformation->IOAPICAddress+IOAPIC_REGISTER_WINDOW));
}

void Kernel::LocalAPIC::GlobalEnableLocalAPIC(void) {
    // Disable all interrupts
    IO::Write(0x21 , 0xFF);
    IO::Write(0xA1 , 0xFF);
    // Enable Local APIC, set E bit in APIC_BASE MSR to 1
    // MSR 0x1B : APIC_BASE
    // Contains 32bits of Local APIC base and E flag, which if it enabled, Local APIC is globally enabled.
    __asm__ ("mov rcx , 0x1B");
    __asm__ ("rdmsr");
    __asm__ ("or eax , 0b100000000000");    // E(APIC Global Enable flag) = 1
    __asm__ ("wrmsr");
}

void Kernel::LocalAPIC::EnableLocalAPIC(void) {
    unsigned int EAX;
    unsigned int EDX;
    struct CoreInformation *CoreInformation = CoreInformation::GetInstance();
    // Kernel::printf("LocalAPIC Register Address from the structure : 0x%X\n" , CoreInformation->LocalAPICAddress);
    // To enable APIC, set bit 8 in LocalAPIC Spurious interrupt vector register to 1 
    // Currently, we're enabling BSP's Local APIC
    WriteRegister(LAPIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER , ReadRegister(LAPIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER)|0x100);
    // For debugging, read the register again
    // Kernel::printf("LAPIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER      : 0x%X\n" , ReadRegister(LAPIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER));
}

bool Kernel::LocalAPIC::CheckBSP(void) {    // Check if current CPU is BSP
    unsigned int EAX;
    // MSR 0x1B : APIC_BASE
    // Check if bit 8, which is BSP bit, is enabled.
    // If it is, currently running core is BSP, it it isn't currently running core is AP.
    __asm__ ("mov rcx , 0x1B");
    __asm__ ("rdmsr");
    __asm__ ("mov %0 , eax":"=r"(EAX)); // We just need lower 32bits of the register
    if((EAX & 0b100000000) == 0b100000000) {
        return true;
    }
    else {
        return false;
    }
}

unsigned int ActivatedCoreCount = 0;
/*
void Kernel::LocalAPIC::SendActivatedSignal(void) {
    CoreActivated = 1;          // AP sends this signal, and BSP gets it
    ActivatedCoreCount++;
}*/

unsigned int Kernel::LocalAPIC::GetActivatedCoreCount(void) {
    return ActivatedCoreCount;
}

unsigned int Kernel::LocalAPIC::GetCurrentAPICID(void) {
    // Local APIC ID Register contains currently running core's APIC ID, which is in bit 24-31
    return (unsigned int)(Kernel::LocalAPIC::ReadRegister(LAPIC_ID_REGISTER) >> 24);
}

void Kernel::LocalAPIC::ActiveAPCores(void) {
    int i;
    int j;
    int ActivatedCoreCount;
    unsigned int BSPAPICID;
    // Wow so I made two running core counter
    unsigned long LocalAPICBootLoaderLocation = 0x8000; // Pre-saved APIC boot loader location
    unsigned short *RunningCoreCount = (unsigned short *)(LocalAPICBootLoaderLocation+2+1);
    struct CoreInformation *CoreInformation = CoreInformation::GetInstance();
    if(CheckBSP() == 0) {
        Kernel::printf("The current core is AP core!\n");
        return;
    }
    // Local APIC ID Register contains BSP core APIC ID
    // Since it's confirmed that we're running in BSP core, by simply getting the current APIC ID, we can get the BSP core APIC ID.
    BSPAPICID = GetCurrentAPICID();
    
    __asm__ ("cli");
    // Level Trigger : For INIT IPI
    // Level Trigger , Delivery Mode : INIT , Sending Level : Assert , Using Abbreviation , For every core
    WriteRegister(LAPIC_ERROR_STATUS_REGISTER , 0x00);
    // Send INIT IPI for every core usinb abbreviation.
    
    // To-do : comment
    WriteRegister(LAPIC_INTERRUPT_COMMAND_REGISTER , LAPIC_ICR_IPI_INIT|LAPIC_ICR_LEVEL_ASSERT|LAPIC_ICR_SENDING_FOR_EVERYONE_EXCEPT_FOR_ME);
    WriteRegister(0x310 , ReadRegister(0x310)|((CoreInformation->LocalAPICID[i] & 0b1111) << 24));
    // Wait 10ms for the response
    Kernel::PIT::DelayMilliseconds(10);
    // If Local APIC status is pending, sending IPI is failed. :/
    while((ReadRegister(LAPIC_INTERRUPT_COMMAND_REGISTER) & LAPIC_ICR_SENT_STATUS_PENDING) == LAPIC_ICR_SENT_STATUS_PENDING) {
        // Kernel::printf("Failed sending Init IPI for : LAPIC 0x%X, Processor 0x%X\n" , CoreInformation->LocalAPICID[i] , CoreInformation->LocalAPICProcessorID[i]);
        // return;
    }
    for(j = 0; j < 2; j++) {
        WriteRegister(LAPIC_ERROR_STATUS_REGISTER , 0x00);
        WriteRegister(LAPIC_INTERRUPT_COMMAND_REGISTER , LAPIC_ICR_SENDING_FOR_EVERYONE_EXCEPT_FOR_ME|LAPIC_ICR_LEVEL_ASSERT|LAPIC_ICR_IPI_STARTUP|0x08);
        while((ReadRegister(LAPIC_INTERRUPT_COMMAND_REGISTER) & LAPIC_ICR_SENT_STATUS_PENDING) == LAPIC_ICR_SENT_STATUS_PENDING) {
            // Kernel::printf("Failed sending AP Setup IPI for : LAPIC 0x%X, Processor 0x%X\n" , CoreInformation->LocalAPICID[i] , CoreInformation->LocalAPICProcessorID[i]);
            // return;
        }
    }
    
    //__asm__ ("sti");
    while(1) { // Wait until activated core count is equal to number of cores
        ActivatedCoreCount = GetActivatedCoreCount();
        Kernel::PIT::DelayMilliseconds(10);
        if(ActivatedCoreCount == GetActivatedCoreCount()) {
            break;
        }
    }
    CoreInformation::GetInstance()->CoreCount = (GetActivatedCoreCount()+1);
    Kernel::printf("Activated Core Count : %d\n" , CoreInformation::GetInstance()->CoreCount);
}

void Kernel::LocalAPIC::SendEOI(void) { // Send End Of Interrupt Signal to APIC
    // By setting EOI Register to 0, we can send the EOI signal to APIC.
    Kernel::LocalAPIC::WriteRegister(LAPIC_EOI_REGISTER , 0);
}

///// Timer //////

volatile unsigned int InitialLocalAPICTickCount;

void Kernel::LocalAPIC::Timer::Initialize(void) {
    // Kernel::printf("Initializing Timer\n");
    if(CheckBSP() == true) {
        WriteRegister(LAPIC_DIVIDE_CONFIG_REGISTER , 0b0011);
        WriteRegister(LAPIC_INITIAL_COUNT_REGISTER , 0xFFFFFFFF); // Initial value of the counter
        PIT::DelayMicroseconds(10); // kinda dangerous?
        WriteRegister(LAPIC_LVT_TIMER_REGISTER , LAPIC_TIMER_MASK_INTERRUPT);
        InitialLocalAPICTickCount = 0xFFFFFFFF-LocalAPIC::Timer::GetTickCount();
    }
    WriteRegister(LAPIC_LVT_TIMER_REGISTER , (0b10 << 16)|41); // LVT Timer Register, determins how interrupt occur.
    WriteRegister(LAPIC_DIVIDE_CONFIG_REGISTER , 0b0011);
    WriteRegister(LAPIC_INITIAL_COUNT_REGISTER , InitialLocalAPICTickCount); // Initial value of the counter
    // Kernel::printf("Initial LocalAPIC Tick Count : %d\n" , InitialLocalAPICTickCount);
    TimerSpinLock.Initialize();
}

void Kernel::LocalAPIC::Timer::SetInitialValue(unsigned int Value) {
    WriteRegister(LAPIC_INITIAL_COUNT_REGISTER , Value); // Initial value of the counter
}

void Kernel::LocalAPIC::Timer::Enable(void) {
    WriteRegister(LAPIC_INITIAL_COUNT_REGISTER , InitialLocalAPICTickCount);
}

void Kernel::LocalAPIC::Timer::Disable(void) {
    WriteRegister(LAPIC_INITIAL_COUNT_REGISTER , 0);
}

unsigned int Kernel::LocalAPIC::Timer::GetTickCount(void) {
    return ReadRegister(LAPIC_CURRENT_COUNT_REGISTER);
}

unsigned int Kernel::LocalAPIC::Timer::GetInitialValue(void) {
    return InitialLocalAPICTickCount;
}

void Kernel::LocalAPIC::Timer::MainInterruptHandler(void) {
    // Kernel::TaskManagement::SwitchTaskInTimerInterrupt();
    Kernel::LocalAPIC::SendEOI();
}

/// I/O APIC ///

void Kernel::IOAPIC::InitializeRedirectionTable(void) {
    int i;
    unsigned long TableAddress;
    struct MPFloatingTable::MPFloatingPointer *MPFloatingPointer = (struct MPFloatingTable::MPFloatingPointer *)MPFloatingTable::FindMPFloatingTable();
    struct MPFloatingTable::MPFloatingTableHeader *TableHeader = (struct MPFloatingTable::MPFloatingTableHeader *)MPFloatingPointer->PhysicalAddressPointer;
    
    struct MPFloatingTable::Entries::IOInterruptAssignment *IOInterruptAssignmentEntry;
    struct MPFloatingTable::Entries::LocalInterrupt *LocalInterruptEntry;
    
    struct IORedirectionTable RedirectionTable;
    memset(&(RedirectionTable) , 0 , sizeof(struct IORedirectionTable));
    if(MPFloatingPointer == 0x00) {
        Kernel::printf("MP floating pointer not found.\n");
        while(1) {
            ;
        }
    }
    Kernel::printf("MP Table Entry Count : %d\n" , TableHeader->EntryCount);
    TableAddress = MPFloatingPointer->PhysicalAddressPointer+sizeof(struct MPFloatingTable::MPFloatingTableHeader);
    for(i = 0; i < TableHeader->EntryCount; i++) {
        switch(*((unsigned char*)TableAddress)) {
            case 0:
                TableAddress += sizeof(struct MPFloatingTable::Entries::Processor);
                break;
            case 1:
                TableAddress += sizeof(struct MPFloatingTable::Entries::BUS);
                break;
            case 2:
                TableAddress += sizeof(struct MPFloatingTable::Entries::IOAPIC);
                break;
            case 3:
                IOInterruptAssignmentEntry = (struct MPFloatingTable::Entries::IOInterruptAssignment *)TableAddress;
                RedirectionTable.DeliveryMode = IOAPIC_IOREDTBL_DELIVERY_MODE_FIXED;
                RedirectionTable.DestinationMode = 0x00;
                RedirectionTable.DestinationAddress = 0x00;
                RedirectionTable.InterruptVector = IOInterruptAssignmentEntry->SourceBusIRQ+0x20;
                WriteIORedirectionTable(IOInterruptAssignmentEntry->DestinationIOAPICINTIN , RedirectionTable);
                // Kernel::printf("INTIN %d -> IRQ %d\n" , IOInterruptAssignmentEntry->DestinationIOAPICINTIN , IOInterruptAssignmentEntry->SourceBusIRQ);
                TableAddress += sizeof(struct MPFloatingTable::Entries::IOInterruptAssignment);
                break;
            case 4:
                TableAddress += sizeof(struct MPFloatingTable::Entries::LocalInterrupt);
                break;
            default:
                TableAddress += 8;
                break;
        };
    }
    
}

void Kernel::IOAPIC::WriteIORedirectionTable(int INTIN , struct IORedirectionTable RedirectionTable) {
    unsigned long Data;
    memcpy(&(Data) , &(RedirectionTable) , sizeof(unsigned long));
    IOAPIC::WriteRegister(IOAPIC_REGISTER_IO_REDIRECTION_TABLE+(INTIN*2) , Data & 0xFFFFFFFF);
    IOAPIC::WriteRegister(IOAPIC_REGISTER_IO_REDIRECTION_TABLE+(INTIN*2)+1 , Data >> 32);
}
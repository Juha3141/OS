#include <APIC.hpp>
#include <PIT.hpp>

void Kernel::LocalAPIC::WriteRegister(unsigned int RegisterAddress , unsigned int Data) {
    CPUProcessorsInformation *CoreInformation = Kernel::ACPI::GetCoresInformation();
    *((unsigned int *)(CoreInformation->LocalAPICAddress+RegisterAddress)) = Data;
}

void Kernel::LocalAPIC::WriteRegister_L(unsigned int RegisterAddress , unsigned long Data) {
    CPUProcessorsInformation *CoreInformation = Kernel::ACPI::GetCoresInformation();
    *((unsigned long *)(CoreInformation->LocalAPICAddress+RegisterAddress)) = Data;
}
    
unsigned int Kernel::LocalAPIC::ReadRegister(unsigned int RegisterAddress) {
    CPUProcessorsInformation *CoreInformation = Kernel::ACPI::GetCoresInformation();
    return *((unsigned int *)(CoreInformation->LocalAPICAddress+RegisterAddress));
}
unsigned int Kernel::LocalAPIC::ReadRegister_L(unsigned int RegisterAddress) {
    CPUProcessorsInformation *CoreInformation = Kernel::ACPI::GetCoresInformation();
    return *((unsigned long *)(CoreInformation->LocalAPICAddress+RegisterAddress));
}


void Kernel::LocalAPIC::EnableLocalAPIC(void) {
    unsigned int EAX;
    unsigned int EDX;
    CPUProcessorsInformation *CoreInformation = Kernel::ACPI::GetCoresInformation();
    IO::Write(0x21 , 0xFF);
    IO::Write(0xA1 , 0xFF);

    __asm__ ("mov rcx , 27");
    __asm__ ("rdmsr");
    __asm__ ("or eax , 0b100000000000");
    __asm__ ("wrmsr");
    __asm__ ("mov rcx , 27");
    __asm__ ("rdmsr");
    Kernel::printf("LocalAPIC Register Address : 0x%X\n" , CoreInformation->LocalAPICAddress);
    WriteRegister(LAPIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER , ReadRegister(LAPIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER)|0x100);
    Kernel::printf("LAPIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER : 0x%X\n" , ReadRegister(LAPIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER));
}

bool Kernel::LocalAPIC::CheckBSP(void) {
    unsigned int EAX;
    unsigned int EDX;
    __asm__ ("mov rcx , 27");
    __asm__ ("rdmsr");
    __asm__ ("mov %0 , eax":"=r"(EAX));
    __asm__ ("mov %0 , edx":"=r"(EDX));
    if((EAX & 0b100000000) == 0b100000000) {
        return 1;
    }
    else {
        return 0;
    }
}

unsigned int CurrentlyRunningCoreCount = 0;

void Kernel::LocalAPIC::ActiveAPCores(void) {
    int i;
    int j;
    unsigned int BSPAPICID;
    unsigned short *RunningCoreCount = (unsigned short *)(0x8000+2+1);
    unsigned long LocalAPICBootLoaderLocation = 0x8000;
    CPUProcessorsInformation *CoreInformation = Kernel::ACPI::GetCoresInformation();
    if(CheckBSP() == 0) {
        Kernel::printf("The current core is AP core!\n");
        return;
    }
    BSPAPICID = ((ReadRegister(0x20) >> (1 << 24)) & 0b11111111);
    
    __asm__ ("cli");
    // Level Trigger : For INIT IPI
    // Level Trigger , Delivery Mode : INIT , Sending Level : Assert , Not using Abbreviation
    /*for(i = 0; i < CoreInformation->CoreCount; i++) {
        if(CoreInformation->LocalAPICID[i] == BSPAPICID) {
            Kernel::printf("Skip BSP\n");
        }
        else {
            WriteRegister(LAPIC_ERROR_STATUS_REGISTER , 0x00);
            // Level Trigger : For INIT IPI
            // Level Trigger , Delivery Mode : INIT , Sending Level : Assert , Not using Abbreviation
            WriteRegister(0x310 , ReadRegister(0x310)|((CoreInformation->LocalAPICID[i] & 0b1111) << 24));
            WriteRegister(LAPIC_INTERRUPT_COMMAND_REGISTER , LAPIC_ICR_IPI_INIT|LAPIC_ICR_LEVEL_TRIGGER|LAPIC_ICR_LEVEL_ASSERT|LAPIC_ICR_NOT_USING_ABBREVIATION);
            Kernel::PIT::DelayMilliseconds(10);
            if((ReadRegister(LAPIC_INTERRUPT_COMMAND_REGISTER) & LAPIC_ICR_SENT_STATUS_PENDING) == LAPIC_ICR_SENT_STATUS_PENDING) {
                Kernel::printf("Failed sending Init IPI for : LAPIC 0x%X, Processor 0x%X\n" , CoreInformation->LocalAPICID[i] , CoreInformation->LocalAPICProcessorID[i]);
                return;
            }
            for(j = 0; j < 2; j++) {
                WriteRegister(0x310 , ReadRegister(0x310)|((CoreInformation->LocalAPICID[i] & 0b1111) << 24));
                WriteRegister(LAPIC_INTERRUPT_COMMAND_REGISTER , LAPIC_ICR_NOT_USING_ABBREVIATION|LAPIC_ICR_LEVEL_ASSERT|LAPIC_ICR_PHYSICAL_DESTINATION_MODE|LAPIC_ICR_IPI_STARTUP|0x08);
                Kernel::PIT::DelayMicroseconds(200);
                if((ReadRegister(LAPIC_INTERRUPT_COMMAND_REGISTER) & LAPIC_ICR_SENT_STATUS_PENDING) == LAPIC_ICR_SENT_STATUS_PENDING) {
                    Kernel::printf("Failed sending AP Setup IPI for : LAPIC 0x%X, Processor 0x%X\n" , CoreInformation->LocalAPICID[i] , CoreInformation->LocalAPICProcessorID[i]);
                    return;
                }
            }
        }
    }*/
    WriteRegister(LAPIC_ERROR_STATUS_REGISTER , 0x00);
    WriteRegister(LAPIC_INTERRUPT_COMMAND_REGISTER , LAPIC_ICR_IPI_INIT|LAPIC_ICR_LEVEL_ASSERT|LAPIC_ICR_SENDING_FOR_EVERYONE_EXCEPT_FOR_ME);
    WriteRegister(0x310 , ReadRegister(0x310)|((CoreInformation->LocalAPICID[i] & 0b1111) << 24));
    Kernel::PIT::DelayMilliseconds(10);
    if((ReadRegister(LAPIC_INTERRUPT_COMMAND_REGISTER) & LAPIC_ICR_SENT_STATUS_PENDING) == LAPIC_ICR_SENT_STATUS_PENDING) {
        Kernel::printf("Failed sending Init IPI for : LAPIC 0x%X, Processor 0x%X\n" , CoreInformation->LocalAPICID[i] , CoreInformation->LocalAPICProcessorID[i]);
        return;
    }
    for(j = 0; j < 2; j++) {
        WriteRegister(LAPIC_ERROR_STATUS_REGISTER , 0x00);
        WriteRegister(LAPIC_INTERRUPT_COMMAND_REGISTER , LAPIC_ICR_SENDING_FOR_EVERYONE_EXCEPT_FOR_ME|LAPIC_ICR_LEVEL_ASSERT|LAPIC_ICR_IPI_STARTUP|0x08);
        Kernel::PIT::DelayMicroseconds(200);
        if((ReadRegister(LAPIC_INTERRUPT_COMMAND_REGISTER) & LAPIC_ICR_SENT_STATUS_PENDING) == LAPIC_ICR_SENT_STATUS_PENDING) {
            Kernel::printf("Failed sending AP Setup IPI for : LAPIC 0x%X, Processor 0x%X\n" , CoreInformation->LocalAPICID[i] , CoreInformation->LocalAPICProcessorID[i]);
            return;
        }
    }
}
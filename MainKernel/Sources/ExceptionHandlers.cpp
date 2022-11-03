#include <ExceptionHandlers.hpp>
#include <Kernel.hpp>

// Function usage : Mask the specified interrupt
// Master : controlls
void Kernel::PIC::Mask(int InterruptNumber) {
    unsigned short Data = IO::Read(0x21)|(IO::Read(0xA1) << 8);
    if(InterruptNumber < 32) {
        return;
    }
    Data |= (0x01 << (InterruptNumber-32));
    IO::Write(0x21 , Data & 0xFF);
    IO::Write(0xA1 , (Data >> 8) & 0xFF);
}

// Function usage : Unmask the specified interrupt
void Kernel::PIC::Unmask(int InterruptNumber) {
    unsigned short Data = IO::Read(0x21)|(IO::Read(0xA1) << 8);
    if(InterruptNumber < 32) {
        return;
    }
    if((Data & (0x01 << (InterruptNumber-32))) == (0x01 << (InterruptNumber-32))) {
        Data ^= (0x01 << (InterruptNumber-32));
    }
    IO::Write(0x21 , Data & 0xFF);
    IO::Write(0xA1 , (Data >> 8) & 0xFF);
}

// Function usage : Send EOI(End of interrupt) signal to master PIC
void Kernel::PIC::SendEOI(int InterruptNumber) {
    IO::Write(0x20 , 0x20);     // 0x20 : EOI signal, send EOI to the master
    if(InterruptNumber >= 32+8) {
        IO::Write(0xA0 , 0x20); // send EOI to the slave
    }
}

__attribute__ ((naked)) void Kernel::Exceptions::DividedByZero(void) {
    ProcessExceptions(0 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::Debug(void) {
    ProcessExceptions(1 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::NonMaskableInterrupt(void) {
    ProcessExceptions(2 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::Breakpoint(void) {
    ProcessExceptions(3 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::Overflow(void) {
    ProcessExceptions(4 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::BoundRangeExceeded(void) {
    ProcessExceptions(5 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::InvalidOpcode(void) {
    ProcessExceptions(6 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::DeviceNotAvailable(void) {
    ProcessExceptions(7 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::DoubleFault(void) {
    unsigned long ErrorCode;
    __asm__ ("mov %0 , [rsp+8]"::"r"((ErrorCode)));
    ProcessExceptions(8 , ErrorCode);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::CorprocessorSegmentOverrun(void) {
    ProcessExceptions(9 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::InvalidTSS(void) {
    unsigned long ErrorCode;
    __asm__ ("mov %0 , [rsp+8]"::"r"((ErrorCode)));
    ProcessExceptions(10 , ErrorCode);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::SegmentNotPresent(void) {
    unsigned long ErrorCode;
    __asm__ ("mov %0 , [rsp+8]"::"r"((ErrorCode)));
    ProcessExceptions(11 , ErrorCode);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::StackSegmentFault(void) {
    unsigned long ErrorCode;
    __asm__ ("mov %0 , [rsp+8]"::"r"((ErrorCode)));
    ProcessExceptions(12 , ErrorCode);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::GeneralProtectionFault(void) {
    unsigned long ErrorCode;
    __asm__ ("mov %0 , [rsp+8]"::"r"((ErrorCode)));
    ProcessExceptions(13 , ErrorCode);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::PageFault(void) {
    unsigned long ErrorCode;
    __asm__ ("mov %0 , [rsp+8]"::"r"((ErrorCode)));
    ProcessExceptions(14 , ErrorCode);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::Reserved15(void) {
    ProcessExceptions(15 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::x87FloatPointException(void) {
    ProcessExceptions(16 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::AlignmentCheck(void) {
    unsigned long ErrorCode;
    __asm__ ("mov %0 , [rsp+8]"::"r"((ErrorCode)));
    ProcessExceptions(17 , ErrorCode);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::MachineCheck(void) {
    ProcessExceptions(18 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::SIMDFloatingPointException(void) {
    ProcessExceptions(19 , 0x00);
}

__attribute__ ((naked)) void Kernel::Exceptions::VirtualizationException(void) {
    ProcessExceptions(20 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::ControlProtectionException(void) {
    unsigned long ErrorCode;
    __asm__ ("mov %0 , [rsp+8]"::"r"((ErrorCode)));
    ProcessExceptions(21 , ErrorCode);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::Reserved22(void) {
    ProcessExceptions(22 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::Reserved23(void) {
    ProcessExceptions(23 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::Reserved24(void) {
    ProcessExceptions(24 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::Reserved25(void) {
    ProcessExceptions(25 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::Reserved26(void) {
    ProcessExceptions(26 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::Reserved27(void) {
    ProcessExceptions(27 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::HypervisorInjectionException(void) {
    ProcessExceptions(28 , 0x00);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::VMMCommunicationException(void) {
    unsigned long ErrorCode;
    __asm__ ("mov %0 , [rsp+8]"::"r"((ErrorCode)));
    ProcessExceptions(29 , ErrorCode);
    __asm__ ("iretq");
}

__attribute__ ((naked)) void Kernel::Exceptions::SecurityException(void) {
    unsigned long ErrorCode;
    __asm__ ("mov %0 , [rsp+8]"::"r"((ErrorCode)));
    ProcessExceptions(30 , ErrorCode);
    __asm__ ("iretq");
}

void Kernel::Exceptions::ProcessExceptions(int ExceptionNumber, unsigned long ErrorCode) {
    char ExceptionNames[29][32] = {
        "Divided by Zero" , "Debug" , "Non-Maskable Interrupt" , 
        "Breakpoint" , "Overflow" , "Bound Range Exceeded" , 
        "Invalid Opcode" , "Device not Available" , "Double Fault" , 
        "Corprocessor Segment Overrun" , "Invalid TSS" , "Segment Not Present" , 
        "Stack Segment Fault" , "General Protection Fault" , "Page Fault" , 
        "Reserved (Interrupt number 15)" , "x87 Floating Point Exception" , 
        "Alignment Check" , "Machine Check" , "SIMD Floating Point Exception" , 
        "Virtualization Exception" , "Control Protection Exception" , "Reserved" , 
        "Hypervisor Injection Exception" , "VMM Communication Exception" , 
        "Security Exception" , 
    };
    __asm__ ("cli");/*
    Kernel::ClearScreen(0x00 , 0x04);*/
    Kernel::PrintString("[Exception occurred]\n");
    Kernel::printf("Name         : \"%s\"\n" , ExceptionNames[ExceptionNumber]);
    Kernel::printf("IST Location : 0x%X\n" , ((IST_STARTADDRESS+IST_SIZE)-sizeof(struct STACK_STRUCTURE)));
    while(1) {
        ;
    }
}
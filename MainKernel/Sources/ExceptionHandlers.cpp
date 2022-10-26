#include <ExceptionHandlers.hpp>

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
    struct STACK_STRUCTURE *IST = (struct STACK_STRUCTURE *)((IST_STARTADDRESS+IST_SIZE)-sizeof(struct STACK_STRUCTURE));
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
    Kernel::printf("Exception occurred in %s%s" , (IST->CS == USER_CS) ? "User Level" : "Kernel Level" , 
                                                  (IST->CS == USER_CS) ? " [Program Terminated]\n" : " [Kernel System Aborted]\n");
    Kernel::printf("Location   : 0x%02X\n" , IST->RIP);
    Kernel::printf("Name       : \"%s\"\n" , ExceptionNames[ExceptionNumber]);
    if((ExceptionNumber == 8)||(ExceptionNumber == 10)||(ExceptionNumber == 11)
     ||(ExceptionNumber == 12)||(ExceptionNumber == 13)||(ExceptionNumber == 14)
     ||(ExceptionNumber == 17)||(ExceptionNumber == 21)||(ExceptionNumber == 29)
     ||(ExceptionNumber == 30)) {
        Kernel::printf("Error Code : 0x%X\n" , ErrorCode);
    }/*
    Kernel::printf("Registers  : \n");
    Kernel::printf("RAX=%016x   RBX   =%016x   RCX=%016x\n" , IST->RAX , IST->RBX , IST->RCX);
    Kernel::printf("RDX=%016x   RDI   =%016x   RSI=%016x\n" , IST->RDX , IST->RDI , IST->RSI);
    Kernel::printf("R8 =%016x   R9    =%016x   R10=%016x\n" , IST->R8  , IST->R9  , IST->R10);
    Kernel::printf("R11=%016x   R12   =%016x   R13=%016x\n" , IST->R11 , IST->R12 , IST->R13);
    Kernel::printf("R14=%016x   R15   =%016x   RBP=%016x\n" , IST->R14 , IST->R15 , IST->RBP);
    Kernel::printf("RSP=%016x   RFlags=%016x   CS =%016x\n" , IST->RSP , IST->RFlags , IST->CS);
    Kernel::printf("DS =%016x   ES    =%016x   FS =%016x\n" , IST->DS , IST->ES , IST->FS);
    Kernel::printf("GS =%016x   SS    =%016x   \n" , IST->GS , IST->SS);
    Kernel::printf("Press power button to restart the machine.\n");*/
    while(1) {
        ;
    }
}
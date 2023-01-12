#include <ExceptionHandlers.hpp>
#include <Kernel.hpp>
#include <Graphics/Graphic.hpp>

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

void Kernel::Exceptions::ProcessExceptions(int ExceptionNumber , unsigned long ErrorCode) {
    unsigned long CR0;
    unsigned long CR1;
    unsigned long CR2;
    unsigned long CR3;
    unsigned long CR4;
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
    struct Kernel::STACK_STRUCTURE *IST = (struct Kernel::STACK_STRUCTURE *)(IST_STARTADDRESS+IST_SIZE-sizeof(struct Kernel::STACK_STRUCTURE));
    __asm__ ("cli");
    Kernel::PrintString("[Exception occurred]\n");
    __asm__ ("mov %0 , cr0":"=r"(CR0));
    __asm__ ("mov %0 , cr1":"=r"(CR1));
    __asm__ ("mov %0 , cr2":"=r"(CR2));
    __asm__ ("mov %0 , cr3":"=r"(CR3));
    __asm__ ("mov %0 , cr4":"=r"(CR4));
    Kernel::printf("Vector Number : %d(%s)\n" , ExceptionNumber , ExceptionNames[ExceptionNumber]);
    Kernel::printf("IST Location  : 0x%X\n" , ((IST_STARTADDRESS+IST_SIZE)-sizeof(struct STACK_STRUCTURE)));
    Kernel::printf("Dumping Registers : \n");
    Kernel::printf("RAX=0x%X RBX=0x%X RCX=0x%X RDX=0x%X\n" , IST->RAX , IST->RBX , IST->RCX , IST->RDX);
    Kernel::printf("RDI=0x%X RSI=0x%X R8=0x%X R9=0x%X\n" , IST->RDI , IST->RSI , IST->R8 , IST->R9);
    Kernel::printf("R10=0x%X R11=0x%X R12=0x%X R13=0x%X\n" , IST->R10 , IST->R11 , IST->R12 , IST->R13);
    Kernel::printf("R14=0x%X R15=0x%X RBP=0x%X RSP=0x%X\n" , IST->R14 , IST->R15 , IST->RBP , IST->RSP);
    Kernel::printf("RIP=0x%X RFlags=0x%X CS=0x%X DS=0x%X\n" , IST->RIP , IST->RFlags , IST->CS , IST->DS);
    Kernel::printf("ES=0x%X FS=0x%X GS=0x%X SS=0x%X\n" , IST->ES , IST->FS , IST->GS , IST->SS);
    while(1) {
        __asm__ ("hlt");
    }
}
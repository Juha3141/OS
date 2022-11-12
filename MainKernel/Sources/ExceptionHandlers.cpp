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
   /*static char ExceptionNames[29][32] = {
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
    };*/
    /*
     * FIXME : Large array causes error in the entire kernel
    */
    __asm__ ("cli");/*
    Kernel::ClearScreen(0x00 , 0x04);*/
    Kernel::PrintString("[Exception occurred]\n");
    Graphics::VBE::DrawText(10 , 10 , 0xFF0000 , "[Exception occurred]");
    Kernel::printf("Vector Number : %d\n" , ExceptionNumber);
    Graphics::VBE::DrawText(10 , 10+16 , 0xFF0000 , "Vector Number : %d\n" , ExceptionNumber);
    Kernel::printf("IST Location  : 0x%X\n" , ((IST_STARTADDRESS+IST_SIZE)-sizeof(struct STACK_STRUCTURE)));
    Graphics::VBE::DrawText(10 , 10+(16*2) , 0xFF0000 , "IST Location  : 0x%X\n" , ((IST_STARTADDRESS+IST_SIZE)-sizeof(struct STACK_STRUCTURE)));
    while(1) {
        ;
    }
}
#include <Keyboard.hpp>

__attribute__ ((naked)) void Kernel::Keyboard::InterruptHandler(void) {
    __asm__ ("push rbp");
    __asm__ ("mov rbp , rsp");

    SAVE_REGISTERS_TO_STACK();


    IO::Read(0x60);
    IO::Write(0x20 , 0x20);
    
    LOAD_REGISTERS_FROM_STACK();

    __asm__ ("leave");
    __asm__ ("iretq");
}
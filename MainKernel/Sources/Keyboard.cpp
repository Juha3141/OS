#include <Keyboard.hpp>

void Kernel::Keyboard::MainInterruptHandler(void) {
    Kernel::printf("A");
    IO::Read(0x60);
    
    PIC::SendEOI(33);
}
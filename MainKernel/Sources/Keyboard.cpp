#include <Keyboard.hpp>

void Kernel::Keyboard::MainInterruptHandler(void) {
    Kernel::printf("A");
    IO::Read(0x60);

    IO::Write(0x20 , 0x20);
}
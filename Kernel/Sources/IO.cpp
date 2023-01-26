#include <Kernel.hpp>

unsigned char IO::Read(unsigned short Port) {
    unsigned char Data;
    __asm__ ("mov dx , %0"::"r"(Port));
    __asm__ ("in al , dx");
    __asm__ ("mov %0 , al":"=r"(Data));
    return Data;
}

void IO::Write(unsigned short Port , unsigned char Data) {
    __asm__ ("mov dx , %0"::"r"(Port));
    __asm__ ("mov al , %0"::"r"(Data));
    __asm__ ("out dx , al");
}
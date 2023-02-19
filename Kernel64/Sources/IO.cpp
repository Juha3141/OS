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

unsigned short IO::ReadWord(unsigned short Port) {
    unsigned short Data;
    __asm__ ("mov dx , %0"::"r"(Port));
    __asm__ ("in ax , dx");
    __asm__ ("mov %0 , ax":"=r"(Data));
    return Data;
}

void IO::WriteWord(unsigned short Port , unsigned short Data) {
    __asm__ ("mov dx , %0"::"r"(Port));
    __asm__ ("mov ax , %0"::"r"(Data));
    __asm__ ("out dx , ax");
}

unsigned int IO::ReadDWord(unsigned short Port) {
    unsigned int Data;
    __asm__ ("mov dx , %0"::"r"(Port));
    __asm__ ("in eax , dx");
    __asm__ ("mov %0 , eax":"=r"(Data));
    return Data;
}

void IO::WriteDWord(unsigned short Port , unsigned int Data) {
    __asm__ ("mov dx , %0"::"r"(Port));
    __asm__ ("mov eax , %0"::"r"(Data));
    __asm__ ("out dx , eax");
}
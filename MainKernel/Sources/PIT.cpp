#include <PIT.hpp>

static volatile unsigned short PITFrequency;
static unsigned long TickCount = 0;

__attribute__ ((naked)) void Kernel::PIT::InterruptHandler(void) {
    __asm__ ("push rbp");
    __asm__ ("mov rbp , rsp");

    SAVE_REGISTERS_TO_STACK();

    __asm__ ("mov al , 0x20");
    __asm__ ("out 0x20 , al");

    TickCount += 1;
    
    LOAD_REGISTERS_FROM_STACK();

    __asm__ ("leave");
    __asm__ ("iretq");
}

unsigned long Kernel::PIT::GetTickCount(void) {
    return TickCount;
}

void Kernel::PIT::Initialize(void) {
    IO::Write(0x21 , 0b11111110);
    PITFrequency = PIT_CONVERT_US_TO_HZ(100);
    
    IO::Write(PIT_MODE_COMMAND_REGISTER , 0b00110000);
    IO::Write(PIT_MODE_COMMAND_REGISTER , 0b00110100); // 16bit binary mode , Rate generator , Channel 0
    IO::Write(PIT_CHANNEL0_DATA , PITFrequency & 0xFF);
    IO::Write(PIT_CHANNEL0_DATA , (PITFrequency >> 8) & 0xFF);
}

unsigned short Kernel::PIT::GetCurrentPITFrequency(void) {
    return PITFrequency;
}

unsigned short Kernel::PIT::ReadPITCounter(unsigned char CounterNumber) {
    IO::Write(PIT_MODE_COMMAND_REGISTER , (CounterNumber & 0b11) << 6);
    return IO::Read(PIT_CHANNEL0_DATA+(CounterNumber & 0b11))|(IO::Read(PIT_CHANNEL0_DATA+(CounterNumber & 0b11)) << 8);
}

void Kernel::PIT::DelayByPITCount(unsigned int PITCount) {
    unsigned short CounterValue = ReadPITCounter(PIT_COUNTER0);
    while(1) {
        if((CounterValue-ReadPITCounter(PIT_COUNTER0)) >= PITCount) {
            break;
        }
    }
}

void Kernel::PIT::DelayMilliseconds(unsigned int Milliseconds) {
    int i;
    if(Milliseconds == 0) {
        __asm__ ("nop");
        return;   
    }
    IO::Write(PIT_MODE_COMMAND_REGISTER , 0b00110100); // 16bit binary mode , Rate generator , Channel 0
    IO::Write(PIT_CHANNEL0_DATA , 0x00);
    IO::Write(PIT_CHANNEL0_DATA , 0x00);
    for(i = 0; i < Milliseconds; i++) {
        DelayByPITCount(PIT_CONVERT_MS_TO_HZ(1));
    }
    IO::Write(PIT_MODE_COMMAND_REGISTER , 0b00110100);
    IO::Write(PIT_CHANNEL0_DATA , PITFrequency & 0xFF);
    IO::Write(PIT_CHANNEL0_DATA , (PITFrequency >> 8) & 0xFF);
}

void Kernel::PIT::DelayMicroseconds(unsigned int Microseconds) {
    int i;
    if(Microseconds == 0) {
        __asm__ ("nop");
        return;   
    }
    if(Microseconds%1000 == 0) {
        DelayMilliseconds(Microseconds/1000);
        return;
    }
    IO::Write(PIT_MODE_COMMAND_REGISTER , 0b00110100); // 16bit binary mode , Rate generator , Channel 0
    IO::Write(PIT_CHANNEL0_DATA , 0x00);
    IO::Write(PIT_CHANNEL0_DATA , 0x00);
    if(Microseconds <= 100) {
        DelayByPITCount(PIT_CONVERT_US_TO_HZ(Microseconds));
    }
    else {
        for(i = 0; i < Microseconds/100; i++) {
            DelayByPITCount(PIT_CONVERT_US_TO_HZ(100));
        }
        if(Microseconds%100 != 0) {
            DelayByPITCount(Microseconds%100);
        }
    }
    IO::Write(PIT_MODE_COMMAND_REGISTER , 0b00110100);
    IO::Write(PIT_CHANNEL0_DATA , PITFrequency & 0xFF);
    IO::Write(PIT_CHANNEL0_DATA , (PITFrequency >> 8) & 0xFF);
}
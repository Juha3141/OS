#include <PIT.hpp>
#include <TaskManagement.hpp>

volatile unsigned short PITFrequency;
unsigned long TickCount = 0;

void PIT::MainInterruptHandler(void) {
    const unsigned char Spinner[4] = {'-' , '\\' , '|' , '/'};
    unsigned char *VideoMemory = (unsigned char *)0xB8000;
    VideoMemory[79*2] = Spinner[TickCount%4];
    TickCount += 1;
    MemoryManagement::CheckNodeCorruption();
    LocalAPIC::SendEOI();
}

unsigned long PIT::GetTickCount(void) {
    return TickCount;
}

void PIT::Initialize(void) {
    PITFrequency = PIT_CONVERT_US_TO_HZ(200);          // Do NOT set this value to 100, it might crash
                                                        // the timer system!
    
    IO::Write(PIT_MODE_COMMAND_REGISTER , 0b00110000);
    IO::Write(PIT_MODE_COMMAND_REGISTER , 0b00110100); // 16bit binary mode , Rate generator , Channel 0
    IO::Write(PIT_CHANNEL0_DATA , PITFrequency & 0xFF);
    IO::Write(PIT_CHANNEL0_DATA , (PITFrequency >> 8) & 0xFF);
    
    PIC::Unmask(32);                                   // Unmask timer interrupt(32)
}

unsigned short PIT::GetCurrentPITFrequency(void) {
    return PITFrequency;
}

unsigned short PIT::ReadPITCounter(unsigned char CounterNumber) {
    IO::Write(PIT_MODE_COMMAND_REGISTER , (CounterNumber & 0b11) << 6);
    return IO::Read(PIT_CHANNEL0_DATA+(CounterNumber & 0b11))|(IO::Read(PIT_CHANNEL0_DATA+(CounterNumber & 0b11)) << 8);
}

void PIT::DelayByPITCount(unsigned int PITCount) {
    unsigned short CounterValue = ReadPITCounter(PIT_COUNTER0);
    while(1) {
        if((CounterValue-ReadPITCounter(PIT_COUNTER0)) >= PITCount) {
            break;
        }
    }
}

void PIT::DelayMilliseconds(unsigned int Milliseconds) {
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

void PIT::DelayMicroseconds(unsigned int Microseconds) {
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
#include <Mouse.hpp>

static Kernel::Mouse::DataManager *MouseDataManager; // Management structure of the mouse data

void Kernel::Mouse::Initialize(void) {
    unsigned int ID;
    unsigned char Output;
    MouseDataManager = (Kernel::Mouse::DataManager *)Kernel::SystemStructure::Allocate(sizeof(Kernel::Mouse::DataManager) , &(ID));    // Allocate the system structure
    MouseDataManager->Initialize();/*
    IO::Write(0x64 , 0xA8);
    IO::Write(0x60 , 0xD4);
    PIT::DelayMilliseconds(10);
    IO::Write(0x60 , 0xF4);

    IO::Write(0x64 , 0x20);
    while(1) {
        if((IO::Read(0x64) & 0b10)) {
            break;
        }
    }
    Output = IO::Read(0x60)|0x02;
    IO::Write(0x64 , 0x60);
    PIT::DelayMilliseconds(10);
    IO::Write(0x60 , Output);
    PIC::Unmask(44);*/
}

void Kernel::Mouse::MainInterruptHandler(void) {
    IO::Read(0x60);
    static int i = 0;
    const unsigned char Spinner[4] = {'-' , '\\' , '|' , '/'};  // Spinner(for debugging purpose)
    unsigned char *VideoMemory = (unsigned char *)0xB8000;
    VideoMemory[77*2] = Spinner[i];
    i++;
    if(i >= 4) {
        i = 0;
    }
    PIC::SendEOI(44);
}

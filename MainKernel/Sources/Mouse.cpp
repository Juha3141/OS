#include <Mouse.hpp>

static Kernel::Mouse::DataManager *MouseDataManager; // Management structure of the mouse data

void Kernel::Mouse::Initialize(void) {
    unsigned int ID;
    unsigned char Output;
    MouseDataManager = (Kernel::Mouse::DataManager *)Kernel::SystemStructure::Allocate(sizeof(Kernel::Mouse::DataManager) , &(ID));    // Allocate the system structure
    MouseDataManager->Initialize();

    IO::Write(0x64 , 0xA8);             // Send Mouse Enable command to command port
    IO::Write(0x64 , 0xD4);             // Enable data transfer
    while(1) {                          // Wait until input buffer is empty
        if(!(IO::Read(0x64) & 0b10)) {
            break;
        }
    }
    IO::Write(0x60 , 0xF4);             // Enable Mouse Enable command to data port
    Kernel::printf("ACK Signal : 0x%X\n" , IO::Read(0x60));

    IO::Write(0x64 , 0x20);             // Read command port data
    while(1) {
        if(IO::Read(0x64) & 0b01) {     // Wait until output buffer is full
            break;
        }
    }
    Output = IO::Read(0x60)|0x02;       // Set Mouse Interrupt bit(0b10) to 1
    IO::Write(0x64 , 0x60);             // Send command port data
    while(1) {                          // Wait until input buffer is empty
        if(!(IO::Read(0x64) & 0b10)) {
            break;
        }
    }
    IO::Write(0x60 , Output);           // Write command port data
}

void Kernel::Mouse::MainInterruptHandler(void) {
    MouseDataManager->ProcessMouseData(IO::Read(0x60));
    PIC::SendEOI(44);
}

void Kernel::Mouse::DataManager::ProcessMouseData(unsigned char Data) {
    if(DataPhase == 0) {
        DataPhase++;
        TemproryMouseData.ButtonData = Data;
    }
    else if(DataPhase == 1) {
        DataPhase++;
        TemproryMouseData.RelativeX = Data;
    }
    else if(DataPhase == 2) {
        DataPhase++;
        TemproryMouseData.RelativeY = Data;
    }
    else {
        DataPhase = 0;
        Kernel::printf("(%d %d 0x%02X)\n" , TemproryMouseData.RelativeX , TemproryMouseData.RelativeY , TemproryMouseData.ButtonData);
        MouseDataQueue.Enqueue(TemproryMouseData);
    }
}
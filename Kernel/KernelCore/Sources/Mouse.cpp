#include <Mouse.hpp>

static Mouse::DataManager *MouseDataManager; // Management structure of the mouse data

void Mouse::Initialize(void) {
    unsigned int ID;
    unsigned char Output;
    MouseDataManager = (Mouse::DataManager *)SystemStructure::Allocate(sizeof(Mouse::DataManager));    // Allocate the system structure
    MouseDataManager->Initialize();/*

    IO::Write(0x64 , 0xA8);             // Send Mouse Enable command to command port
    IO::Write(0x64 , 0xD4);             // Enable data transfer
    while(1) {                          // Wait until input buffer is empty
        if(!(IO::Read(0x64) & 0b10)) {
            break;
        }
    }
    IO::Write(0x60 , 0xF4);             // Enable Mouse Enable command to data port
    printf("ACK Signal : 0x%X\n" , IO::Read(0x60));

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
    
    PIC::Unmask(44);            // Unmask IRQ 12(Mouse Interrupt)
    */
    PIC::Unmask(32+2);          // Unmask IRQ 2(Slave PIC)
    PIC::Unmask(32+9);          // Unmask IRQ 9(Slave PIC)
}

void Mouse::MainInterruptHandler(void) {
    static int i = 0;
    const unsigned char Spinner[4] = {'-' , '\\' , '|' , '/'};  // Spinner(for debugging purpose)
    unsigned char *VideoMemory = (unsigned char *)0xB8000;
    MouseDataManager->ProcessMouseData(IO::Read(0x60));
    VideoMemory[77*2] = Spinner[i];     // Just a basic spinner in the screen
    i++;                                // If the interrupt is called, the spinner spins
    if(i >= 4) {
        i = 0;
    }
    PIC::SendEOI(44);
}

void Mouse::DataManager::ProcessMouseData(unsigned char Data) {
    if(DataPhase == 0) {
        DataPhase++;
        TemporaryMouseData.ButtonData = Data;
    }
    else if(DataPhase == 1) {
        DataPhase++;
        TemporaryMouseData.RelativeX = Data;
    }
    else if(DataPhase == 2) {
        DataPhase++;
        TemporaryMouseData.RelativeY = Data;
    }
    else {
        DataPhase = 0;
    }
    if(DataPhase >= 3) {
        if((TemporaryMouseData.ButtonData & 0x10) == 0x10) {
            TemporaryMouseData.RelativeX |= 0xFFFFFF00;
        }
        if((TemporaryMouseData.ButtonData & 0x20) == 0x20) {
            TemporaryMouseData.RelativeY |= 0xFFFFFF00;
        }
        TemporaryMouseData.RelativeY = -TemporaryMouseData.RelativeY;
        MouseDataQueue.Enqueue(TemporaryMouseData);
        DataPhase = 0;
    }
}

bool Mouse::IsDataQueueEmpty(void) {
    return MouseDataManager->MouseDataQueue.IsEmpty();
}

bool Mouse::GetMouseDataQueue(struct Mouse::MouseData *Data) {
    return (struct Mouse::MouseData *)MouseDataManager->MouseDataQueue.Dequeue(Data);
}
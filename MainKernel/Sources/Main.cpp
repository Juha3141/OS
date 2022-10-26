#include <EssentialLibrary.hpp>
#include <Kernel.hpp>

extern "C" void Main(void) {/*
    int i;
    long double X = -7;
    long double Y = -7;
    long double Value;
    int ScreenX = 0;
    int ScreenY = 0; // (((x)^2+(y)^2-36))^3+(-1)(x)^2(y)^3=0
    unsigned char *VideoMemory = (unsigned char *)0xA0000;

    for(Y = 7; Y > -7; Y -= 0.1) {
        for(X = -7; X < 7; X += 0.1) {
            Value = (((double)(X*X)+(double)(Y*Y))-36);
            Value = ((double)(Value*Value*Value)-((double)(X*X*Y*Y*Y)));
            if((Value <= 40) && (Value >= -40)) {
                VideoMemory[ScreenY*320+ScreenX] = 0x0F;
            }
            ScreenX += 1;
        }
        ScreenX = 0;
        ScreenY += 1;
    }
    while(1) {
        ;
    }*/
    __asm__ ("cli");
    Kernel::SystemStructure::Initialize();
    Kernel::TextScreen80x25::Initialize();
    Kernel::ClearScreen(0x00 , 0x07);
    Kernel::DescriptorTables::Initialize();
    Kernel::MemoryManagement::Initialize();
    Kernel::PIT::Initialize();
    /*
    if(Kernel::ACPI::SaveCoresInformation() == 0) {
        Kernel::printf("Using MP Configurating Table\n");
    }
    
    Kernel::LocalAPIC::EnableLocalAPIC();
    Kernel::LocalAPIC::ActiveAPCores();
    */
    IO::Write(0x21 , 0b11111100);
    __asm__ ("sti");

    while(1) {
        ;
    }
}

extern "C" void APStartup(void) {
    /* To do : Seperate AP & BSP , 
     * Create TSS for AP
     * Create Memory Management System
     * Create Synchronization System(Spinlock)
     * Create Task Management System
     * Create IO Redirecting table
     */
    
    
    while(1) {
        
    }
}
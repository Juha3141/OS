#include <EssentialLibrary.hpp>
#include <Kernel.hpp>

#include <Queue.hpp>

#include <Graphics/Graphic.hpp>

extern "C" void Main(void) {
    /*
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
    Kernel::Keyboard::Initialize();
    Kernel::Mouse::Initialize();

    Kernel::printf("Kernel is initialized.\n");

    /*
    if(Kernel::ACPI::SaveCoresInformation() == 0) {
        Kernel::printf("Using MP Configurating Table\n");
    }
    
    Kernel::LocalAPIC::EnableLocalAPIC();
    Kernel::LocalAPIC::ActiveAPCores();   
    */
    Kernel::printf("Enabling Interrupt.\n");
    __asm__ ("sti");
    int i = 0;
    int X;
    int Y;
    unsigned int Color[3] = {0xFF0000 , 0x00FF00 , 0x0000FF};
    struct Kernel::Mouse::MouseData MouseData;
    for(Y = 0; Y < 768; Y++) {
        for(X = 0; X < 1024; X++) {
            Graphics::VBE::DrawPixel(X , Y , 0x444444);
        }
    }
    X = 1024/2;
    Y = 768/2;
    while(1) {
        if(Kernel::Mouse::GetMouseDataQueue(&(MouseData)) == true) {
            if((MouseData.ButtonData & MOUSE_BUTTONLEFT) == MOUSE_BUTTONLEFT) {
                i = 0;
            }
            else if((MouseData.ButtonData & MOUSE_BUTTONMIDDLE) == MOUSE_BUTTONMIDDLE) {
                i = 1;
            }
            else if((MouseData.ButtonData & MOUSE_BUTTONRIGHT) == MOUSE_BUTTONRIGHT) {
                i = 2;
            }
            X += ((char)MouseData.RelativeX);
            Y += ((char)MouseData.RelativeY);
            Graphics::VBE::DrawPixel(X , Y , Color[i]);
            Graphics::VBE::DrawPixel(X+1 , Y , Color[i]);
            Graphics::VBE::DrawPixel(X , Y+1 , Color[i]);
            Graphics::VBE::DrawPixel(X+1 , Y+1 , Color[i]);
        }
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
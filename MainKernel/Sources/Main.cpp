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
    Kernel::printf("Kernel::TextScreen80x25::Initialize();\n");
    Kernel::DescriptorTables::Initialize();
    Kernel::printf("Kernel::DescriptorTables::Initialize();\n");
    Kernel::MemoryManagement::Initialize();
    Kernel::printf("Kernel::MemoryManagement::Initialize();\n");
    Kernel::PIT::Initialize();
    Kernel::printf("Kernel::PIT::Initialize();\n");
    Kernel::Keyboard::Initialize();
    Kernel::printf("Kernel::Keyboard::Initialize();\n");
    Kernel::Mouse::Initialize();
    Kernel::printf("Kernel::Mouse::Initialize();\n");

    Graphics::Initialize();

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
    Graphics::Layer Layer1;
    Graphics::Layer Layer2;
    Layer1.Initialize(0 , 0 , 1024 , 768);
    Layer2.Initialize(512-36 , 334-36 , 72 , 16);
    Layer1.DrawRectangle(0 , 0 , 1024/2 , 768/2 , 0xFF0000);
    Layer1.DrawRectangle(1024/2 , 0 , 1024 , 768/2 , 0x00FF00);
    Layer1.DrawRectangle(0 , 768/2 , 1024/2 , 768 , 0x0000FF);
    Layer1.DrawRectangle(1024/2 , 768/2 , 1024 , 768 , 0xFFFFFF);
    Layer2.DrawRectangle(0 , 0 , 72 , 16 , 0xFFFFFF);
    Layer2.DrawText(0 , 0 , 0x00 , "DEEZ NUTS");  // Add redrawing system

    Layer1.Register();
    Layer2.Register();
    X = 512-4;
    Y = 334-4;

    Graphics::UpdateLayer(&(Layer1));
    while(1) {
        if(Kernel::Mouse::GetMouseDataQueue(&(MouseData)) == true) {
            X += MouseData.RelativeX;
            Y += MouseData.RelativeY;
            Layer2.Move(X , Y);
        }
    }
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
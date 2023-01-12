#include <EssentialLibrary.hpp>
#include <Kernel.hpp>
#include <Queue.hpp>
#include <TaskManagement.hpp>
#include <ResourceAccessManagement.hpp>

#include <Graphics/Graphic.hpp>

void GraphicModeDemo(void);
void HelloWorld(void);
void PrivilagedOnes(void);

int ActivatedAPCount = 0;
Kernel::SpinLock::Resource SpinLock1;
Kernel::SpinLock::Resource SpinLock2;

extern "C" void Main(void) {
    __asm__ ("cli");
    Kernel::SystemStructure::Initialize();
    Kernel::TextScreen80x25::Initialize();
    Kernel::DescriptorTables::Initialize();
    Kernel::MemoryManagement::Initialize();
    //Kernel::PIT::Initialize();
    Kernel::Keyboard::Initialize();
    Kernel::Mouse::Initialize();
    
    Kernel::TaskManagement::Initialize();
    if(Kernel::ACPI::SaveCoresInformation() == false) {
        Kernel::printf("Using MP Floating Table\n");
        if(Kernel::MPFloatingTable::SaveCoresInformation() == false) {
            Kernel::printf("Failed gathering core information\n");
            __asm__ ("cli");
            while(1) {
                __asm__ ("hlt");
            }
        }
    }
    /*
    
    SpinLock1.Initialize();
    SpinLock2.Initialize();
    Kernel::LocalAPIC::ActiveAPCores();
    
    //Graphics::Initialize();
    */
    Kernel::LocalAPIC::Timer::Initialize();
    Kernel::printf("Enabling Interrupt.\n");
    Kernel::printf("Kernel is initialized.\n");
    __asm__ ("sti");
    
    int i = 0;
    for(i = 0; i < 80; i++) {
        Kernel::TaskManagement::CreateTask((unsigned long)HelloWorld , TASK_FLAGS_WORKING|TASK_FLAGS_PRIVILAGE_KERNEL , 9 , 1024 , "Test1");
    }
    for(i = 0; i < 80; i++) {
        Kernel::TaskManagement::CreateTask((unsigned long)HelloWorld , TASK_FLAGS_WORKING|TASK_FLAGS_PRIVILAGE_KERNEL , 8 , 1024 , "Test1");
    }
    for(i = 0; i < 80; i++) {
        Kernel::TaskManagement::CreateTask((unsigned long)HelloWorld , TASK_FLAGS_WORKING|TASK_FLAGS_PRIVILAGE_KERNEL , 7 , 1024 , "Test1");
    }
    for(i = 0; i < 80; i++) {
        Kernel::TaskManagement::CreateTask((unsigned long)HelloWorld , TASK_FLAGS_WORKING|TASK_FLAGS_PRIVILAGE_KERNEL , 6 , 1024 , "Test1");
    }
    for(i = 0; i < 80; i++) {
        Kernel::TaskManagement::CreateTask((unsigned long)HelloWorld , TASK_FLAGS_WORKING|TASK_FLAGS_PRIVILAGE_KERNEL , 5 , 1024 , "Test1");
    }
    for(i = 0; i < 80; i++) {
        Kernel::TaskManagement::CreateTask((unsigned long)HelloWorld , TASK_FLAGS_WORKING|TASK_FLAGS_PRIVILAGE_KERNEL , 4 , 1024 , "Test1");
    }
    for(i = 0; i < 80; i++) {
        Kernel::TaskManagement::CreateTask((unsigned long)HelloWorld , TASK_FLAGS_WORKING|TASK_FLAGS_PRIVILAGE_KERNEL , 3 , 8*1024 , "Test1");
    }
    for(i = 0; i < 80; i++) {
        Kernel::TaskManagement::CreateTask((unsigned long)HelloWorld , TASK_FLAGS_WORKING|TASK_FLAGS_PRIVILAGE_KERNEL , 2 , 8*1024 , "Test1");
    }
    for(i = 0; i < 80; i++) {
        Kernel::TaskManagement::CreateTask((unsigned long)HelloWorld , TASK_FLAGS_WORKING|TASK_FLAGS_PRIVILAGE_KERNEL , 1 , 1024 , "Test1");
    }
    for(i = 0; i < 80; i++) {
        Kernel::TaskManagement::CreateTask((unsigned long)PrivilagedOnes , TASK_FLAGS_WORKING|TASK_FLAGS_PRIVILAGE_KERNEL , 0 , 8*1024 , "Privilaged Ones");
    }
    unsigned char *VideoMemory = (unsigned char *)TEXTSCREEN_80x25_VIDEOMEMORY;
    unsigned char Spinner[4] = {'-' , '\\' , '|' , '/'};
    while(1) {
        VideoMemory[80*11*2] = Spinner[i++];
        VideoMemory[80*11*2+1] = 0x0E;
        if(i >= 4) {
            i = 0;
        }
    }
}

void HelloWorld(void) {
    unsigned char *VideoMemory = (unsigned char *)TEXTSCREEN_80x25_VIDEOMEMORY;
    int i = 0;
    while(1) {
        VideoMemory[(80+(Kernel::TaskManagement::GetCurrentlyRunningTaskID()-1))*2] = ((i++)%10)+'0';
        VideoMemory[(80+(Kernel::TaskManagement::GetCurrentlyRunningTaskID()-1))*2+1] = ((Kernel::TaskManagement::GetCurrentlyRunningTaskID())%0x0E)+1;
    }
}

void PrivilagedOnes(void) {
    unsigned char *VideoMemory = (unsigned char *)TEXTSCREEN_80x25_VIDEOMEMORY;
    int i = 0;
    while(1) {
        VideoMemory[(80+(Kernel::TaskManagement::GetCurrentlyRunningTaskID()-1))*2] = ((i++)%10)+'0';
        VideoMemory[(80+(Kernel::TaskManagement::GetCurrentlyRunningTaskID()-1))*2+1] = 0x0F;
    }
}

extern "C" void APStartup(void) {
    /* To do : Seperate AP & BSP , 
     * Create TSS for AP
     * Create Memory Management System
     * Create IO Redirecting table
     */
    while(1) {
        __asm__ ("hlt");
    }
}

void GraphicModeDemo(void) {
    int i = 0;
    int L2X;
    int L2Y;
    int L3X;
    int L3Y;
    int L4X;
    int L4Y;
    unsigned int Color[3] = {0xFF0000 , 0x00FF00 , 0x0000FF};
    struct Kernel::Mouse::MouseData MouseData;
    Graphics::Layer Layer1;
    Graphics::Layer Layer2;
    Graphics::Layer Layer3;
    Graphics::Layer Layer4;
    Layer1.Initialize(0 , 0 , 1024 , 768);
    Layer2.Initialize(512-36 , 334-36 , 72 , 16);
    Layer3.Initialize(300 , 300 , 300 , 200);
    Layer4.Initialize(400 , 400 , 350 , 350);
    Layer1.DrawRectangle(0 , 0 , 1024/2 , 768/2 , 0xFF0000);
    Layer1.DrawRectangle(1024/2 , 0 , 1024 , 768/2 , 0x00FF00);
    Layer1.DrawRectangle(0 , 768/2 , 1024/2 , 768 , 0x0000FF);
    Layer1.DrawRectangle(1024/2 , 768/2 , 1024 , 768 , 0xFFFFFF);

    Layer2.DrawRectangle(0 , 0 , 36 , 16 , 0xB2CCFF);
    Layer2.DrawRectangle(36 , 0 , 72 , 16 , 0xB2EBF4);

    Layer3.DrawRectangle(0 , 0 , 300 , 200 , 0x000000);
    Layer3.DrawRectangle(5 , 5 , 300-5 , 200-5 , 0x222222);
    
    Layer4.DrawRectangle(0 , 0 , 350 , 350 , 0x000000);
    Layer4.DrawRectangle(5 , 5 , 350-5 , 350-5 , 0xFFFFFF);


    Layer1.Register();
    Layer2.Register();
    Layer3.Register();
    Layer4.Register();
    
    L2X = 512-4;
    L2Y = 334-4;
    L3X = 300;
    L3Y = 300;
    L4X = 400;
    L4Y = 400;
    Graphics::UpdateLayer(Layer1);
    while(1) {
        if(Kernel::Mouse::GetMouseDataQueue(&(MouseData)) == true) {
            if((MouseData.ButtonData & MOUSE_BUTTONLEFT) == MOUSE_BUTTONLEFT) {
                L2X += MouseData.RelativeX;
                L2Y += MouseData.RelativeY;
                Layer2.Move(L2X , L2Y);
            }
            if((MouseData.ButtonData & MOUSE_BUTTONRIGHT) == MOUSE_BUTTONRIGHT) {
                L3X += MouseData.RelativeX;
                L3Y += MouseData.RelativeY;
                Layer3.Move(L3X , L3Y);
            }
            if((MouseData.ButtonData & MOUSE_BUTTONMIDDLE) == MOUSE_BUTTONMIDDLE) {
                L4X += MouseData.RelativeX;
                L4Y += MouseData.RelativeY;
                Layer4.Move(L4X , L4Y);
            }
            i++;
        }
    }
}
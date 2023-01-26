#include <EssentialLibrary.hpp>
#include <Kernel.hpp>
#include <Queue.hpp>
#include <TaskManagement.hpp>
#include <ResourceAccessManagement.hpp>
#include <Paging.hpp>

extern "C" void Main(void) {
    __asm__ ("cli");
    Kernel::SystemStructure::Initialize();
    Kernel::TextScreen80x25::Initialize();
    Kernel::DescriptorTables::Initialize();
    Kernel::MemoryManagement::Initialize();
    //Kernel::PIT::Initialize();
    Kernel::Keyboard::Initialize();
    Kernel::Mouse::Initialize();

    if(Kernel::ACPI::Initialize() == false) {
        Kernel::printf("Failed gathering information from ACPI\n");
    }
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
    Kernel::TaskManagement::Initialize();
    /*
    Kernel::LocalAPIC::ActiveAPCores();
    */
    Kernel::LocalAPIC::Timer::Initialize();
    Kernel::printf("Enabling Interrupt.\n");
    Kernel::printf("Kernel is initialized.\n");
    __asm__ ("sti");
    while(1) {
        ;
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
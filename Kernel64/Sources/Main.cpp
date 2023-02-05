#include <EssentialLibrary.hpp>
#include <Kernel.hpp>
#include <Queue.hpp>
#include <TaskManagement.hpp>
#include <ResourceAccessManagement.hpp>
#include <Paging.hpp>
#include <Drivers/DeviceDriver.hpp>
#include <Drivers/BootRAMDisk.hpp>

#include <FileSystem/ISO9660.hpp>
#include <FileSystem/VirtualFileSystem.hpp>

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

    Kernel::FileSystem::Initialize();
    
    Kernel::FileSystem::ISO9660::Register();

    Kernel::Drivers::StorageSystem::Initialize();
    Kernel::Drivers::StorageSystem::Standard *BootRAMDiskDriver
     = Kernel::Drivers::StorageSystem::AssignSystem(
        Kernel::Drivers::BootRAMDisk::PostInitialization , 
        Kernel::Drivers::BootRAMDisk::ReadSector , 
        Kernel::Drivers::BootRAMDisk::WriteSector , 
        Kernel::Drivers::BootRAMDisk::GetGeometry);
    Kernel::Drivers::StorageSystem::Register(BootRAMDiskDriver , "BootRAMDisk0");
    
    char Buffer[6];
    Kernel::FileSystem::FileInfo *FileInfo
     = Kernel::FileSystem::OpenFile("::BootRAMDisk0/HELLO.TXT" , "hello");
    if(FileInfo == 0x00) {
        Kernel::printf("File not found.\n");
        while(1) {
            ;
        }
    }
    Kernel::FileSystem::ReadFile(FileInfo , 5 , Buffer);
    Kernel::printf("%s\n" , Buffer);
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
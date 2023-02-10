#include <EssentialLibrary.hpp>
#include <Kernel.hpp>
#include <Queue.hpp>
#include <TaskManagement.hpp>
#include <ResourceAccessManagement.hpp>
#include <Paging.hpp>

#include <Drivers/DeviceDriver.hpp>
#include <Drivers/BootRAMDisk.hpp>
#include <Drivers/PATA.hpp>
#include <Drivers/PATA_CD.hpp>

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
    Kernel::Drivers::StorageSystem::Initialize();

    Kernel::FileSystem::ISO9660::Register();

    Kernel::Drivers::BootRAMDisk::Register();
    Kernel::Drivers::PATA::Register();
    
    char Buffer[6];
    Kernel::FileSystem::FileInfo *FileInfo
     = Kernel::FileSystem::OpenFile("::ramdisk0::0/HELLO.TXT" , "hello");
    if(FileInfo == 0x00) {
        Kernel::printf("File not found.\n");
        while(1) {
            ;
        }
    }
    Kernel::FileSystem::ReadFile(FileInfo , 6 , Buffer);
    Kernel::printf("%s" , Buffer);
    Kernel::FileSystem::ReadFile(FileInfo , 5 , Buffer);
    Kernel::printf("%s\n" , Buffer);
    Kernel::FileSystem::FileInfo *FileInfo2
     = Kernel::FileSystem::OpenFile("::idehd0::0/HELLO.TXT" , "");
    Kernel::FileSystem::FileInfo *FileInfo3
     = Kernel::FileSystem::OpenFile("::idecd0::0/TEST.TXT" , "");
    /*
    int i;
    unsigned short Base = ATA_PRIMARY_BASE;
    unsigned short Control = ATA_DEVICECONTROL_PRIMARY_BASE;
    for(i = 0; i < 2; i++) {
        Kernel::printf("Current port : 0x%X , 0x%X\n" , Base , Control);
        if(Kernel::Drivers::ATA::ReadInformation(Base , Control , true) == false) {
            Kernel::printf("Failed at 0x%X , 0x%X\n" , Base , Control);
            break;
        }
        if(Kernel::Drivers::ATA::ReadInformation(Base , Control , false) == false) {
            Kernel::printf("Failed at 0x%X , 0x%X\n" , Base , Control);
            break;
        }
        Base = ATA_SECONDARY_BASE;
        Control = ATA_DEVICECONTROL_SECONDARY_BASE;
    }*/
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
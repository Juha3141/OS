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
#include <Drivers/PCI.hpp>

#include <FileSystem/ISO9660.hpp>
#include <FileSystem/VirtualFileSystem.hpp>

using namespace Kernel;
using namespace Drivers;

extern "C" void Main(void) {
    __asm__ ("cli");
    SystemStructure::Initialize(); // good
    TextScreen80x25::Initialize(); // good
    DescriptorTables::Initialize(); // error?
    MemoryManagement::Initialize();
    // PIT::Initialize();
    Keyboard::Initialize();
    Mouse::Initialize();
    if(ACPI::Initialize() == false) {
        printf("Failed gathering information from ACPI\n");
    }

    if(ACPI::SaveCoresInformation() == false) {
        printf("Using MP Floating Table\n");
        if(MPFloatingTable::SaveCoresInformation() == false) {
            printf("Failed gathering core information\n");
            __asm__ ("cli");
            while(1) {
                __asm__ ("hlt");
            }
        }
    }
    Kernel::printf("APCI::SaveCoresInformation();\n");
    TaskManagement::Initialize();
    Kernel::printf("TaskManagement::Initialize();\n");
    /*
    LocalAPIC::ActiveAPCores();
    */
    LocalAPIC::Timer::Initialize();
    Kernel::printf("LocalAPIC::Timer::Initialize();\n");
    printf("Enabling Interrupt.\n");
    printf("Kernel is initialized.\n");
    __asm__ ("sti");
    
    /*unsigned int i;
    unsigned char *Data = (unsigned char *)MEMORYMANAGEMENT_MEMORY_STARTADDRESS;
    Kernel::printf("0x%X : " , Data);
    int j;
    for(i = 0; i < 0x100000; i++) {
        if(((i%16) == 0x00) && (i != 0x00)) {
            if((j%16) == 0x00) {
                Kernel::Keyboard::GetASCIIData();
            }
            Kernel::printf("\n");
            Kernel::printf("0x%X : " , &(Data[i]));
            j++;
        }
        if(Data[i] < 0x10) {
            Kernel::printf("0");
        }
        Kernel::printf("%X " , (unsigned char)Data[i]);
    }*/

    /*
    unsigned int i;
    unsigned long *Data = (unsigned long *)MEMORYMANAGEMENT_MEMORY_STARTADDRESS;
    unsigned long Previous;
    Kernel::printf("0x%X : \n" , Data);
    __asm__ ("cli");
    for(i = 0; i < 0x100000; i++) {
        Previous = Data[i];
        Data[i] = 0xCAFEBABEDEADBADA;
        PIT::DelayMicroseconds(50);
        if(Data[i] != 0xCAFEBABEDEADBADA) {
            Kernel::printf("0x%X : 0x%X != 0x%X\n" , 0xCAFEBABEDEADBADA , Data[i]);
        }
        Data[i] = Previous;
        if((unsigned long)&(Data[i])%0x1000 == 0) {
            Kernel::printf("0x%X pass\n" , &(Data[i]));
        }
    }
    Kernel::printf("Manual probing done.\n");
    __asm__ ("sti");
    */

    FileSystem::Initialize();
    Kernel::printf("File System Initialized\n");
    Drivers::StorageSystem::Initialize();
    Kernel::printf("Storage System Initialized\n");
    FileSystem::ISO9660::Register();
    Kernel::printf("ISO9660 File system registered\n");
    Drivers::BootRAMDisk::Register();
    Kernel::printf("BootRAMDisk registered\n");
    Drivers::PATA::Register();
    Kernel::printf("Detecting PCI devices\n");
    Drivers::PCI::Detect();
    Kernel::printf("Done\n");
    /*
    char Buffer[6];
    FileSystem::FileInfo *FileInfo
     = FileSystem::OpenFile("::ramdisk0::0/HELLO.TXT" , "hello");
    if(FileInfo == 0x00) {
        printf("File not found.\n");
        while(1) {
            ;
        }
    }
    FileSystem::ReadFile(FileInfo , 6 , Buffer);
    printf("%s" , Buffer);
    FileSystem::ReadFile(FileInfo , 5 , Buffer);
    printf("%s\n" , Buffer);
    FileSystem::FileInfo *FileInfo2
     = FileSystem::OpenFile("::idehd0::0/HELLO.TXT" , "");
    FileSystem::FileInfo *FileInfo3
     = FileSystem::OpenFile("::idecd0::0/TEST.TXT" , "");*/

    /*
    int i;
    unsigned short Base = ATA_PRIMARY_BASE;
    unsigned short Control = ATA_DEVICECONTROL_PRIMARY_BASE;
    for(i = 0; i < 2; i++) {
        printf("Current port : 0x%X , 0x%X\n" , Base , Control);
        if(Drivers::ATA::ReadInformation(Base , Control , true) == false) {
            printf("Failed at 0x%X , 0x%X\n" , Base , Control);
            break;
        }
        if(Drivers::ATA::ReadInformation(Base , Control , false) == false) {
            printf("Failed at 0x%X , 0x%X\n" , Base , Control);
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
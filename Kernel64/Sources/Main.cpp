#include <EssentialLibrary.hpp>
#include <Kernel.hpp>
#include <Queue.hpp>
#include <TaskManagement.hpp>
#include <ResourceAccessManagement.hpp>
#include <Paging.hpp>

#include <Drivers/DeviceDriver.hpp>
#include <Drivers/RAMDisk.hpp>
#include <Drivers/PATA.hpp>
#include <Drivers/PATA_CD.hpp>
#include <Drivers/PCI.hpp>

#include <FileSystem/ISO9660.hpp>
#include <FileSystem/FAT16.hpp>

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
            // temporary info
            struct CoreInformation *CoreInformation = (struct CoreInformation *)CoreInformation::GetInstance();
            CoreInformation->LocalAPICAddress = 0xFEE00000;
            CoreInformation->IOAPICAddress = 0xFEC00000;
            CoreInformation->IOAPICID = 0x00;
            CoreInformation->MPUsed = false;
            CoreInformation->CoreCount = 1;
            Kernel::printf("Using Standard Addresses\n");
        }
    }
    
    TaskManagement::Initialize();
    
    /*
    LocalAPIC::Timer::Initialize();
    LocalAPIC::EnableLocalAPIC();
    IOAPIC::Initialize();
    LocalAPIC::ActiveAPCores();
    */
    
    Kernel::printf("LocalAPIC Initialized\n");
    __asm__ ("sti");
    Kernel:printf("Enabled interrupt\n");
    FileSystem::Initialize();
    Kernel::printf("File System Initialized\n");
    Drivers::StorageSystem::Initialize();
    Kernel::printf("Storage System Initialized\n");
    FileSystem::ISO9660::Register();
    FileSystem::FAT16::Register();

    Drivers::PATA::Register();
    Drivers::RAMDisk::Register();
    Kernel::printf("Detecting PCI devices\n");
    Drivers::PCI::Detect();
    Kernel::printf("Done\n");

    unsigned char *Buffer = (unsigned char *)Kernel::MemoryManagement::Allocate(40960*512);
    StorageSystem::Storage *ISORAMDisk = Drivers::RAMDisk::CreateRAMDisk(8192 , 2048 , 0x720000);
    Kernel::printf("Registering RAM disk (16MB)\n");
    FileSystem::FileInfo *VBRFile = FileSystem::ISO9660::OpenFile(ISORAMDisk , "RDIMG.IMG");
    if(VBRFile == 0x00) {
        Kernel::printf("File not found.\n");
        while(1) {
            ;
        }
    }
    FileSystem::ISO9660::ReadFile(ISORAMDisk , VBRFile , 16*1024*1024 , Buffer);
    StorageSystem::Storage *SysRAMDisk = Drivers::RAMDisk::CreateRAMDisk(40960 , 512 , (unsigned long)Buffer); // 8MBs of System RAM Disk, Formatted by FAT12
    
    Kernel::printf("rdimg.img Buffer : 0x%X\n" , Buffer);
    struct FileSystem::FAT16::VBR VBR;
    unsigned int TotalSector;
    unsigned int RootDirectorySector;
    FileSystem::FAT16::GetVBR(SysRAMDisk , &(VBR));
    Kernel::printf("Done\n");
    /*
    Kernel::printf("FAT Area       : %d\n" , FileSystem::FAT16::GetFATAreaLocation(&(VBR)));
    Kernel::printf("Root Directory : %d\n" , FileSystem::FAT16::GetRootDirectoryLocation(&(VBR)));
    Kernel::printf("Sectors per cluster : %d\n" , VBR.SectorsPerCluster);
    TotalSector = (VBR.TotalSector16 == 0) ? VBR.TotalSector32 : VBR.TotalSector16;
    Kernel::printf("TotalSector    : %d\n" , TotalSector);
    RootDirectorySector = ((VBR.RootDirectoryEntryCount*32)+(VBR.BytesPerSector-1))/VBR.BytesPerSector;
    Kernel::printf("Root Directory Size : %d\n" , RootDirectorySector);
    Kernel::printf("FAT Size       : %d\n" ,((TotalSector-(VBR.ReservedSectorCount+RootDirectorySector))+((256*VBR.SectorsPerCluster)+VBR.NumberOfFAT-1))/(VBR.NumberOfFAT+(256*VBR.SectorsPerCluster)));
    */
    FileSystem::FileInfo *FileInfo = FileSystem::FAT16::OpenFile(SysRAMDisk , "Hello.txt");
    if(FileInfo == 0x00) {
        Kernel::printf("File \"Hello.txt\" not found.\n");
    }
    FileSystem::FileInfo *FileInfo2 = FileSystem::FAT16::OpenFile(SysRAMDisk , "Hello.txt");
    if(FileInfo2 == 0x00) {
        Kernel::printf("File \"Testing/Hello.txt\" not found.\n");
    }
    char Data[FileInfo2->FileSize+1] = {0 , };
    FileSystem::FAT16::ReadFile(SysRAMDisk , FileInfo2 , FileInfo2->FileSize , Data);
    Kernel::printf("Data : \n");
    Kernel::printf("%s\n" , Data);
    Kernel::printf("===================================\n");

    // To-do : Create interface
    while(1) {
        ;
    }
}

extern "C" void APStartup(void) {
    /* To do : Seperate AP & BSP , 
     * Create TSS for AP
     * Create Memory Management System
     * Create IO Redirection table
     */
    unsigned long i = 0;
    unsigned long CoreID = LocalAPIC::GetCurrentAPICID();
    while(1) {
        __asm__ ("hlt");
    }
}
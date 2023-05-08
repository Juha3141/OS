#include <EssentialLibrary.hpp>
#include <Kernel.hpp>
#include <Queue.hpp>
#include <TaskManagement.hpp>
#include <ResourceAccessManagement.hpp>
#include <Paging.hpp>
#include <MutualExclusion.hpp>

#include <Drivers/DeviceDriver.hpp>
#include <Drivers/RAMDisk.hpp>
#include <Drivers/PATA.hpp>
#include <Drivers/PATA_CD.hpp>
#include <Drivers/PCI.hpp>

#include <FileSystem/MBR.hpp>
#include <FileSystem/GPT.hpp>

#include <FileSystem/ISO9660.hpp>
#include <FileSystem/FAT16.hpp>

using namespace Kernel;
using namespace Drivers;

unsigned long KernelStackBase = 0;
unsigned long KernelStackSize = 2*1024*1024;

// problem : Kernel stack is too small & overrides kernel
// One core per 2MB *** 

void AutoFormat_FAT16(struct StorageSystem::Storage *Storage);

extern "C" void Main(void) {
    __asm__ ("cli");
    SystemStructure::Initialize(); // good
    TextScreen80x25::Initialize(); // good
    MemoryManagement::Initialize();
    DescriptorTables::Initialize(); // error?
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
    LocalAPIC::Timer::Initialize();/*
    LocalAPIC::GlobalEnableLocalAPIC();
    LocalAPIC::EnableLocalAPIC();
    IOAPIC::InitializeRedirectionTable();
    LocalAPIC::ActivateAPCores();*/
    __asm__ ("sti");

    // To-do : Make proper stack system
    KernelStackBase = (unsigned long)Kernel::MemoryManagement::Allocate(KernelStackSize*CoreInformation::GetInstance()->CoreCount);
    KernelStackBase += KernelStackSize*CoreInformation::GetInstance()->CoreCount;

    FileSystem::Initialize();
    Kernel::printf("File System Initialized\n");
    Drivers::StorageSystem::Initialize();
    Kernel::printf("Storage System Initialized\n");

    FileSystem::ISO9660::Register();
    FileSystem::FAT16::Register();

    Drivers::PATA::Register();
    // Drivers::PATA_CD::Register();
    Drivers::RAMDisk::Register();
    Kernel::printf("Detecting PCI devices\n");
    Drivers::PCI::Detect();
    Kernel::printf("Done\n");
    // To-do : Create interface
    Drivers::StorageSystem::Storage *RAMDiskStorage = Drivers::RAMDisk::CreateRAMDisk(65536 , 512 , 0x00);
    Kernel::printf("Created Live RAM Disk (Size : %dMB)\n" , RAMDiskStorage->Geometry.BytesPerSector*RAMDiskStorage->Geometry.TotalSectorCount/1024/1024);
    Kernel::printf("Location : 0x%X\n" , RAMDiskStorage->Resources[0]);
    // AutoFormat_FAT16(Storage);

    // To-do : Create function that creates partition
    AutoFormat_FAT16(RAMDiskStorage);
    while(1) {
        ;
    }
}

void AutoFormat_FAT16(struct StorageSystem::Storage *Storage) {
    struct FileSystem::FAT16::VBR VBR;
    unsigned char ByteCode[140] = {
        0xB8 , 0x00 , 0x00 , 0x8E , 0xD8 , 0xB8 , 0x00 , 0xB8 , 0x8E , 0xC0 , 0x31 , 0xFF , 0x26 , 0xC6 , 0x05 , 0x00 , 
        0x47 , 0x26 , 0xC6 , 0x05 , 0x07 , 0x47 , 0x81 , 0xFF , 0xA0 , 0x0F , 0x72 , 0xF0 , 0x26 , 0x66 , 0xC7 , 0x06 , 
        0x00 , 0x00 , 0x48 , 0x07 , 0x6F , 0x07 , 0x26 , 0x66 , 0xC7 , 0x06 , 0x04 , 0x00 , 0x77 , 0x07 , 0x20 , 0x07 , 
        0x26 , 0x66 , 0xC7 , 0x06 , 0x08 , 0x00 , 0x64 , 0x07 , 0x69 , 0x07 , 0x26 , 0x66 , 0xC7 , 0x06 , 0x0C , 0x00 , 
        0x64 , 0x07 , 0x20 , 0x07 , 0x26 , 0x66 , 0xC7 , 0x06 , 0x10 , 0x00 , 0x79 , 0x07 , 0x6F , 0x07 , 0x26 , 0x66 , 
        0xC7 , 0x06 , 0x14 , 0x00 , 0x75 , 0x07 , 0x20 , 0x07 , 0x26 , 0x66 , 0xC7 , 0x06 , 0x18 , 0x00 , 0x67 , 0x07 , 
        0x65 , 0x07 , 0x26 , 0x66 , 0xC7 , 0x06 , 0x1C , 0x00 , 0x74 , 0x07 , 0x20 , 0x07 , 0x26 , 0x66 , 0xC7 , 0x06 , 
        0x20 , 0x00 , 0x68 , 0x07 , 0x65 , 0x07 , 0x26 , 0x66 , 0xC7 , 0x06 , 0x24 , 0x00 , 0x72 , 0x07 , 0x65 , 0x07 , 
        0x26 , 0x66 , 0xC7 , 0x06 , 0x28 , 0x00 , 0x3F , 0x07 , 0x00 , 0x00 , 0xEB , 0xFE , };
    unsigned char *BootSector = (unsigned char *)Kernel::MemoryManagement::Allocate(512);
    struct FileSystem::FAT16::SFNEntry VolumeLabel;
    struct StorageSystem::Partition Partition;
    FileSystem::FAT16::WriteVBR(&(VBR) , &(Storage->Geometry));
    memcpy(BootSector , &(VBR) , sizeof(struct FileSystem::FAT16::VBR));
    memcpy(BootSector+sizeof(struct FileSystem::FAT16::VBR) , ByteCode , 140);
    BootSector[510] = 0x55;
    BootSector[511] = 0xAA;
    Storage->Driver->WriteSectorFunction(Storage , 0 , 1 , BootSector);
    Kernel::printf("Root Directory Location : %d\n" , FileSystem::FAT16::GetRootDirectoryLocation(&(VBR)));
    Kernel::printf("FAT Area Location       : %d\n" , FileSystem::FAT16::GetFATAreaLocation(&(VBR)));
    memset(&(VolumeLabel) , 0 , sizeof(struct FileSystem::FAT16::SFNEntry));
    memcpy(VolumeLabel.FileName , "NO NAME    " , 11);
    VolumeLabel.Attribute = 0x08;
    
    Partition.StartAddressLBA = 128; // Always starts in this sector
    Partition.EndAddressLBA = Storage->Geometry.TotalSectorCount-Partition.StartAddressLBA;
    Partition.PartitionType = 0x86;
    Partition.IsBootable = 0;
// setup drive    
    MBR::Identifier MBRIdentifier(Storage->Driver , Storage);
    MBRIdentifier.CreatePartition(Partition);
    StorageSystem::AddLogicalDrive(Storage->Driver , Storage , &(Partition) , 1);


    /* <Major Problem in Kernel Core>
     * The Faulty Memory management system(Especially "MemoryManagement::Free") was all the problem.
     * The allocated data overwrote the system structure(In this case, it was IST), and caused triple fault.
     * If we can't find how this happends, which is, how the memory manager failed to manage memory, we might 
     * have to re-write this entire memory management system.
    */

    /* <What we have to do with blank disk>
     * 1. Write MBR
     * 2. Create new partition
     * 3. Format the partition with some file system
    */ 
    
    FileSystem::FAT16::WriteClusterInfo(Storage , 0 , 0xFFF8 , &(VBR));
    FileSystem::FAT16::WriteClusterInfo(Storage , 1 , 0xFFFF , &(VBR));
    FileSystem::FAT16::WriteSFNEntry(Storage , FileSystem::FAT16::GetRootDirectoryLocation(&(VBR)) , &(VolumeLabel));
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
    static MutualExclusion::SpinLock *SpinLock = 0x00;
    if(SpinLock == 0x00) {
        SpinLock = (MutualExclusion::SpinLock *)MemoryManagement::Allocate(sizeof(MutualExclusion::SpinLock));
        SpinLock->Initialize();
    }
    __asm__ ("cli");
    DescriptorTables::Initialize();
    LocalAPIC::EnableLocalAPIC();
    __asm__ ("sti");
    while(1) {
        ;
    }
}

// To-do : More reliable way to detect core
// To-do : Change driver to Polymorphism.
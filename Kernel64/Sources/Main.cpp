#include <EssentialLibrary.hpp>
#include <Kernel.hpp>
#include <Queue.hpp>
#include <TaskManagement.hpp>
#include <ResourceAccessManagement.hpp>
#include <Paging.hpp>
#include <MutualExclusion.hpp>

#include <Drivers/StorageDriver.hpp>
#include <Drivers/FileSystemDriver.hpp>

#include <Drivers/RAMDisk.hpp>
#include <Drivers/IDE.hpp>
#include <Drivers/PCI.hpp>

#include <FileSystem/MBR.hpp>
#include <FileSystem/GPT.hpp>

#include <FileSystem/ISO9660.hpp>
#include <FileSystem/FAT16.hpp>

unsigned long KernelStackBase = 0;
unsigned long KernelStackSize = 2*1024*1024;

// problem : Kernel stack is too small & overrides kernel
// One core per 2MB *** 

// problem 2 : Using traditional malloc-like system crashes when using abstract class
// -> use new/delete keyword.

void InitializeDrive_MBR(struct Storage *Storage);
void CreatePartition_FAT16(struct Storage *Storage , const char *VolumeLabel , unsigned long StartAddress , unsigned long Size);

extern "C" void Main(void) {
    __asm__ ("cli");
    SystemStructure::Initialize(); // good
    TextScreen80x25::Initialize(); // good
    MemoryManagement::Initialize();
    DescriptorTables::Initialize();
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
            printf("Using Standard Addresses\n");
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
    KernelStackBase = (unsigned long)MemoryManagement::Allocate(KernelStackSize*CoreInformation::GetInstance()->CoreCount);
    KernelStackBase += KernelStackSize*CoreInformation::GetInstance()->CoreCount;
    printf("KernelStackBase : 0x%X\n" , KernelStackBase);
    
    FileSystem::Initialize();
    printf("File System Initialized\n");
    StorageSystem::Initialize();
    printf("Storage System Initialized\n");
    ISO9660::Register();
    FAT16::Register();
    printf("File System Registered\n");
    printf("RAMDiskDriver::Register : 0x%X\n" , &(RAMDiskDriver::Register));
    
    RAMDiskDriver::Register();
    IDEDriver::Register();
    PCI::Detect();
    
    struct Storage *RAMDiskStorage = RAMDiskDriver::CreateRAMDisk(65536 , 512 , 0x00);
    printf("Registered RAMDisk Storage , Location : 0x%X\n" , RAMDiskStorage->PhysicalInfo.Resources[0]);

    // To-do : Create interface
    // AutoFormat_FAT16(Storage);
    
    // Let's do some cleaning
    // To-do : Create function that creates partition
    struct Storage *Storage = StorageSystem::SearchStorage("idehd" , 0);
    if(Storage == 0x00) {
        printf("Not found.\n");
        while(1) {
            ;
        }
    }
    InitializeDrive_MBR(Storage);
    CreatePartition_FAT16(Storage , "MAIN" , 128 , (100*1024*1024)/512); // 100MiB
    // There's chance that the error was because the faulty initialization code
    // (especially WriteClusterInfo. It's sus.)
    // LogicalStorage 0
    class FileSystemDriver *FileSystemDriver = Storage->LogicalStorages->GetObject(0)->FileSystem;
    // FileSystemDriver->CreateFile(Storage->LogicalStorages->GetObject(0) , "hello this is testing long file name.txt" , 0);
    
    FileSystemDriver->CreateDir(Storage->LogicalStorages->GetObject(0) , "Test directory");
    FileSystemDriver->CreateFile(Storage->LogicalStorages->GetObject(0) , "Test directory/blank file.txt");
    struct FileInfo *TestingFile = FileSystemDriver->OpenFile(Storage->LogicalStorages->GetObject(0) , "Test directory/blank file.txt" , FILESYSTEM_OPEN_OVERWRITE);
    if(TestingFile == 0x00) {
        printf("File not found.\n");
        while(1) {
            ;
        }
    }
    else {
        printf("Found file!\n");
        printf("Sector address : %d\n" , TestingFile->Location);
    }
    FileSystemDriver->WriteFile(TestingFile , 45 , (void *)"All work and no play makes Jack a dull boy.\r\n");
    printf("Done.\n");
    while(1) {
        ;
    }
}

const unsigned char CommonByteCode[140] = {
    0xB8 , 0x00 , 0x00 , 0x8E , 0xD8 , 0xB8 , 0x00 , 0xB8 , 0x8E , 0xC0 , 0x31 , 0xFF , 0x26 , 0xC6 , 0x05 , 0x00 , 
    0x47 , 0x26 , 0xC6 , 0x05 , 0x07 , 0x47 , 0x81 , 0xFF , 0xA0 , 0x0F , 0x72 , 0xF0 , 0x26 , 0x66 , 0xC7 , 0x06 , 
    0x00 , 0x00 , 0x48 , 0x07 , 0x6F , 0x07 , 0x26 , 0x66 , 0xC7 , 0x06 , 0x04 , 0x00 , 0x77 , 0x07 , 0x20 , 0x07 , 
    0x26 , 0x66 , 0xC7 , 0x06 , 0x08 , 0x00 , 0x64 , 0x07 , 0x69 , 0x07 , 0x26 , 0x66 , 0xC7 , 0x06 , 0x0C , 0x00 , 
    0x64 , 0x07 , 0x20 , 0x07 , 0x26 , 0x66 , 0xC7 , 0x06 , 0x10 , 0x00 , 0x79 , 0x07 , 0x6F , 0x07 , 0x26 , 0x66 , 
    0xC7 , 0x06 , 0x14 , 0x00 , 0x75 , 0x07 , 0x20 , 0x07 , 0x26 , 0x66 , 0xC7 , 0x06 , 0x18 , 0x00 , 0x67 , 0x07 , 
    0x65 , 0x07 , 0x26 , 0x66 , 0xC7 , 0x06 , 0x1C , 0x00 , 0x74 , 0x07 , 0x20 , 0x07 , 0x26 , 0x66 , 0xC7 , 0x06 , 
    0x20 , 0x00 , 0x68 , 0x07 , 0x65 , 0x07 , 0x26 , 0x66 , 0xC7 , 0x06 , 0x24 , 0x00 , 0x72 , 0x07 , 0x65 , 0x07 , 
    0x26 , 0x66 , 0xC7 , 0x06 , 0x28 , 0x00 , 0x3F , 0x07 , 0x00 , 0x00 , 0xEB , 0xFE , 
};

void InitializeDrive_MBR(struct Storage *Storage) {
    struct FAT16::VBR VBR;
    unsigned char *BootSector = (unsigned char *)MemoryManagement::Allocate(512);
    FAT16::WriteVBR(&(VBR) , Storage , "POTATOOS" , "NO NAME   " , "          ");
    memset(BootSector , 0 , 512);
    memcpy(BootSector , &(VBR) , sizeof(struct FAT16::VBR));
    memcpy(BootSector+sizeof(struct FAT16::VBR) , CommonByteCode , 140);
    BootSector[510] = 0x55;
    BootSector[511] = 0xAA;
    Storage->Driver->WriteSector(Storage , 0 , 1 , BootSector);
    // MemoryManagement::Free(BootSector);
}

void CreatePartition_FAT16(struct Storage *Storage , const char *VolumeLabel , unsigned long StartAddress , unsigned long Size) {
    struct Partition Partition;
    struct FAT16::SFNEntry VolumeLabelEntry;
    
    struct FAT16::VBR VBR;
    struct Storage *CurrentPartition;
    char VolumeLabelSFNName[12];
    unsigned char *BootSector = (unsigned char *)MemoryManagement::Allocate(512);

    Partition.StartAddressLBA = StartAddress; // Always starts in this sector
    Partition.EndAddressLBA = StartAddress+Size;
    Partition.PartitionType = 0x86;
    Partition.IsBootable = 0;
    
    MBR::Identifier MBRIdentifier(Storage->Driver , Storage);
    MBRIdentifier.CreatePartition(Partition); // Physically create partition
    StorageSystem::AddLogicalDrive(Storage->Driver , Storage , &(Partition) , 1); // Setup basic information
    
    CurrentPartition = Storage->LogicalStorages->GetObject(Storage->LogicalStorages->CurrentObjectCount-1);

    FAT16::WriteVBR(&(VBR) , CurrentPartition , "POTATOOS" , "NO NAME   " , "FAT16     ");
    memset(BootSector , 0 , 512);
    memcpy(BootSector , &(VBR) , sizeof(struct FAT16::VBR));
    memcpy(BootSector+sizeof(struct FAT16::VBR) , CommonByteCode , 140);
    BootSector[510] = 0x55;
    BootSector[511] = 0xAA;
    CurrentPartition->Driver->WriteSector(CurrentPartition , 0 , 1 , BootSector);
    // MemoryManagement::Free(BootSector);
    
    memset(&(VolumeLabelEntry) , 0 , sizeof(struct FAT16::SFNEntry));
    FAT16::CreateVolumeLabelName(VolumeLabelSFNName , VolumeLabel);
    memcpy(VolumeLabelEntry.FileName , VolumeLabelSFNName , 11);
    VolumeLabelEntry.Attribute = 0x08;
    
    FAT16::WriteClusterInfo(CurrentPartition , 0 , 0xFFF8 , &(VBR)); // this doesn't work
    FAT16::WriteClusterInfo(CurrentPartition , 1 , 0xFFFF , &(VBR));
    FAT16::WriteSFNEntry(CurrentPartition , FAT16::GetRootDirectoryLocation(&(VBR)) , &(VolumeLabelEntry));
    
    CurrentPartition->FileSystem = new FAT16::Driver;
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
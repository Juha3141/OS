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
#include <Drivers/BootRAMDisk.hpp>

#include <FileSystem/MBR.hpp>
#include <FileSystem/GPT.hpp>
#include <FileSystem/MountSystem.hpp>

#include <FileSystem/ISO9660.hpp>
#include <FileSystem/FAT16.hpp>

#include <Shell.hpp>

unsigned long KernelStackBase;
unsigned long KernelStackSize = 2*1024*1024;

// problem : Kernel stack is too small & overrides kernel
// One core per 2MB *** 

void InitializeDrive_MBR(struct Storage *Storage);
void CreatePartition_FAT16(struct Storage *Storage , const char *VolumeLabel , unsigned long StartAddress , unsigned long Size);

extern "C" void Main(void) {
    __asm__ ("cli");
    KernelStackBase = 0x00;
    SystemStructure::Initialize(); // good
    TextScreen80x25::Initialize(); // good
    MemoryManagement::Initialize();
    DescriptorTables::Initialize();
    Keyboard::Initialize(); // ps/2 (temporary integrated keyboard driver.. I guess.)
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
// good
    TaskManagement::Initialize();
    LocalAPIC::Timer::Initialize();
    LocalAPIC::GlobalEnableLocalAPIC();
    LocalAPIC::EnableLocalAPIC();
    IOAPIC::InitializeRedirectionTable();
// good
    // To-do : Make proper stack system
    printf("Core Count : %d\n" , KernelStackSize*CoreInformation::GetInstance()->CoreCount);
    KernelStackBase = (unsigned long)MemoryManagement::Allocate(KernelStackSize*CoreInformation::GetInstance()->CoreCount);
    memset(((void *)KernelStackBase) , 0 , (KernelStackSize*CoreInformation::GetInstance()->CoreCount));
    KernelStackBase += KernelStackSize*CoreInformation::GetInstance()->CoreCount;

    LocalAPIC::ActivateAPCores();
    __asm__ ("sti");
    
    printf("Kernel stack base initialized\n");
    FileSystem::Initialize();
    printf("File System Initialized\n");
    MountSystem::Initialize();
    printf("Mount System Initialized\n");
    StorageSystem::Initialize();
    printf("Storage System Initialized\n");
    
    ISO9660::Register();
    FAT16::Register();
    printf("File System Registered\n");
    
    RAMDiskDriver::Register();
    IDEDriver::Register();
    printf("Storage Driver Registered\n");
    PCI::Detect();
    printf("PCI Driver Registered\n");

    unsigned char *RAMDiskBuffer = (unsigned char *)MemoryManagement::Allocate(16384*1024);
    unsigned char *RAMDiskBuffer2 = (unsigned char *)MemoryManagement::Allocate(16384*1024);
    struct Storage *Storage = RAMDiskDriver::CreateRAMDisk((BOOTRAMDISK_ENDADDRESS-BOOTRAMDISK_ADDRESS)/BOOTRAMDISK_BYTES_PER_SECTOR , BOOTRAMDISK_BYTES_PER_SECTOR , BOOTRAMDISK_ADDRESS);
    memset(RAMDiskBuffer , 0 , 16*1024*1024);
    memset(RAMDiskBuffer2 , 0 , 16*1024*1024);
    printf("RAMDisk created - 1\n");
    struct Storage *MainStorage = RAMDiskDriver::CreateRAMDisk(16*1024*1024/512 , 512 , (unsigned long)RAMDiskBuffer);
    printf("RAMDisk created - 2\n");
    CreatePartition_FAT16(MainStorage , "LEL" , 128 , 32768-128);
    
    struct Storage *MainStoragePartition = MainStorage->LogicalStorages->GetObject(0);
    FileSystem::SetHeadStorage(MainStoragePartition);
    printf("MainStorage : 0x%X\n" , MainStoragePartition);
    
    struct FileInfo *File = Storage->FileSystem->OpenFile(Storage , "RDIMG.IMG" , FILESYSTEM_OPEN_READ);
    Storage->FileSystem->ReadFile(File , File->FileSize , RAMDiskBuffer2);
    Storage =  RAMDiskDriver::CreateRAMDisk(16384*1024/512 , 512 , (unsigned long)RAMDiskBuffer2);

    MainStoragePartition->FileSystem->CreateDir(MainStoragePartition , "System");
    Shell::ShellSystem ShellSystem;
    
    ShellSystem.Start();

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
    MemoryManagement::Free(BootSector);
}

/// @brief Create FAT16 partition to a storage
/// @param Storage Target storage
/// @param VolumeLabel Volume Label
/// @param StartAddress Start sector of partition
/// @param Size LBA size of partition
void CreatePartition_FAT16(struct Storage *Storage , const char *VolumeLabel , unsigned long StartAddress , unsigned long Size) {
    struct Partition Partition;
    struct FAT16::SFNEntry VolumeLabelEntry;
    
    struct FAT16::VBR VBR;
    struct Storage *CurrentPartition;
    char VolumeLabelSFNName[12];
    unsigned char *BootSector = (unsigned char *)MemoryManagement::Allocate(512);
    MBR::Identifier MBRIdentifier;

    Partition.StartAddressLBA = StartAddress; // Always starts in this sector
    Partition.EndAddressLBA = StartAddress+Size;
    Partition.PartitionType = 0x86;
    Partition.IsBootable = 0;
    
    MBRIdentifier.WriteStorageInfo(Storage->Driver , Storage);
    MBRIdentifier.CreatePartition(Partition); // Physically create partition
    StorageSystem::AddLogicalDrive(Storage->Driver , Storage , &(Partition) , 1); // Setup basic information
    
    CurrentPartition = Storage->LogicalStorages->GetObject(Storage->LogicalStorages->Count-1);
    printf("CurrentPartition : 0x%X\n" , CurrentPartition);
    FAT16::WriteVBR(&(VBR) , CurrentPartition , "POTATOOS" , "NO NAME   " , "FAT16     ");
    memset(BootSector , 0 , 512);
    memcpy(BootSector , &(VBR) , sizeof(struct FAT16::VBR));
    memcpy(BootSector+sizeof(struct FAT16::VBR) , CommonByteCode , sizeof(CommonByteCode));
    BootSector[510] = 0x55;
    BootSector[511] = 0xAA;
    CurrentPartition->Driver->WriteSector(CurrentPartition , 0 , 1 , BootSector);
    MemoryManagement::Free(BootSector);

    memset(&(VolumeLabelEntry) , 0 , sizeof(struct FAT16::SFNEntry));
    FAT16::CreateVolumeLabelName(VolumeLabelSFNName , VolumeLabel);
    memcpy(VolumeLabelEntry.FileName , VolumeLabelSFNName , 11);
    VolumeLabelEntry.Attribute = 0x08;
    
    FAT16::WriteClusterInfo(CurrentPartition , 0 , 0xFFF8 , &(VBR));
    FAT16::WriteClusterInfo(CurrentPartition , 1 , 0xFFFF , &(VBR));
    FAT16::WriteSFNEntry(CurrentPartition , FAT16::GetRootDirectoryLocation(&(VBR)) , &(VolumeLabelEntry));
    
    CurrentPartition->FileSystem = new FAT16::Driver;
    strcpy(CurrentPartition->FileSystemString , "FAT16");
}

extern "C" void APStartup(void) {
    /* To do : Seperate AP & BSP , 
     * Create TSS for AP
     * Create Memory Management System
     * Create IO Redirection table
     */
    __asm__ ("cli");
    DescriptorTables::Initialize();
    LocalAPIC::EnableLocalAPIC();
    LocalAPIC::Timer::Initialize();
    __asm__ ("sti");
    while(1) {
        ;
    }
}
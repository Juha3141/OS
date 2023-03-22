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

#include <FileSystem/ISO9660.hpp>
#include <FileSystem/FAT16.hpp>

using namespace Kernel;
using namespace Drivers;

unsigned long KernelStackBase;
unsigned long KernelStackSize = 2*1024*1024;
unsigned int StackReady = 0;

// problem : Kernel stack is too small & overrides kernel
// One core per 2MB *** 

Kernel::MutualExclusion::SpinLock CoreSpinLock;

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
    
    LocalAPIC::Timer::Initialize();
    LocalAPIC::GlobalEnableLocalAPIC();
    LocalAPIC::EnableLocalAPIC();
    IOAPIC::InitializeRedirectionTable();
    CoreSpinLock.Initialize();
    LocalAPIC::ActiveAPCores();

    // To-do : Make proper stack system
    KernelStackBase = (unsigned long)Kernel::MemoryManagement::Allocate(KernelStackSize*CoreInformation::GetInstance()->CoreCount);
    KernelStackBase += KernelStackSize*CoreInformation::GetInstance()->CoreCount;
    StackReady = 1;

    __asm__ ("sti");
    Kernel:printf("Kernel core initialized\n");

    while(1) {
        ;
    }

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
    StorageSystem::Storage *ISORAMDisk = Drivers::RAMDisk::CreateRAMDisk(12736 , 2048 , 0x720000);
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
    Kernel::printf("Done\n");
    FileSystem::FileInfo *FileInfo = FileSystem::FAT16::OpenFile(SysRAMDisk , "Hello.txt");
    if(FileInfo == 0x00) {
        Kernel::printf("File \"Hello.txt\" not found.\n");
    }
    FileSystem::FileInfo *FileInfo2 = FileSystem::FAT16::OpenFile(SysRAMDisk , "Testing/Hello.txt");
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
        Kernel::printf("%c" , Kernel::Keyboard::GetASCIIData());
    }
}

void TestTask(void) {
    unsigned char *VideoMemory = (unsigned char *)(0xB8000+(TaskManagement::GetCurrentlyRunningTaskID()*2));
    char haha[4] = {'-' , '\\' , '|' , '/'};
    unsigned char i = 0;
    while(1) {
        *(VideoMemory) = (i++);
        Kernel::PIT::DelayMicroseconds(100);
    }
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

// To-do : More reliable way to detect core
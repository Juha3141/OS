#include <Drivers/DeviceDriver.hpp>
#include <Drivers/StorageDriver.hpp>
#include <Drivers/FileSystemDriver.hpp>

#include <FileSystem/MBR.hpp>
#include <FileSystem/GPT.hpp>

using namespace Kernel;
using namespace Kernel::Drivers;

struct DriverSystemManager *StorageDriverManager;

void StorageSystem::Initialize(void) {
    StorageDriverManager = (struct DriverSystemManager *)Kernel::MemoryManagement::Allocate(sizeof(struct DriverSystemManager));
    StorageDriverManager->Initialize();
}

Kernel::Drivers::StorageSystem::Driver *Kernel::Drivers::StorageSystem::AssignDriver(
StandardPreInitializationFunction PreInitialization , 
StandardReadSectorFunction ReadSectorFunction , 
StandardWriteSectorFunction WriteSectorFunction , 
StandardGetGeometryFunction GetGeometryFunction) {
    StorageSystem::Driver *StorageDriver = (StorageSystem::Driver *)Kernel::MemoryManagement::Allocate(sizeof(StorageSystem::Driver));
    StorageDriver->PreInitialization = PreInitialization;
    StorageDriver->ReadSectorFunction = ReadSectorFunction;
    StorageDriver->WriteSectorFunction = WriteSectorFunction;
    StorageDriver->GetGeometryFunction = GetGeometryFunction;
    return StorageDriver;
}

bool StorageSystem::RegisterStorageDriver(StorageSystem::Driver *StorageDriver , const char *DriverName) {
    int i;
    unsigned long ID;
    if((ID = StorageDriverManager->Register((unsigned long)StorageDriver)) == DRIVERSYSTEM_INVALIDID) {
        Kernel::printf("Failed registering storage system.\n");
        return false;
    }
    StorageDriver->StoragesManager = (struct DriverSystemManager *)Kernel::MemoryManagement::Allocate(sizeof(struct DriverSystemManager));
    StorageDriver->StoragesManager->Initialize();
    StorageDriver->ID = ID;
    strcpy(StorageDriver->DriverName , DriverName);
    if(StorageDriver->PreInitialization(StorageDriver) == false) { // Detect and register the storage system
        Kernel::printf("Pre-initialization failed\n");
        return false;
    }
    return true;
}

StorageSystem::Driver *StorageSystem::SearchStorageDriver(const char *DriverName) {
    int i;
    if(StorageDriverManager->SystemCount == 0x00) {
        return 0;
    }
    for(i = 0; i < StorageDriverManager->SystemCount; i++) {
        if(memcmp(((StorageSystem::Driver *)StorageDriverManager->SystemList[i])->DriverName , DriverName , strlen(DriverName)) == 0) {
            return (StorageSystem::Driver *)StorageDriverManager->SystemList[i];
        }
    }
    return 0; // can't find storage
}

StorageSystem::Driver *StorageSystem::SearchStorageDriver(unsigned long ID) {
    return (StorageSystem::Driver *)StorageDriverManager->GetSystem(ID);
}

StorageSystem::Driver *StorageSystem::DeregisterStorageDriver(const char *DriverName) {
    int i;
    for(i = 0; i < StorageDriverManager->SystemCount; i++) {
        if(memcmp(((StorageSystem::Driver *)StorageDriverManager->SystemList[i])->DriverName , DriverName , strlen(DriverName)) == 0) {
            return (StorageSystem::Driver *)StorageDriverManager->Deregister(i);
        }
    }
    return 0; // can't find storage
}

StorageSystem::Driver *StorageSystem::DeregisterStorageDriver(unsigned long ID) {
    return (StorageSystem::Driver *)StorageDriverManager->Deregister(ID);
}

StorageSystem::Storage *StorageSystem::AssignStorage(int PortsCount , int FlagsCount , int IRQsCount , int ResourcesCount) {
    StorageSystem::Storage *Storage;
    Storage = (StorageSystem::Storage *)Kernel::MemoryManagement::Allocate(sizeof(StorageSystem::Storage));
    if(PortsCount != 0x00) {
        Storage->Ports = (unsigned short *)Kernel::MemoryManagement::Allocate(PortsCount*sizeof(unsigned short));
        Storage->PortsCount = PortsCount;
    }
    if(FlagsCount != 0x00) {
        Storage->Flags = (unsigned long *)Kernel::MemoryManagement::Allocate(FlagsCount*sizeof(unsigned long));
        Storage->FlagsCount = FlagsCount;
    }
    if(IRQsCount != 0x00) {
        Storage->IRQs = (unsigned int *)Kernel::MemoryManagement::Allocate(IRQsCount*sizeof(unsigned int));
        Storage->IRQsCount = IRQsCount;
    }
    if(ResourcesCount != 0x00) {
        Storage->Resources = (unsigned long *)Kernel::MemoryManagement::Allocate(ResourcesCount*sizeof(unsigned long));
        Storage->ResourcesCount = ResourcesCount;
    }
    return Storage;
}

bool StorageSystem::RegisterStorage(StorageSystem::Driver *StorageDriver , StorageSystem::Storage *Storage) {
    int i;
    FileSystem::Standard *FileSystem;
    Partition *PartitionNode;
    if(StorageDriver == 0x00) {
        return false;
    }
    MBR::Identifier MBRIdentifier(StorageDriver , Storage);
    GPT::Identifier GPTIdentifier(StorageDriver , Storage);
    Storage->Driver = StorageDriver;
    Storage->ID = StorageDriver->StoragesManager->Register((unsigned long)Storage);
    if(Storage->ID == DRIVERSYSTEM_INVALIDID) {
        return false;
    }
    if(StorageDriver->GetGeometryFunction(Storage , &(Storage->Geometry)) == false) {
        Kernel::printf("Failed getting geometry\n");
        return false;
    }
    // errorrr
    if(MBRIdentifier.Detect() == true) {
        Storage->Partitions = MBRIdentifier.GetPartition();
        Storage->PartitionCount = MBRIdentifier.PartitionCount;
        Kernel::printf("Partition Count : %d\n" , MBRIdentifier.PartitionCount);
        Storage->PartitionScheme = STORAGESYSTEM_MBR;
    }
    else {
        if(GPTIdentifier.Detect() == true) {
            Storage->Partitions = GPTIdentifier.GetPartition();
            Storage->PartitionCount = GPTIdentifier.PartitionCount;
            Kernel::printf("Partition Count : %d\n" , GPTIdentifier.PartitionCount);
            Storage->PartitionScheme = STORAGESYSTEM_GPT;
        }
        else {
            Storage->PartitionScheme = STORAGESYSTEM_ETC;
        }
    }
    FileSystem = FileSystem::DetectFileSystem(Storage);
    if(FileSystem == 0x00) {
        Storage->FileSystem = 0x00;
        strcpy(Storage->FileSystemString , "NONE");
        Kernel::printf("Warning : Storage %d has no file system.\n" , Storage->ID);
        return true;
    }
    Storage->FileSystem = FileSystem;
    Kernel::printf("File System Detected : %s\n" , FileSystem->FileSystemString);
    strcpy(Storage->FileSystemString , FileSystem->FileSystemString);
    return true;
}

bool StorageSystem::RegisterStorage(const char *DriverName , StorageSystem::Storage *Storage) {
    return RegisterStorage(SearchStorageDriver(DriverName) , Storage);
}

StorageSystem::Storage *StorageSystem::SearchStorage(const char *DriverName , unsigned long StorageID) {
    StorageSystem::Driver *StorageDriver = SearchStorageDriver(DriverName);
    return (StorageSystem::Storage *)StorageDriver->StoragesManager->GetSystem(StorageID);
}

bool StorageSystem::DeregisterStorage(const char *DriverName , unsigned long StorgeID) {
    return true;
}
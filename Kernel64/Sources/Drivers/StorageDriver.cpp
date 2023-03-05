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
        if(memcmp(((StorageSystem::Driver *)StorageDriverManager->SystemList[i])->DriverName , DriverName , strlen(((StorageSystem::Driver *)StorageDriverManager->SystemList[i])->DriverName)) == 0) {
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

void AddLogicalDrive(StorageSystem::Driver *StorageDriver , StorageSystem::Storage *Storage , StorageSystem::Partition *Partitions , int PartitionCount) {
    int i;
    Storage->LogicalStorages = (StorageSystem::Storage **)Kernel::MemoryManagement::Allocate(PartitionCount*sizeof(StorageSystem::Storage *));
    for(i = 0; i < PartitionCount; i++) {
        Storage->LogicalStorages[i] = (StorageSystem::Storage *)Kernel::MemoryManagement::Allocate(sizeof(StorageSystem::Storage));
        memcpy(&(Storage->LogicalStorages[i]->Geometry) , &(Storage->Geometry) , sizeof(StorageSystem::StorageGeometry));
        Storage->LogicalStorages[i]->Ports = Storage->Ports;
        Storage->LogicalStorages[i]->PortsCount = Storage->PortsCount;
        Storage->LogicalStorages[i]->IRQs = Storage->IRQs;
        Storage->LogicalStorages[i]->IRQsCount = Storage->IRQsCount;
        Storage->LogicalStorages[i]->Flags = Storage->Flags;
        Storage->LogicalStorages[i]->FlagsCount = Storage->FlagsCount;
        Storage->LogicalStorages[i]->Resources = Storage->Resources;
        Storage->LogicalStorages[i]->ResourcesCount = Storage->ResourcesCount;
        Storage->LogicalStorages[i]->StorageType = 0x01;
        memcpy(&(Storage->LogicalStorages[i]->LogicalPartitionInfo) , &(Partitions[i]) , sizeof(StorageSystem::Partition));
        RegisterStorage(StorageDriver , Storage->LogicalStorages[i]);
    }
}

bool StorageSystem::RegisterStorage(StorageSystem::Driver *StorageDriver , StorageSystem::Storage *Storage) {
    int i;
    StorageSystem::Storage *LogicalStorage;
    FileSystem::Standard *FileSystem;
    Partition *Partitions;
    if(StorageDriver == 0x00) {
        return false;
    }
    MBR::Identifier MBRIdentifier(StorageDriver , Storage);
    GPT::Identifier GPTIdentifier(StorageDriver , Storage);
    if(Storage->StorageType == 0x00) {
        if(StorageDriver->GetGeometryFunction(Storage , &(Storage->Geometry)) == false) {
            return false;
        }
    }
    Storage->Driver = StorageDriver;
    // Storage is dependent to StorageDriver. We need to seperate them, and create universal storage info container system.
    Storage->ID = StorageDriver->StoragesManager->Register((unsigned long)Storage); // StorageDriver***
    if(Storage->ID == DRIVERSYSTEM_INVALIDID) {
        return false;
    }
    Kernel::printf("Registered Storage System , ID : %d(%s)\n" , Storage->ID , (Storage->StorageType == 0x00) ? "Physical" : "Logical");
    // Goal : Create indivisual partition
    if(MBRIdentifier.Detect() == true) {
        Partitions = MBRIdentifier.GetPartition();
        Storage->PartitionCount = MBRIdentifier.PartitionCount;
        Storage->PartitionScheme = STORAGESYSTEM_MBR;
        AddLogicalDrive(StorageDriver , Storage , Partitions , MBRIdentifier.PartitionCount);
    }
    else {
        if(GPTIdentifier.Detect() == true) {
            Partitions = GPTIdentifier.GetPartition();
            Storage->PartitionCount = GPTIdentifier.PartitionCount;
            Storage->PartitionScheme = STORAGESYSTEM_GPT;
            AddLogicalDrive(StorageDriver , Storage , Partitions , GPTIdentifier.PartitionCount);
        }
        else {
            Storage->PartitionScheme = STORAGESYSTEM_ETC;
        }
    }
    FileSystem = FileSystem::DetectFileSystem(Storage);
    if(FileSystem == 0x00) {
        Storage->FileSystem = 0x00;
        strcpy(Storage->FileSystemString , "NONE");
        Kernel::printf("No file system\n");
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
    if(StorageDriver == 0x00) {
        return 0x00;
    }
    return (StorageSystem::Storage *)StorageDriver->StoragesManager->GetSystem(StorageID);
}

bool StorageSystem::DeregisterStorage(const char *DriverName , unsigned long StorgeID) {
    return true;
}
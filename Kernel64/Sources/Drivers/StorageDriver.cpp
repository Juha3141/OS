#include <Drivers/StorageDriver.hpp>
#include <Drivers/FileSystemDriver.hpp>

#include <FileSystem/MBR.hpp>
#include <FileSystem/GPT.hpp>

using namespace Kernel;

void StorageSystem::Initialize(void) {
    StorageDriverManager::GetInstance()->Initialize();
    // StorageManager::GetInstance()->Initialize();
    // Revision 2-2 : Implement indivisual storage manager for indivisual drivers.
}

// Interface of StorageDriverManager
bool StorageSystem::RegisterDriver(StorageDriver *Driver , const char *DriverName) {
    StorageDriverManager *DriverManager = StorageDriverManager::GetInstance();
    Kernel::printf("DriverManager : 0x%X\n" , DriverManager);
    Kernel::printf("Driver : 0x%X\n" , Driver);
    if(DriverManager->Register(Driver) == STORAGESYSTEM_INVALIDID) {
        return false;
    }
    Driver->StorageManager = new class StorageManager;
    Driver->StorageManager->Initialize();
    strcpy(Driver->DriverName , DriverName);
    Kernel::printf("Registered driver info. : ID %d , registered as name \"%s\"\n" , Driver->DriverID , Driver->DriverName);
    Driver->PreInitialization();
    return true;
}

StorageDriver *StorageSystem::SearchDriver(const char *DriverName) {
    return StorageDriverManager::GetInstance()->GetObjectByName(DriverName);
}

StorageDriver *StorageSystem::SearchDriver(unsigned long ID) {
    return StorageDriverManager::GetInstance()->GetObject(ID);
}

unsigned long StorageSystem::DeregisterDriver(const char *DriverName) {
    return StorageDriverManager::GetInstance()->Deregister(
        StorageDriverManager::GetInstance()->GetObjectByName(DriverName)
    );
}

unsigned long StorageSystem::DeregisterDriver(unsigned long ID) {
    return StorageDriverManager::GetInstance()->Deregister(
        StorageDriverManager::GetInstance()->GetObject(ID)
    );
}

bool StorageSystem::RegisterStorage(struct StorageDriver *Driver , struct Storage *Storage) {
    int i;
    StorageDriverManager *DriverManager = StorageDriverManager::GetInstance();
    
    struct LogicalStorage *LogicalStorage;
    struct Partition *Partitions;

    StorageSchemeIdentifier *StorageIdentifier[2];
    FileSystemDriver *FileSystem;
    if(Driver == 0x00) {
        return false;
    }
    if(Storage->Type == Storage::StorageType::Physical) {
        if(Driver->GetGeometry(Storage , &(Storage->PhysicalInfo.Geometry)) == false) {
            return false;
        }
        Storage->ID = Driver->StorageManager->Register(Storage); // call superclass
        if(Storage->ID == STORAGESYSTEM_INVALIDID) {
            return false;
        }
        Kernel::printf("Registered Physical Storage System , ID : %d(%s)\n" , Storage->ID , (Storage->Type == Storage::StorageType::Physical) ? "Physical" : "Logical");
        Storage->PartitionID = PARTITIONID_PHYSICALDRIVE;
        Storage->Driver = Driver;
    }
    else {
        Storage->Driver = new PartitionDriver;
        ((PartitionDriver *)Storage->Driver)->SetSuperDriver(Driver);
        
    }
    // If it's logical storage, do not register to storage manager
    
    StorageIdentifier[0] = new MBR::Identifier(Driver , Storage);
    StorageIdentifier[1] = new GPT::Identifier(Driver , Storage);
    
    // Storage is dependent to StorageDriver. We need to seperate them, and create universal storage info container system.
    // Goal : Create indivisual partition
    Storage->PartitionScheme = STORAGESYSTEM_ETC;
    for(int i = 0; i < 2; i++) {
        if(StorageIdentifier[i]->Detect() == true) {
            Partitions = StorageIdentifier[i]->GetPartition();
            Storage->PartitionScheme = (i+1); // 1 : MBR , 2 : GPT
            
            Storage->LogicalStorages = new struct StorageManager;
            Storage->LogicalStorages->Initialize();
            
            AddLogicalDrive(Driver , Storage , Partitions , StorageIdentifier[i]->PartitionCount);
            break;
        }
    }

    FileSystem = FileSystem::DetectFileSystem(Storage);
    if(FileSystem == 0x00) {
        Storage->FileSystem = 0x00;
        strcpy(Storage->FileSystemString , "NONE");
        Kernel::printf("%s%d" , Driver->DriverName , Storage->ID);
        if(Storage->PartitionID != PARTITIONID_PHYSICALDRIVE) {
            Kernel::printf(",part%d" , Storage->PartitionID);
        }
        Kernel::printf(" : No file system\n");
        return true;
    }
    Storage->FileSystem = FileSystem;
    strcpy(Storage->FileSystemString , FileSystem->FileSystemString);
    
    Kernel::printf("%s%d : File System Detected : %s\n" , Driver->DriverName , Storage->ID , Storage->FileSystemString);
    return true;
}

bool StorageSystem::RegisterStorage(unsigned long DriverID , struct Storage *Storage) {
    StorageDriver *Driver = SearchDriver(DriverID);
    if(Driver == 0x00) {
        return false;
    }
    return RegisterStorage(Driver , Storage);
}

bool StorageSystem::RegisterStorage(const char *DriverName , struct Storage *Storage) {
    StorageDriver *Driver = SearchDriver(DriverName);
    if(Driver == 0x00) {
        return false;
    }
    return RegisterStorage(Driver , Storage);
}

struct Storage *StorageSystem::SearchStorage(const char *DriverName , unsigned long StorageID) {
    struct StorageDriver *StorageDriver = SearchDriver(DriverName);
    if(StorageDriver == 0x00) {
        return 0x00;
    }
    return (struct Storage *)StorageDriver->StorageManager->GetObject(StorageID);
}

struct Storage *StorageSystem::SearchStorage(unsigned long DriverID , unsigned long StorageID) {
    struct StorageDriver *StorageDriver = SearchDriver(DriverID);
    if(StorageDriver == 0x00) {
        return 0x00;
    }
    return (struct Storage *)StorageDriver->StorageManager->GetObject(StorageID);
}

/// @brief Add logical drive to Storage (Disclaimer : This function doesn't calculate how full the storage is)
//         Use at your own risk, this function can't guarantee the stability when disk's full.
/// @param StorageDriver Storage driver (Stroage->Driver)
/// @param Storage Physical Storage
/// @param Partitions List of partitions
/// @param PartitionCount number of partition to add 
void StorageSystem::AddLogicalDrive(StorageDriver *Driver , struct Storage *Storage , struct Partition *Partitions , int PartitionCount) {
    int i;
    int j = 0;
    int CurrentPartitionCount;
    struct Storage *LogicalStorage;
    if(Storage->LogicalStorages == 0x00) {
        Storage->LogicalStorages = new struct StorageManager;
        Storage->LogicalStorages->Initialize();
    }
    CurrentPartitionCount = Storage->LogicalStorages->CurrentObjectCount;
    if((Storage->PartitionScheme == STORAGESYSTEM_MBR) && (Storage->LogicalStorages->CurrentObjectCount == 4)) {
        return;
    }
    Kernel::printf("PartitionCount : %d\n" , PartitionCount);
    for(i = CurrentPartitionCount; i < CurrentPartitionCount+PartitionCount; i++) {
        LogicalStorage = (struct Storage *)Kernel::MemoryManagement::Allocate(sizeof(struct Storage));
        // Write info of parent storage
        LogicalStorage->ID = Storage->ID;
        LogicalStorage->Type = Storage::StorageType::Logical;

        memcpy(&(LogicalStorage->PhysicalInfo) , &(Storage->PhysicalInfo) , sizeof(struct PhysicalStorageInfo));
        memcpy(&(LogicalStorage->LogicalPartitionInfo) , &(Partitions[j++]) , sizeof(struct Partition));
        // to-do : fix infinite loop error
        LogicalStorage->PartitionID = Storage->LogicalStorages->Register(LogicalStorage);
        Kernel::printf("LogicalStorage->PartitionID : %d\n" , LogicalStorage->PartitionID);
        Kernel::printf("Partition %d : %d~%d\n" , i-CurrentPartitionCount , LogicalStorage->LogicalPartitionInfo.StartAddressLBA , LogicalStorage->LogicalPartitionInfo.StartAddressLBA+LogicalStorage->LogicalPartitionInfo.EndAddressLBA);
        RegisterStorage(Driver , LogicalStorage);
    }
    Kernel::printf("Storage->LogicalStorages : 0x%X\n" , Storage->LogicalStorages);
    Kernel::printf("Storage->PartitionCount : %d\n" , Storage->LogicalStorages->CurrentObjectCount);
}

static void AssignPhysicalInfo(Storage *Storage , int PortsCount , int FlagsCount , int IRQsCount , int ResourcesCount) {
    if(PortsCount != 0x00) {
        Storage->PhysicalInfo.Ports = (unsigned short *)Kernel::MemoryManagement::Allocate(PortsCount*sizeof(unsigned short));
        Storage->PhysicalInfo.PortsCount = PortsCount;
    }
    if(FlagsCount != 0x00) {
        Storage->PhysicalInfo.Flags = (unsigned long *)Kernel::MemoryManagement::Allocate(FlagsCount*sizeof(unsigned long));
        Storage->PhysicalInfo.FlagsCount = FlagsCount;
    }
    if(IRQsCount != 0x00) {
        Storage->PhysicalInfo.IRQs = (unsigned int *)Kernel::MemoryManagement::Allocate(IRQsCount*sizeof(unsigned int));
        Storage->PhysicalInfo.IRQsCount = IRQsCount;
    }
    if(ResourcesCount != 0x00) {
        Storage->PhysicalInfo.Resources = (unsigned long *)Kernel::MemoryManagement::Allocate(ResourcesCount*sizeof(unsigned long));
        Storage->PhysicalInfo.ResourcesCount = ResourcesCount;
    }
}

struct Storage *StorageSystem::Assign(int PortsCount , int FlagsCount , int IRQsCount , int ResourcesCount , enum Storage::StorageType Type) {
    struct Storage *Storage = (struct Storage *)Kernel::MemoryManagement::Allocate(sizeof(struct Storage));
    AssignPhysicalInfo((struct Storage *)Storage , PortsCount , FlagsCount , IRQsCount , ResourcesCount);
    Storage->Type = Type;
    return Storage;
}

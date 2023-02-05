#include <Drivers/DeviceDriver.hpp>
#include <Drivers/StorageDriver.hpp>
#include <Drivers/FileSystemDriver.hpp>

struct Kernel::Drivers::DriverSystemManager<Kernel::Drivers::StorageSystem::Standard> *StorageSystemManager;

void Kernel::Drivers::StorageSystem::Initialize(void) {
    StorageSystemManager = (struct DriverSystemManager<StorageSystem::Standard> *)Kernel::MemoryManagement::Allocate(sizeof(struct DriverSystemManager<StorageSystem::Standard>));
    StorageSystemManager->Initialize();
    Kernel::printf("StorageSystemManager : 0x%X\n" , StorageSystemManager);
}

Kernel::Drivers::StorageSystem::Standard *Kernel::Drivers::StorageSystem::AssignSystem(
StandardPostInitializationFunction PostInitialization , 
StandardReadSectorFunction ReadSectorFunction , 
StandardWriteSectorFunction WriteSectorFunction , 
StandardGetGeometryFunction GetGeometrySectorFunction) {
    StorageSystem::Standard *StorageSystem = (StorageSystem::Standard *)Kernel::MemoryManagement::Allocate(sizeof(StorageSystem::Standard));
    StorageSystem->PostInitialization = PostInitialization;
    StorageSystem->ReadSectorFunction = ReadSectorFunction;
    StorageSystem->WriteSectorFunction = WriteSectorFunction;
    StorageSystem->GetGeometryFunction = GetGeometrySectorFunction;
    if(GetGeometrySectorFunction(&(StorageSystem->Geometry)) == false) {
        Kernel::printf("Failed gathering geometry information\n");
    }
    if(PostInitialization() == false) {
        Kernel::printf("Post-initialization failed\n");
        return 0x00;
    }
    return StorageSystem;
}

bool Kernel::Drivers::StorageSystem::Register(Kernel::Drivers::StorageSystem::Standard *StorageSystem , const char *Name) {
    int i;
    unsigned long ID;
    FileSystem::Standard *FileSystem;
    if((ID = StorageSystemManager->Register(StorageSystem)) == DRIVERSYSTEM_INVALIDID) {
        Kernel::printf("Failed registering storage system.\n");
        return false;
    }
    StorageSystem->ID = ID;
    FileSystem = FileSystem::DetectFileSystem(StorageSystem);
    if(FileSystem == 0x00) {
        StorageSystem->FileSystem = 0x00;
        strcpy(StorageSystem->FileSystemString , "NONE");
        Kernel::printf("Warning : Storage %s has no file system.\n" , Name);
        return true;
    }
    StorageSystem->FileSystem = FileSystem;
    Kernel::printf("File System Detected : %s\n" , FileSystem->FileSystemString);
    strcpy(StorageSystem->FileSystemString , FileSystem->FileSystemString);
    strcpy(StorageSystem->StorageName , Name);
    return true;
}

Kernel::Drivers::StorageSystem::Standard *Kernel::Drivers::StorageSystem::Search(const char *StorageName) {
    int i;
    for(i = 0; i < StorageSystemManager->SystemCount; i++) {
        if(memcmp(StorageSystemManager->SystemList[i].System->StorageName , StorageName , strlen(StorageName)) == 0) {
            return StorageSystemManager->SystemList[i].System;
        }
    }
    return 0; // can't find storage
}

Kernel::Drivers::StorageSystem::Standard *Kernel::Drivers::StorageSystem::Search(unsigned long ID) {
    return StorageSystemManager->GetSystem(ID);
}

Kernel::Drivers::StorageSystem::Standard *Kernel::Drivers::StorageSystem::Deregister(unsigned long ID) {
    return StorageSystemManager->Deregister(ID);
}
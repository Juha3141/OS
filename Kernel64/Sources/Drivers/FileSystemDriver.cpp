#include <Drivers/DeviceDriver.hpp>
#include <Drivers/FileSystemDriver.hpp>
#include <Drivers/StorageDriver.hpp>

struct Kernel::Drivers::DriverSystemManager<Kernel::FileSystem::Standard> *FileSystemManager;

void Kernel::FileSystem::Initialize(void) {
    FileSystemManager = (struct Drivers::DriverSystemManager<FileSystem::Standard> *)Kernel::MemoryManagement::Allocate(sizeof(struct Drivers::DriverSystemManager<FileSystem::Standard>));
    FileSystemManager->Initialize();
    Kernel::printf("FileSystemManager : 0x%X\n" , FileSystemManager);
}

Kernel::FileSystem::Standard *Kernel::FileSystem::AssignSystem(
StandardCheckFunction Check , 

StandardOpenFileFunction OpenFile , 
StandardCloseFileFunction CloseFile , 
StandardRemoveFileFunction RemoveFile , 

StandardWriteFileFunction WriteFile , 
StandardReadFileFunction ReadFile , 
StandardSetFileOffsetFunction SetFileOffset , 

StandardReadDirectoryFunction ReadDirectory , 
StandardGetFileCountInDirectoryFunction GetFileCountInDirectory) {
    FileSystem::Standard *FileSystem = (FileSystem::Standard *)Kernel::MemoryManagement::Allocate(sizeof(FileSystem::Standard));
    FileSystem->Check = Check;

    FileSystem->OpenFile = OpenFile;
    FileSystem->CloseFile = CloseFile;
    FileSystem->RemoveFile = RemoveFile;

    FileSystem->WriteFile = WriteFile;
    FileSystem->ReadFile = ReadFile;
    FileSystem->SetFileOffset = SetFileOffset;

    FileSystem->ReadDirectory = ReadDirectory;
    FileSystem->GetFileCountInDirectory = GetFileCountInDirectory;
    return FileSystem;
}

bool Kernel::FileSystem::Register(Kernel::FileSystem::Standard *FileSystem , const char *FileSystemString) {
    strcpy(FileSystem->FileSystemString , FileSystemString);
    if(FileSystemManager->Register(FileSystem) == false) {
        return false;
    }
    return true;
}

Kernel::FileSystem::Standard *Kernel::FileSystem::Search(unsigned long ID) {
    return FileSystemManager->GetSystem(ID);
}

Kernel::FileSystem::Standard *Kernel::FileSystem::Search(const char *FileSystemString) {
    int i;
    for(i = 0; i < FileSystemManager->SystemCount; i++) {
        if(memcmp(FileSystemManager->SystemList[i].System->FileSystemString , FileSystemString , strlen(FileSystemString)) == 0) {
            return FileSystemManager->SystemList[i].System;
        }
    }
    return 0x0; // can't find storage
}

Kernel::FileSystem::Standard *Kernel::FileSystem::DetectFileSystem(Drivers::StorageSystem::Standard *StorageSystem) {
    int i;
    for(i = 0; i < FileSystemManager->SystemCount; i++) {
        Kernel::printf("FileSystemManager->SystemList[i].System : 0x%X\n" , FileSystemManager->SystemList[i].System);
        Kernel::printf("FileSystemManager->SystemList[i].System->Check : 0x%X\n" , FileSystemManager->SystemList[i].System->Check);
        if(FileSystemManager->SystemList[i].System->Check(StorageSystem) == true) {
            return FileSystemManager->SystemList[i].System;
        }
    }
    return 0x00;
}
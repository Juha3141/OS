#include <Drivers/DeviceDriver.hpp>
#include <Drivers/FileSystemDriver.hpp>
#include <Drivers/StorageDriver.hpp>

struct Kernel::Drivers::DriverSystemManager *FileSystemManager;

void Kernel::FileSystem::Initialize(void) {
    FileSystemManager = (struct Drivers::DriverSystemManager *)Kernel::MemoryManagement::Allocate(sizeof(struct Drivers::DriverSystemManager));
    FileSystemManager->Initialize();
}

Kernel::FileSystem::Standard *Kernel::FileSystem::AssignSystem(
StandardCheckFunction Check , 

StandardOpenFileFunction OpenFile , 
StandardCloseFileFunction CloseFile , 
StandardRemoveFileFunction RemoveFile , 

StandardWriteFileFunction WriteFile , 
StandardReadFileFunction ReadFile , 

StandardReadDirectoryFunction ReadDirectory , 
StandardGetFileCountInDirectoryFunction GetFileCountInDirectory) {
    FileSystem::Standard *FileSystem = (FileSystem::Standard *)Kernel::MemoryManagement::Allocate(sizeof(FileSystem::Standard));
    FileSystem->Check = Check;

    FileSystem->OpenFile = OpenFile;
    FileSystem->CloseFile = CloseFile;
    FileSystem->RemoveFile = RemoveFile;

    FileSystem->WriteFile = WriteFile;
    FileSystem->ReadFile = ReadFile;

    FileSystem->ReadDirectory = ReadDirectory;
    FileSystem->GetFileCountInDirectory = GetFileCountInDirectory;
    return FileSystem;
}

bool Kernel::FileSystem::Register(Kernel::FileSystem::Standard *FileSystem , const char *FileSystemString) {
    strcpy(FileSystem->FileSystemString , FileSystemString);
    if(FileSystemManager->Register((unsigned long)FileSystem) == false) {
        return false;
    }
    return true;
}

Kernel::FileSystem::Standard *Kernel::FileSystem::Search(unsigned long ID) {
    return (Kernel::FileSystem::Standard *)FileSystemManager->GetSystem(ID);
}

Kernel::FileSystem::Standard *Kernel::FileSystem::Search(const char *FileSystemString) {
    int i;
    for(i = 0; i < FileSystemManager->SystemCount; i++) {
        if(memcmp(((Kernel::FileSystem::Standard *)FileSystemManager->SystemList[i])->FileSystemString , FileSystemString , strlen(FileSystemString)) == 0) {
            return (Kernel::FileSystem::Standard *)FileSystemManager->SystemList[i];
        }
    }
    return 0x0; // can't find storage
}

Kernel::FileSystem::Standard *Kernel::FileSystem::DetectFileSystem(Drivers::StorageSystem::Storage *Storage) {
    int i;
    for(i = 0; i < FileSystemManager->SystemCount; i++) {
        if(((Kernel::FileSystem::Standard *)FileSystemManager->SystemList[i])->Check(Storage) == true) {
            return (Kernel::FileSystem::Standard *)FileSystemManager->SystemList[i];
        }
    }
    return 0x00;
}
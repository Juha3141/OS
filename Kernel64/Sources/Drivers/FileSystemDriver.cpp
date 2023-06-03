#include <Drivers/FileSystemDriver.hpp>
#include <Drivers/StorageDriver.hpp>

void FileSystem::Initialize(void) {
    FileSystemManager::GetInstance()->Initialize();
}


bool FileSystem::Register(FileSystemDriver *Driver , const char *FileSystemString) {
    strcpy(Driver->FileSystemString , FileSystemString);
    if(FileSystemManager::GetInstance()->Register(Driver) == false) {
        return false;
    }
    return true;
}

FileSystemDriver *FileSystem::Search(unsigned long ID) {
    return FileSystemManager::GetInstance()->GetObject(ID);
}

FileSystemDriver *FileSystem::Search(const char *FileSystemString) {
    return FileSystemManager::GetInstance()->GetObjectByName(FileSystemString);
}

FileSystemDriver *FileSystem::DetectFileSystem(struct Storage *Storage) {
    int i;
    FileSystemManager *Manager = FileSystemManager::GetInstance();
    for(i = 0; i < Manager->Count; i++) {
        if(Manager->Check(i , Storage) == true) {
            return Manager->GetObject(i);
        }
    }
    return 0x00;
}
#include <FileSystem/VirtualFileSystem.hpp>

// File Name Format : ::[Storage Name]/Directory/Directory ...

static int GetStorageNameLength(const char *FileName) {
    int i;
    if((FileName[0] != ':')||(FileName[1] != ':')) {
        return 0;
    }
    for(i = 2; i < strlen(FileName); i++) {
        if(FileName[i] == '/') {
            break;
        }
    }
    return i-2;
}

Kernel::FileSystem::FileInfo *Kernel::FileSystem::OpenFile(const char *FileName , const char *Mode) {
    int StorageNameLength = GetStorageNameLength(FileName);
    char StorageName[StorageNameLength+1];
    char VirtualFileName[strlen(FileName)-2-StorageNameLength+1];
    Drivers::StorageSystem::Standard *StorageSystem;
    if(StorageNameLength == 0) {
        return 0x00;
    }
    strncpy(StorageName , FileName+2 , StorageNameLength);
    strncpy(VirtualFileName , FileName+2+StorageNameLength+1 , strlen(FileName)-2-StorageNameLength+1);
    StorageSystem = Drivers::StorageSystem::Search(StorageName);
    return StorageSystem->FileSystem->OpenFile(StorageSystem , VirtualFileName);
}

int Kernel::FileSystem::ReadFile(FileInfo *FileInfo , unsigned long Size , void *Buffer) {
    Drivers::StorageSystem::Standard *StorageSystem = Drivers::StorageSystem::Search(FileInfo->StorageSystemID);
    unsigned int ReadSize;
    unsigned int SectorAddress = FileInfo->SectorAddress+(FileInfo->FileOffset/FileInfo->BlockSize);
    unsigned int SectorCountToRead = Size/FileInfo->BlockSize+(((Size%FileInfo->BlockSize) == 0) ? 0 : 1);
    unsigned char *TempBuffer = (unsigned char *)Kernel::MemoryManagement::Allocate(SectorCountToRead*FileInfo->BlockSize);
    if((ReadSize = StorageSystem->ReadSectorFunction(SectorAddress , SectorCountToRead , TempBuffer)) == (SectorCountToRead*FileInfo->BlockSize)) {
        memcpy(Buffer , TempBuffer , ReadSize);
        FileInfo->FileOffset += ReadSize;
        Kernel::MemoryManagement::Free(TempBuffer);
        return ReadSize;
    }
    FileInfo->FileOffset += Size;
    memcpy(Buffer , TempBuffer , Size);
    Kernel::MemoryManagement::Free(TempBuffer);
    return Size;
}

int Kernel::FileSystem::CloseFile(FileInfo *FileInfo) {
    return 1;
}
#include <FileSystem/VirtualFileSystem.hpp>

// File Name Format : ::[Storage Name][ID]::[Partition ID]/Directory/Directory ...

static int GetStorageDriverNameLength(const char *FileName , int *NextOff) {
    int i;
    if((FileName[0] != ':')||(FileName[1] != ':')) {
        return 0;
    }
    for(i = 2; i < strlen(FileName); i++) {
        if((FileName[i] >= '0') && (FileName[i] <= '9')) {
            break;
        }
        if((FileName[i] == ':')||(FileName[i] == '/')) {
            return 0; 
        }
    }
    *NextOff = i;
    return i-2;
}

static int GetStorageID(const char *FileName , int *NextOff) {
    int i;
    int j;
    char StorageIDString[strlen(FileName)];
    if(!((FileName[*NextOff] >= '0') && (FileName[*NextOff] <= '9'))) {
        return 0;
    }
    for(i = *NextOff; i < strlen(FileName); i++) {
        if(!((FileName[i] >= '0') && (FileName[i] <= '9'))) {
            break;
        }
        StorageIDString[j++] = FileName[i];
    }
    StorageIDString[j++] = 0x00;
    *NextOff = i;
    return atoi(StorageIDString);
}

static int GetStoragePartitionID(const char *FileName , int *NextOff) {
    int i;
    int j = 0;
    char PartitionIDString[strlen(FileName)];
    if((FileName[*NextOff] != ':')||(FileName[*NextOff+1] != ':')) {
        return -1; // no partition
    }
    *NextOff += 2;
    for(i = *NextOff; i < strlen(FileName); i++) {
        if(!((FileName[i] >= '0') && (FileName[i] <= '9'))) {
            break;
        }
        PartitionIDString[j++] = FileName[i];
    }
    PartitionIDString[j] = 0x00;
    *NextOff = i+1;
    return atoi(PartitionIDString);
}

Kernel::FileSystem::FileInfo *Kernel::FileSystem::OpenFile(const char *FileName , const char *Mode) {
    int NextOff;
    int StorageDriverNameLength = GetStorageDriverNameLength(FileName , &(NextOff));
    int StorageID;
    int PartitionID;
    char StorageDriverName[strlen(FileName)];
    char VirtualFileName[strlen(FileName)];
    Drivers::StorageSystem::Storage *Storage;
    if(StorageDriverNameLength == 0) {
        return 0x00;
    }
    StorageID = GetStorageID(FileName , &(NextOff));
    if(StorageID == -1) {
        return 0x00; // temp
    }
    PartitionID = GetStoragePartitionID(FileName , &(NextOff));
    if(PartitionID == -1) {
        return 0x00; // temp
    }
    strncpy(StorageDriverName , FileName+2 , StorageDriverNameLength);
    strncpy(VirtualFileName , FileName+NextOff , strlen(FileName)-NextOff);
    Storage = Drivers::StorageSystem::SearchStorage(StorageDriverName , StorageID);
    if(Storage == 0x00) {
        return 0x00;
    }
    if(Storage->FileSystem == 0x00) {
        Kernel::printf("No file system in Storage %s #%d\n" , StorageDriverName , StorageID);
        return 0x00;
    }
    return Storage->FileSystem->OpenFile(Storage , VirtualFileName);
}

int Kernel::FileSystem::ReadFile(FileInfo *FileInfo , unsigned long Size , void *Buffer) {
    Drivers::StorageSystem::Driver *StorageDriver = Drivers::StorageSystem::SearchStorageDriver(FileInfo->StorageDriverID);
    Drivers::StorageSystem::Storage *Storage = Drivers::StorageSystem::SearchStorage(StorageDriver->DriverName , FileInfo->StorageID);
    unsigned int ReadSize;
    unsigned int SectorAddress = FileInfo->SectorAddress+(FileInfo->FileOffset/FileInfo->BlockSize);
    unsigned int SectorCountToRead = Size/FileInfo->BlockSize+(((Size%FileInfo->BlockSize) == 0) ? 0 : 1);
    unsigned char *TempBuffer = (unsigned char *)Kernel::MemoryManagement::Allocate(SectorCountToRead*FileInfo->BlockSize);
    if((ReadSize = StorageDriver->ReadSectorFunction(Storage , SectorAddress , SectorCountToRead , TempBuffer)) == (SectorCountToRead*FileInfo->BlockSize)) {
        memcpy(Buffer , TempBuffer+(FileInfo->FileOffset%FileInfo->BlockSize) , ReadSize);
        FileInfo->FileOffset += ReadSize;
        Kernel::MemoryManagement::Free(TempBuffer);
        return ReadSize;
    }
    memcpy(Buffer , TempBuffer+(FileInfo->FileOffset%FileInfo->BlockSize) , Size);
    FileInfo->FileOffset += Size;
    Kernel::MemoryManagement::Free(TempBuffer);
    return Size;
}

int Kernel::FileSystem::CloseFile(FileInfo *FileInfo) {
    return 1;
}
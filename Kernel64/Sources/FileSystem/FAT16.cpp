#include <FileSystem/FAT16.hpp>

using namespace Kernel;
using namespace Kernel::FileSystem;

bool FAT16::Register(void) {
    FileSystem::Standard *FAT16FileSystem = FileSystem::AssignSystem(
    FAT16::Check , 
    FAT16::CreateFile , 
    FAT16::CreateDir , 
    
    FAT16::OpenFile , 
    FAT16::CloseFile , 
    FAT16::RemoveFile , 

    FAT16::WriteFile , 
    FAT16::ReadFile , 
    
    FAT16::ReadDirectory , 
    FAT16::GetFileCountInDirectory);
    return FileSystem::Register(FAT16FileSystem , "FAT16");
}

bool FAT16::Check(Drivers::StorageSystem::Storage *Storage) {
    unsigned char *Buffer;
    struct VBR *BootSector;
    Buffer = (unsigned char *)Kernel::MemoryManagement::Allocate(Storage->Geometry.BytesPerSector);
    if(Storage->Driver->ReadSectorFunction(Storage , 0 , 1 , Buffer) != Storage->Geometry.BytesPerSector) {
        Kernel::printf("Failed reading\n");
        return false;
    }
    BootSector = (struct VBR *)Buffer;
    if(memcmp(BootSector->FileSystemType , "FAT16" , 5) == 0) {
        return true;
    }
    return false;
}

bool FAT16::CreateFile(Drivers::StorageSystem::Storage *Storage , const char *FileName) {
    return false;
}

bool FAT16::CreateDir(Drivers::StorageSystem::Storage *Storage , const char *DirectoryName) {
    return false;
}

// https://www.youtube.com/watch?v=qfumU7wLVd4

FileSystem::FileInfo *FAT16::OpenFile(Drivers::StorageSystem::Storage *Storage , const char *FileName) {
    int i = 0;
    int DirectoryCount;
    int DirectoryLocation;
    int FileClusterLocation;
    char **Directories;
    struct VBR VBR;
    struct SFNEntry SFNEntry;
    struct FileInfo *FileInfo;
    GetVBR(Storage , &(VBR));
    DirectoryCount = GetDirectoryCount(FileName);
    DirectoryLocation = GetRootDirectoryLocation(&(VBR));
    Directories = (char **)Kernel::MemoryManagement::Allocate((DirectoryCount+1)*sizeof(char *)); 
    if(DirectoryCount == 0) {
        strcpy(Directories[0] , FileName);
    }
    else {
        ParseDirectories(FileName , Directories);
        for(i = 0; i < DirectoryCount; i++) {
            GetSFNEntry(Storage , DirectoryLocation , Directories[i] , &(SFNEntry));
            FileClusterLocation = (SFNEntry.StartingClusterHigh << 16)|SFNEntry.StartingClusterLow;
            DirectoryLocation = ClusterToSector(FileClusterLocation , &(VBR));
        }
    }
    if(GetSFNEntry(Storage , DirectoryLocation , Directories[i] , &(SFNEntry)) == false) {
        return 0x00;
    }
    FileClusterLocation = (SFNEntry.StartingClusterHigh << 16)|SFNEntry.StartingClusterLow;
    FileInfo = (FileSystem::FileInfo *)Kernel::MemoryManagement::Allocate(sizeof(FileSystem::FileInfo));
    WriteFileInfo(FileInfo , SFNEntry , FileName , &(VBR));
    return FileInfo;
}

int FAT16::CloseFile(Drivers::StorageSystem::Storage *Storage , FileSystem::FileInfo *FileInfo) {
    return 0;
}

int FAT16::RemoveFile(Drivers::StorageSystem::Storage *Storage , FileSystem::FileInfo *FileInfo) {
    return 0;
}

int FAT16::WriteFile(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo , unsigned long Size , void *Buffer) {
    return 0;
}

int FAT16::ReadFile(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo , unsigned long Size , void *Buffer) {
    struct VBR VBR;
    unsigned int ReadSize;
    unsigned int SectorAddress = FileInfo->SectorAddress+(FileInfo->FileOffset/FileInfo->BlockSize);
    unsigned int SectorCountToRead = Size/FileInfo->BlockSize+(((Size%FileInfo->BlockSize) == 0) ? 0 : 1);
    unsigned char *TempBuffer = (unsigned char *)Kernel::MemoryManagement::Allocate(SectorCountToRead*FileInfo->BlockSize);
    GetVBR(Storage , &(VBR));
    if((ReadSize = ReadCluster(Storage , SectorToCluster(SectorAddress , &(VBR)) , SectorCountToRead*VBR.SectorsPerCluster , TempBuffer , &(VBR))) == (SectorCountToRead*FileInfo->BlockSize)) {
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

int FAT16::ReadDirectory(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo , struct FileSystem::FileInfo *FileList) {
    return 0;
}

int FAT16::GetFileCountInDirectory(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo) {
    return 0;
}

int FAT16::WriteDirectoryData(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo) {
    return 0;
}


void FAT16::WriteVBR(struct VBR *VBR , Drivers::StorageSystem::StorageGeometry *Geometry) {
    // error
}

unsigned int FAT16::ReadCluster(Drivers::StorageSystem::Storage *Storage , unsigned long ClusterNumber , unsigned long ClusterCountToRead , unsigned char *Data , struct VBR *VBR) {
    unsigned long i;
    unsigned long NextClusterAddress = ClusterNumber;
    for(i = 0; i < ClusterCountToRead*VBR->SectorsPerCluster; i++) {
        if(Storage->Driver->ReadSectorFunction(Storage , ClusterToSector(NextClusterAddress , VBR) , 1 , (Data+(i*VBR->BytesPerSector))) != Storage->Geometry.BytesPerSector) {
            break;
        }
        if(ClusterCountToRead%VBR->SectorsPerCluster == 0) {
            NextClusterAddress = FindNextCluster(Storage , NextClusterAddress , VBR);
        }
    }
    return i;
}

unsigned int FAT16::FindFirstEmptyCluster(Drivers::StorageSystem::Storage *Storage) {
    int i;
    unsigned char Data[512];
    struct VBR VBR;
    Storage->Driver->ReadSectorFunction(Storage , 0 , 1 , Data);
    memcpy(&(VBR) , Data , 512);
    for(i = 0; i < VBR.TotalSector16/VBR.SectorsPerCluster; i++) {
        if(FindNextCluster(Storage , i , &(VBR)) == 0x00) {
            return i;
        }
    }
    return 0xFFFFFFFF;
}

unsigned int FAT16::GetFATAreaLocation(struct VBR *VBR) {
    return VBR->ReservedSectorCount;
}

unsigned int FAT16::GetRootDirectoryLocation(struct VBR *VBR) {
    return VBR->ReservedSectorCount+(VBR->FATSize16*VBR->NumberOfFAT);
}

unsigned int FAT16::GetRootDirectorySize(struct VBR *VBR) {
    // In sector
    return ((VBR->RootDirectoryEntryCount*32)+(VBR->BytesPerSector-1))/VBR->BytesPerSector;
}

unsigned int FAT16::GetDataAreaLocation(struct VBR *VBR) {
    return FAT16::GetRootDirectoryLocation(VBR)+FAT16::GetRootDirectorySize(VBR);
}

// Return absolute size of directory(which means it returns size of directory "in bytes")
// Also writes cluster size of the directory
unsigned int FAT16::GetDirectoryInfo(Drivers::StorageSystem::Storage *Storage , unsigned int DirectorySectorAddress , unsigned int *DirectoryClusterSize) {
    int EntryCount = 0;
    int ClusterCount = 1;
    int Offset = 0;
    struct VBR VBR;
    GetVBR(Storage , &(VBR));
    int NextClusterAddress;
    unsigned char Directory[512];
    SFNEntry *Entry;
    NextClusterAddress = SectorToCluster(DirectorySectorAddress , &(VBR));
    Storage->Driver->ReadSectorFunction(Storage , DirectorySectorAddress , 1 , Directory);
    while(1) {
        Entry = (SFNEntry *)((unsigned long)(Directory+Offset));
        Offset += sizeof(SFNEntry);
        if(Entry->Attribute == 0) {
            break;
        }
        EntryCount++;
        if(Offset >= 512) {
            NextClusterAddress = FindNextCluster(Storage , NextClusterAddress , &(VBR));
            Offset = 0;
            Storage->Driver->ReadSectorFunction(Storage , ClusterToSector(NextClusterAddress , &(VBR)) , 1 , Directory);
            ClusterCount++;
        }
    }
    *DirectoryClusterSize = ClusterCount;
    return EntryCount;
}

unsigned int FAT16::ClusterToSector(unsigned int ClusterNumber , struct VBR *VBR) {
    return ((ClusterNumber-2)*VBR->SectorsPerCluster)+GetRootDirectoryLocation(VBR)+GetRootDirectorySize(VBR);
}

unsigned int FAT16::SectorToCluster(unsigned int SectorNumber , struct VBR *VBR) {
    return ((SectorNumber-(+GetRootDirectoryLocation(VBR)+GetRootDirectorySize(VBR)))/VBR->SectorsPerCluster)+2;
}

bool FAT16::GetVBR(Drivers::StorageSystem::Storage *Storage , struct VBR *VBR) {
    unsigned char *Sector = (unsigned char *)Kernel::MemoryManagement::Allocate(Storage->Geometry.BytesPerSector);
    if(Storage->Driver->ReadSectorFunction(Storage , 0 , 1 , Sector) != Storage->Geometry.BytesPerSector) {
        Kernel::MemoryManagement::Free(Sector);
        return false;
    }
    memcpy(VBR , Sector , sizeof(struct VBR));
    Kernel::MemoryManagement::Free(Sector);
    return true;
}

unsigned int FAT16::FindNextCluster(Drivers::StorageSystem::Storage *Storage , unsigned int Cluster , struct VBR *VBR) {
    int SectorAddress = (int)(Cluster/256)+VBR->ReservedSectorCount;
    unsigned char FATArea[512];
    
    Storage->Driver->ReadSectorFunction(Storage , SectorAddress , 1 , FATArea);
    return (FATArea[((Cluster%256)*2)])+(FATArea[((Cluster%256)*2)+1] << 8); 
}

// DirectoryAddress : Directory Location in Sector
// Return file location in cluster
bool FAT16::GetSFNEntry(Drivers::StorageSystem::Storage *Storage , unsigned int DirectoryAddress , const char *FileName , struct SFNEntry *Destination) {
    int i;
    int Offset = 0;
    int EntryCount;
    int LFNEntryCount;
    unsigned int DirectoryClusterSize;
    struct SFNEntry *SFNEntry;
    struct LFNEntry *LFNEntry;
    struct VBR VBR;
    unsigned char *Directory;
    EntryCount = GetDirectoryInfo(Storage , DirectoryAddress , &(DirectoryClusterSize));
    GetVBR(Storage , &(VBR));
    Directory = (unsigned char *)Kernel::MemoryManagement::Allocate(DirectoryClusterSize*VBR.SectorsPerCluster*VBR.BytesPerSector);
    ReadCluster(Storage , SectorToCluster(DirectoryAddress , &(VBR)) , DirectoryClusterSize , Directory , &(VBR));
    char TemporaryFileName[EntryCount*(5+6+2)];
    for(i = 0; i < EntryCount; i++) {
        SFNEntry = (struct SFNEntry *)(Directory+Offset);
        LFNEntry = (struct LFNEntry *)(Directory+Offset);
        if(LFNEntry->Attribute == 0x0F) {
            LFNEntryCount = LFNEntry->SequenceNumber^0x40;
            GetFileNameFromLFN(TemporaryFileName , LFNEntry);
            if(memcmp(FileName , TemporaryFileName , strlen(FileName)) == 0) {
                Offset += LFNEntryCount*sizeof(struct LFNEntry);
                memcpy(Destination , (struct SFNEntry *)(Directory+Offset) , sizeof(struct SFNEntry));
                return true;
            }
            memset(TemporaryFileName , 0 , strlen(TemporaryFileName));
            Offset += sizeof(struct LFNEntry)*LFNEntryCount;
        }
        else {
            Offset += sizeof(struct SFNEntry);
        }
    }
    return false;
}

// Returns 0 if entry is not LFN
// Returns number of entry and file name if it's LFN
int FAT16::GetFileNameFromLFN(char *FileName , struct LFNEntry *Entries) {
    int i;
    int j;
    int k = 0;
    unsigned short Character;
    int LFNEntryCount = Entries[0].SequenceNumber^0x40; // Number of entry = First entry sequence number^0x40
    for(i = LFNEntryCount-1; i >= 0; i--) {
        for(j = 0; j < 5; j++) {
            FileName[k++] = (Entries[i].FileName1[j]) & 0xFF;
        }
        for(j = 0; j < 6; j++) {
            FileName[k++] = (Entries[i].FileName2[j]) & 0xFF;
        }
        for(j = 0; j < 2; j++) {
            FileName[k++] = (Entries[i].FileName3[j]) & 0xFF;
        }
    }
    FileName[k] = 0x00;
    return LFNEntryCount;
}

// Get number of directory entered from file name
// Basically, you get the number of '/' in the string
int FAT16::GetDirectoryCount(const char *FileName) {
    int i;
    int DirectoryCount = 0;
    for(i = 0; i < strlen(FileName); i++) {
        if(FileName[i] == '/') {
            DirectoryCount++;
        }
    }
    return DirectoryCount;
}

int FAT16::ParseDirectories(const char *FileName , char **Directories) {
    // Auto-allocates
    int i;
    int j = 0;
    int PreviousPoint = 0;
    int DirectoryCount;
    if((DirectoryCount = GetDirectoryCount(FileName)) == 0) {
        return 0;
    }
    for(i = 0; i < strlen(FileName); i++) {
        if(FileName[i] == '/') {
            Directories[j] = (char *)Kernel::MemoryManagement::Allocate(i-PreviousPoint+1);
            strncpy(Directories[j] , FileName+PreviousPoint , i-PreviousPoint);
            Kernel::printf("Directories[%d] : %s\n" , j , Directories[j]);
            PreviousPoint = i+1;
            j++;
        }
    }
    Directories[j] = (char *)Kernel::MemoryManagement::Allocate(i-PreviousPoint+1);
    strncpy(Directories[j] , FileName+PreviousPoint , i-PreviousPoint);
    Kernel::printf("Directories[%d] : %s\n" , j , Directories[j]);
    return DirectoryCount;
}

void FAT16::WriteFileInfo(struct FileSystem::FileInfo *FileInfo , struct SFNEntry SFNEntry , const char *FileName , struct VBR *VBR) {
    FileInfo->BlockSize = 512;
    FileInfo->FileName = (char *)Kernel::MemoryManagement::Allocate(strlen(FileName));
    strcpy(FileInfo->FileName , FileName);
    FileInfo->FileSize = SFNEntry.FileSize;
    FileInfo->CreatedDate = SFNEntry.CreatedDate;
    FileInfo->CreatedTime = SFNEntry.CreateTime;
    FileInfo->LastAccessedDate = SFNEntry.LastAccessedDate;
    FileInfo->LastWrittenDate = SFNEntry.LastWrittenDate;
    FileInfo->LastWrittenTime = SFNEntry.LastWrittenTime;
    FileInfo->SectorAddress = ClusterToSector(((SFNEntry.StartingClusterHigh << 16)|SFNEntry.StartingClusterLow) , VBR);
    FileInfo->FileOffset = 0x00;
    switch(SFNEntry.Attribute) {
        case 0x01:
            FileInfo->FileType = FILESYSTEM_FILETYPE_READONLY;
            break;
        case 0x02:
            FileInfo->FileType = FILESYSTEM_FILETYPE_HIDDEN;
            break;
        case 0x04:
            FileInfo->FileType = FILESYSTEM_FILETYPE_SYSTEM;
            break;
        case 0x20:
            FileInfo->FileType = FILESYSTEM_FILETYPE_PRESENT;
            break;
    };
}
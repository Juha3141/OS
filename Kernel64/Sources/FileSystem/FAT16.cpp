#include <FileSystem/FAT16.hpp>

using namespace Kernel;

bool FAT16::Register(void) {
    struct FAT16::Driver *Driver = new FAT16::Driver;
    return FileSystem::Register(Driver , "FAT16");
}

bool FAT16::Driver::Check(struct Storage *Storage) {
    unsigned char *Buffer;
    struct VBR *BootSector;
    Buffer = (unsigned char *)Kernel::MemoryManagement::Allocate(Storage->PhysicalInfo.Geometry.BytesPerSector);
    if(Storage->Driver->ReadSector(Storage , 0 , 1 , Buffer) != Storage->PhysicalInfo.Geometry.BytesPerSector) {
        Kernel::printf("Failed reading\n");
        return false;
    }
    BootSector = (struct VBR *)Buffer;
    if(memcmp(BootSector->FileSystemType , "FAT16" , 5) == 0) {
        return true;
    }
    return false;
}

bool FAT16::Driver::CreateFile(struct Storage *Storage , const char *FileName) {
    return false;
}

bool FAT16::Driver::CreateDir(struct Storage *Storage , const char *DirectoryName) {
    return false;
}

// https://www.youtube.com/watch?v=qfumU7wLVd4

struct FileInfo *FAT16::Driver::OpenFile(struct Storage *Storage , const char *FileName) {
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
    FileInfo = (struct FileInfo *)Kernel::MemoryManagement::Allocate(sizeof(struct FileInfo));
    WriteFileInfo(FileInfo , SFNEntry , FileName , &(VBR));
    return FileInfo;
}

int FAT16::Driver::CloseFile(struct Storage *Storage , struct FileInfo *FileInfo) {
    return 0;
}

int FAT16::Driver::RemoveFile(struct Storage *Storage , struct FileInfo *FileInfo) {
    return 0;
}

int FAT16::Driver::WriteFile(struct Storage *Storage , struct FileInfo *FileInfo , unsigned long Size , void *Buffer) {
    return 0;
}

int FAT16::Driver::ReadFile(struct Storage *Storage , struct FileInfo *FileInfo , unsigned long Size , void *Buffer) {
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

int FAT16::Driver::ReadDirectory(struct Storage *Storage , struct FileInfo *FileInfo , struct FileInfo *FileList) {
    return 0;
}

int FAT16::Driver::GetFileCountInDirectory(struct Storage *Storage , struct FileInfo *FileInfo) {
    return 0;
}

int FAT16::Driver::WriteDirectoryData(struct Storage *Storage , struct FileInfo *FileInfo) {
    return 0;
}


void FAT16::WriteVBR(struct VBR *VBR , struct StorageGeometry *Geometry , const char *OEMID , const char *VolumeLabel , const char *FileSystem) {
    VBR->BytesPerSector = Geometry->BytesPerSector;
    if(Geometry->TotalSectorCount >= 65536) {
        VBR->TotalSector16 = 0;
        VBR->TotalSector32 = Geometry->TotalSectorCount;
    }
    else {
        VBR->TotalSector16 = Geometry->TotalSectorCount;
        VBR->TotalSector32 = 0;
    }
    VBR->MediaType = 0xF8;
    VBR->HiddenSectors = 0;
    VBR->JumpCode[0] = 0xEB;
    VBR->JumpCode[1] = 0x3C;
    VBR->JumpCode[2] = 0x90;
    memcpy(VBR->OEMID , OEMID , 8);
    memcpy(VBR->VolumeLabel , VolumeLabel , 11);
    memcpy(VBR->FileSystemType , FileSystem , 8);
    VBR->Reserved = 0;
    VBR->SerialNumber = 0x31415926;
    VBR->BootSignature = 0x27182848;
    VBR->SectorPerTrack = Geometry->SectorsPerTrack;
    // determine cluster size
    if(Geometry->TotalSectorCount < 32768) {
        VBR->SectorsPerCluster = 8;
    }
    if((Geometry->TotalSectorCount >= 32768) && (Geometry->TotalSectorCount < 262144)) {
        VBR->SectorsPerCluster = 4;
    }
    if((Geometry->TotalSectorCount >= 262144) && (Geometry->TotalSectorCount < 524288)) {
        VBR->SectorsPerCluster = 8;
    }
    if((Geometry->TotalSectorCount >= 524288) && (Geometry->TotalSectorCount < 1048576)) {
        VBR->SectorsPerCluster = 16;
    }
    if((Geometry->TotalSectorCount >= 1048576) && (Geometry->TotalSectorCount < 2097152)) {
        VBR->SectorsPerCluster = 32;
    }
    if((Geometry->TotalSectorCount >= 2097152) && (Geometry->TotalSectorCount <= 4294304)) {
        VBR->SectorsPerCluster = 64;
    }
    VBR->ReservedSectorCount = 1*VBR->SectorsPerCluster; // 1 if not fat32, 32 if fat32
    VBR->FATSize16 = (Geometry->TotalSectorCount/VBR->SectorsPerCluster)*sizeof(unsigned short)/Geometry->BytesPerSector;
    // determine root directory entry count
    // used some of code from :
    // https://github.com/Godzil/dosfstools/blob/master/src/mkdosfs.c, line 603
    switch(Geometry->TotalSectorCount) {
        case 720: // 5.25" - 360KB
            VBR->RootDirectoryEntryCount = 112;
            VBR->SectorsPerCluster = 2;
            VBR->MediaType = 0xFD;
            break;
        case 2400: // 5.25" - 1200KB
            VBR->RootDirectoryEntryCount = 224;
            VBR->SectorsPerCluster = 2;
            VBR->MediaType = 0xF9;
            break;
        case 1440: // 3.5" - 720KB
            VBR->RootDirectoryEntryCount = 112;
            VBR->SectorsPerCluster = 2;
            VBR->MediaType = 0xF9;
            break;
        case 5760: // 3.5" - 2880KB
            VBR->RootDirectoryEntryCount = 224;
            VBR->SectorsPerCluster = 2;
            VBR->MediaType = 0xF0;
            break;
        case 2880: // 3.5" - 1440KB
            VBR->RootDirectoryEntryCount = 224;
            VBR->SectorsPerCluster = 2;
            VBR->MediaType = 0xF0;
            break;
    }
    
    VBR->NumberOfFAT = 2;
    VBR->INT0x13DriveNumber = 0x00;
    
}

unsigned int FAT16::ReadCluster(struct Storage *Storage , unsigned long ClusterNumber , unsigned long ClusterCountToRead , unsigned char *Data , struct VBR *VBR) {
    unsigned long i;
    unsigned long NextClusterAddress = ClusterNumber;
    Kernel::printf("Reading Cluster, Cluster count to read : %d\n" , ClusterCountToRead);
    Kernel::printf("Cluster Number : %d\n" , ClusterNumber);
    Kernel::printf("Sector Address : %d\n" , ClusterToSector(ClusterNumber , VBR));
    for(i = 0; i < ClusterCountToRead*VBR->SectorsPerCluster; i++) {
        if(Storage->Driver->ReadSector(Storage , ClusterToSector(NextClusterAddress , VBR) , 1 , (Data+(i*VBR->BytesPerSector))) != Storage->PhysicalInfo.Geometry.BytesPerSector) {
            break;
        }
        if(ClusterCountToRead%VBR->SectorsPerCluster == 0) {
            NextClusterAddress = FindNextCluster(Storage , NextClusterAddress , VBR);
            if(NextClusterAddress == 0xFFFF) {
                break;
            }
        }
    }
    return i;
}

unsigned int FAT16::WriteCluster(struct Storage *Storage , unsigned long ClusterNumber , unsigned long ClusterCountToRead , unsigned char *Data , struct VBR *VBR) {
    unsigned long i;
    unsigned long NextClusterAddress = ClusterNumber;
    for(i = 0; i < ClusterCountToRead*VBR->SectorsPerCluster; i++) {
        if(Storage->Driver->WriteSector(Storage , ClusterToSector(NextClusterAddress , VBR) , 1 , (Data+(i*VBR->BytesPerSector))) != Storage->PhysicalInfo.Geometry.BytesPerSector) {
            break;
        }
        if(ClusterCountToRead%VBR->SectorsPerCluster == 0) {
            NextClusterAddress = FindNextCluster(Storage , NextClusterAddress , VBR);
            if(NextClusterAddress == 0xFFFF) {
                break;
            }
        }
    }
    return i;
}

unsigned int FAT16::FindFirstEmptyCluster(struct Storage *Storage) {
    int i;
    unsigned char Data[512];
    struct VBR VBR;
    Storage->Driver->ReadSector(Storage , 0 , 1 , Data);
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
unsigned int FAT16::GetDirectoryInfo(struct Storage *Storage , unsigned int DirectorySectorAddress , unsigned int *DirectoryClusterSize) {
    int EntryCount = 0;
    int ClusterCount = 1;
    int Offset = 0;
    struct VBR VBR;
    GetVBR(Storage , &(VBR));
    int NextClusterAddress;
    unsigned char Directory[512];
    SFNEntry *Entry;
    NextClusterAddress = SectorToCluster(DirectorySectorAddress , &(VBR));
    Storage->Driver->ReadSector(Storage , DirectorySectorAddress , 1 , Directory);
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
            Storage->Driver->ReadSector(Storage , ClusterToSector(NextClusterAddress , &(VBR)) , 1 , Directory);
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
    return ((SectorNumber-GetRootDirectoryLocation(VBR)-GetRootDirectorySize(VBR))/VBR->SectorsPerCluster)+2;
}

bool FAT16::GetVBR(struct Storage *Storage , struct VBR *VBR) {
    unsigned char *Sector = (unsigned char *)Kernel::MemoryManagement::Allocate(Storage->PhysicalInfo.Geometry.BytesPerSector);
    if(Storage->Driver->ReadSector(Storage , 0 , 1 , Sector) != Storage->PhysicalInfo.Geometry.BytesPerSector) {
        // Kernel::MemoryManagement::Free(Sector);
        return false;
    }
    memcpy(VBR , Sector , sizeof(struct VBR));
    // Kernel::MemoryManagement::Free(Sector);
    return true;
}

unsigned int FAT16::FindNextCluster(struct Storage *Storage , unsigned int Cluster , struct VBR *VBR) {
    int SectorAddress = (int)(Cluster/256)+VBR->ReservedSectorCount;
    unsigned char FATArea[512];
    
    Storage->Driver->ReadSector(Storage , SectorAddress , 1 , FATArea);
    return (FATArea[((Cluster%256)*2)])+(FATArea[((Cluster%256)*2)+1] << 8); 
}

void FAT16::WriteClusterInfo(struct Storage *Storage , unsigned int Cluster , unsigned short ClusterInfo , struct VBR *VBR) {
    int SectorAddress = (int)(Cluster/256)+VBR->ReservedSectorCount;
    unsigned char FATArea[512];
    
    Storage->Driver->ReadSector(Storage , SectorAddress , 1 , FATArea);
    FATArea[(Cluster%256)*2] = ClusterInfo & 0xFF;
    FATArea[((Cluster%256)*2)+1] = (ClusterInfo >> 8) & 0xFF;
    Storage->Driver->WriteSector(Storage , SectorAddress , 1 , FATArea);
    Storage->Driver->WriteSector(Storage , SectorAddress+VBR->FATSize16 , 1 , FATArea);
}

void FAT16::CreateSFNName(char *SFNName , const char *LFNName , int Number) {
    int i;
    int j;
    int DotIndex = 0;
    char *Buffer = (char *)Kernel::MemoryManagement::Allocate(strlen(LFNName));
    char NumberString[32];
    sprintf(NumberString , "%d" , Number);
    for(i = j = 0; i < strlen(LFNName); i++) {
        if(LFNName[i] != ' ') {
            Buffer[j++] = LFNName[i];
        }
    }
    Buffer[j] = 0x00;
    for(i = 0; i < strlen(Buffer); i++) {
        if(Buffer[i] == '.') {
            break;
        }
    }
    DotIndex = i;
    for(i = 0; Buffer[i] != 0; i++) {
        if((Buffer[i] >= 'a') && (Buffer[i] <= 'z')) {
            Buffer[i] = (Buffer[i]-'a')+'A';
        }
    }
    if(strlen(Buffer) > 8) {
        strncpy(SFNName , Buffer , 6);
        strcat(SFNName , "~");
        strcat(SFNName , NumberString);
    }
    else {
        strcpy(SFNName , Buffer);
        if(strlen(Buffer) < 8) {
            for(i = strlen(Buffer); i < 8; i++) {
                SFNName[i] = ' ';
            }
            SFNName[i] = 0x00;
        }
    }
    strcat(SFNName , Buffer+DotIndex+1);
    // Kernel::MemoryManagement::Free(Buffer);
}

/*
def CheckSum(string):
	sum = 0
	check = 0
	for i in range(0, len(string)):
		check = 0x80 if (check & 1) esle 0
		sum = check + (sum >> 1)
		sum = sum + ord(string[i])
		if sum >= 0x100:
			sum = sum - 0x100
		check = sum
	return sum

*/

unsigned char FAT16::GetSFNChecksum(const char *SFNName) {
    int i;
    unsigned char Sum = 0;
    int Check = 0;
    for(i = 0; i < strlen(SFNName); i++) {
        Check = (Check & 0x01) ? 0x80 : 0x00;
        Sum = Check+(Sum >> 1);
        Sum = Sum+SFNName[i];
        if(Sum >= 0x100) {
            Sum -= 0x100;
        }
        Check = Sum;
    }
    return Sum;
}

bool FAT16::WriteSFNEntry(struct Storage *Storage , unsigned int DirectoryAddress , struct SFNEntry *Entry) {
    unsigned char *Cluster;
    unsigned int ClusterAddress;
    unsigned int ClusterNumber;
    unsigned int DirectoryClusterSize;
    int DirectoryEntryCount;
    struct VBR VBR;

// sector for root directory
    unsigned int SectorAddress = DirectoryAddress;
    unsigned int SectorNumber;
    unsigned int DirectorySectorSize;
    GetVBR(Storage , &(VBR));

    DirectoryEntryCount = GetDirectoryInfo(Storage , DirectoryAddress , &(DirectoryClusterSize));
    // error
    Kernel::printf("Location to write : %d\n" , DirectoryAddress);
    Kernel::printf("VBR.BytesPerSector*VBR.SectorsPerCluster = %d\n" , VBR.BytesPerSector*VBR.SectorsPerCluster);
    
    Cluster = (unsigned char *)Kernel::MemoryManagement::Allocate(VBR.BytesPerSector*VBR.SectorsPerCluster);
    Kernel::printf("Cluster Buffer : 0x%X\n" , Cluster);
    // If root directory, take care of it differently. 
    
    if(DirectoryAddress == GetRootDirectoryLocation(&(VBR))) {
        SectorNumber = (DirectoryEntryCount*sizeof(struct SFNEntry))/VBR.BytesPerSector;
        
        Storage->Driver->ReadSector(Storage , SectorAddress+SectorNumber , 1 , Cluster);
        // this corrupted the data?
        memcpy((Cluster+((DirectoryEntryCount*sizeof(struct SFNEntry))%VBR.BytesPerSector)) , 
        Entry , sizeof(struct SFNEntry));
        Storage->Driver->WriteSector(Storage , SectorAddress+SectorNumber , 1 , Cluster);
        
    }
    else {
        ClusterAddress = SectorToCluster(DirectoryAddress , &(VBR));
        ClusterNumber = (DirectoryEntryCount*sizeof(struct SFNEntry))/(VBR.BytesPerSector*VBR.SectorsPerCluster);
        ReadCluster(Storage , ClusterAddress+ClusterNumber , 1 , Cluster , &(VBR));
        
        memcpy(&(Cluster[(DirectoryEntryCount*sizeof(struct SFNEntry))%(VBR.BytesPerSector*VBR.SectorsPerCluster)]) , 
        Entry , sizeof(struct SFNEntry));
        WriteCluster(Storage , ClusterAddress+ClusterNumber , 1 , Cluster , &(VBR));
    }
    // Kernel::MemoryManagement::Free(Cluster);
    return true;
}

/// @brief Automatically adds LFN Entry to directory, write LFN entry using given file name
/// @param Storage 
/// @param DirectoryAddress Sector Address of directory
/// @param FileName Name of file
/// @return Always return true
bool FAT16::WriteLFNEntry(struct Storage *Storage , unsigned int DirectoryAddress , const char *FileName) {
    int i;
    int j;
    int NameOffset = 0;
    int Checksum;
    int RequiredLFNEntry = (strlen(FileName)/13)+((strlen(FileName)%13 == 0) ? 0 : 1);
    struct LFNEntry *LFNEntry = (struct LFNEntry *)Kernel::MemoryManagement::Allocate((RequiredLFNEntry*sizeof(struct LFNEntry))+4096);
    char SFNName[12];

// cluster for other directory
    unsigned char *Cluster;
    unsigned int ClusterAddress;
    unsigned int ClusterNumber;
    unsigned int ClusterCount;
    unsigned int DirectoryClusterSize;
    int DirectoryEntryCount;
    struct VBR VBR;

// sector for root directory
    unsigned int SectorAddress = DirectoryAddress;
    unsigned int SectorNumber;
    unsigned int SectorCount;
    unsigned int DirectorySectorSize;

    GetVBR(Storage , &(VBR));
    memset(LFNEntry , 0 , RequiredLFNEntry*sizeof(struct LFNEntry));
    CreateSFNName(SFNName , FileName , 1); // To-do : number
    Checksum = GetSFNChecksum(SFNName);
    // Create LFN Entries
    Kernel::printf("SFN file name   : %s\n" , SFNName);
    Kernel::printf("LFN Entry count : %d\n" , RequiredLFNEntry);
    for(i = RequiredLFNEntry-1; i >= 0; i--) {
        LFNEntry[i].Attribute = 0x0F;
        LFNEntry[i].SequenceNumber = (RequiredLFNEntry-i)|(((i == 0) ? 0x40 : 0));
        LFNEntry[i].Checksum = Checksum; // To-do : Checksum is weird
        LFNEntry[i].Reserved = 0x00;
        LFNEntry[i].FirstClusterLow = 0;
        for(j = 0; j < 5; j++) { LFNEntry[i].FileName1[j] = ((NameOffset <= strlen(FileName)) ? ((unsigned short)FileName[NameOffset++]) : 0xFFFF); }
        for(j = 0; j < 6; j++) { LFNEntry[i].FileName2[j] = ((NameOffset <= strlen(FileName)) ? ((unsigned short)FileName[NameOffset++]) : 0xFFFF); }
        for(j = 0; j < 2; j++) { LFNEntry[i].FileName3[j] = ((NameOffset <= strlen(FileName)) ? ((unsigned short)FileName[NameOffset++]) : 0xFFFF); }
    }
    DirectoryEntryCount = GetDirectoryInfo(Storage , DirectoryAddress , &(DirectoryClusterSize));
    Kernel::printf("Directory Entry Count : %d\n" , DirectoryEntryCount);
    
    // If root directory, take care of it differently. 
    if(DirectoryAddress == GetRootDirectoryLocation(&(VBR))) {
        SectorNumber = (DirectoryEntryCount*sizeof(struct SFNEntry))/VBR.BytesPerSector;
        SectorCount = ((RequiredLFNEntry*sizeof(struct SFNEntry))/VBR.BytesPerSector)
                     +(((RequiredLFNEntry*sizeof(SFNEntry))%VBR.BytesPerSector == 0) ? 0 : 1);
        
        Storage->Driver->ReadSector(Storage , SectorNumber , SectorCount , Cluster);
        Kernel::printf("Sector Count : %d\n" , SectorCount);
        Cluster = (unsigned char *)Kernel::MemoryManagement::Allocate(VBR.BytesPerSector*SectorCount);
        Storage->Driver->ReadSector(Storage , SectorAddress+SectorNumber , SectorCount , Cluster);

        memcpy(&(Cluster[(DirectoryEntryCount*sizeof(struct SFNEntry))%(VBR.BytesPerSector)]) , 
        LFNEntry , sizeof(struct SFNEntry)*RequiredLFNEntry);
        Storage->Driver->WriteSector(Storage , SectorAddress+SectorNumber , SectorCount , Cluster);
    }
    else {
        ClusterAddress = SectorToCluster(DirectoryAddress , &(VBR));
        ClusterNumber = (DirectoryEntryCount*sizeof(struct SFNEntry))/(VBR.BytesPerSector*VBR.SectorsPerCluster);
        ClusterCount = ((RequiredLFNEntry*sizeof(struct SFNEntry))/(VBR.BytesPerSector*VBR.SectorsPerCluster))
                     +(((RequiredLFNEntry*sizeof(struct SFNEntry))%(VBR.BytesPerSector*VBR.SectorsPerCluster) == 0) ? 0 : 1);

        Cluster = (unsigned char *)Kernel::MemoryManagement::Allocate(VBR.SectorsPerCluster*VBR.BytesPerSector*ClusterCount);
        ReadCluster(Storage , ClusterNumber , ClusterCount , Cluster , &(VBR));
        
        memcpy(&(Cluster[(DirectoryEntryCount*sizeof(struct SFNEntry))%(VBR.BytesPerSector*VBR.SectorsPerCluster)]) , 
        LFNEntry , sizeof(struct SFNEntry)*RequiredLFNEntry);
        WriteCluster(Storage , ClusterNumber , ClusterCount , Cluster , &(VBR));
    }
    return true; 
}

bool FAT16::GetSFNEntry(struct Storage *Storage , unsigned int DirectoryAddress , const char *FileName , struct SFNEntry *Destination) {
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

void FAT16::WriteFileInfo(struct FileInfo *FileInfo , struct SFNEntry SFNEntry , const char *FileName , struct VBR *VBR) {
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
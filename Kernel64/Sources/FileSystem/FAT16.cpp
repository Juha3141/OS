#include <FileSystem/FAT16.hpp>

bool FAT16::Register(void) {
    struct FAT16::Driver *Driver = new FAT16::Driver;
    return FileSystem::Register(Driver , "FAT16");
}

bool FAT16::Driver::Check(struct Storage *Storage) {
    unsigned char *Buffer;
    struct VBR *BootSector;
    Buffer = (unsigned char *)MemoryManagement::Allocate(Storage->PhysicalInfo.Geometry.BytesPerSector);
    if(Storage->Driver->ReadSector(Storage , 0 , 1 , Buffer) != Storage->PhysicalInfo.Geometry.BytesPerSector) {
        printf("Failed reading\n");
        return false;
    }
    BootSector = (struct VBR *)Buffer;
    if(memcmp(BootSector->FileSystemType , "FAT16" , 5) == 0) {
        return true;
    }
    return false;
}

/// @brief 
/// @param Storage Pointer of the stoage
/// @param FileName Standalized file name (Full file name, including paths) 
/// @param InitialFileSize Initial size of the file
/// @return If successed, return true, if not, return false.
bool FAT16::Driver::CreateFile(struct Storage *Storage , const char *FileName) {
    int i;
    int j = 0;
    int k;
    int DotIndex = 0;
    int DirCountFromName = 0;
    unsigned char Data[512];
    char SFNName[13];
    char *LastName;
    struct VBR VBR;
    struct SFNEntry SFNEntry;
    unsigned int EmptyClusterLocation = FindFirstEmptyCluster(Storage); // Get the first usable cluster(Empty cluster)
    unsigned int DirectoryLocation;
    // Location of directory that's contains the file
    // Get VBR(First sector of the physical/logical storage)
    GetVBR(Storage , &(VBR));
    WriteClusterInfo(Storage , EmptyClusterLocation , 0xFFFF , &(VBR));
    // Initialize SFN Entry
    memset(&(SFNEntry) , 0 , sizeof(struct SFNEntry));
    // Create SFN Name (Ex : HELLOW~1.TXT)

    printf("FileName : %s\n" , FileName);
    LastName = (char *)MemoryManagement::Allocate(strlen(FileName)+1);
    k = strlen(FileName);
    for(i = 0; i < k-1; i++) {
        if(FileName[i] == '/') {
            DotIndex = i;
            break;
        }
    }
    for(i = strlen(FileName)-1; i >= 0; i--) {
        if(FileName[i] == '/') {
            break;
        }
        LastName[j++] = FileName[strlen(FileName)-i+DotIndex];
    }
    LastName[j] = 0;
    CreateSFNName(SFNName , LastName , 1);
    strncpy((char *)SFNEntry.FileName , SFNName , 11);
    // Write data to SFN Entry
    SFNEntry.FileSize = 0;     // Set file size to zero for now.
    SFNEntry.Attribute = 0x20; // normal file
    SFNEntry.StartingClusterLow = EmptyClusterLocation & 0xFFFF;
    SFNEntry.StartingClusterHigh = (EmptyClusterLocation << 16) & 0xFFFF;
    // To-do : Create universal clock for entire operating system
    // Get location of the directory that contains the file - which is get by name of the file
    DirectoryLocation = GetDirectoryLocation(Storage , FileName);
    
    // If the directory is not found, process error
    if(DirectoryLocation == 0xFFFFFFFF) {
        return false;
    }

    // Write file entry to directory address, LFN comes before SFN.
    printf("Directory Location -> : %d\n" , DirectoryLocation);
    WriteLFNEntry(Storage , DirectoryLocation , LastName);
    WriteSFNEntry(Storage , DirectoryLocation , &(SFNEntry));
    
    printf("Directory Location : %d\n" , GetRootDirectoryLocation(&(VBR)));
    printf("Allocated Cluster : %d\n" , EmptyClusterLocation);
    // Allocated sector address
    printf("Sector address : %d\n" , ClusterToSector(EmptyClusterLocation , &(VBR)));
    printf("Cluster address : 0x%X%X\n" , SFNEntry.StartingClusterHigh , SFNEntry.StartingClusterLow);
    
    return true;
}

bool FAT16::Driver::CreateDir(struct Storage *Storage , const char *DirectoryName) {
    int i;
    int DirCountFromName = 0;
    unsigned char Data[512];
    char SFNName[13];
    struct VBR VBR;
    struct SFNEntry SFNEntry;
    unsigned int EmptyClusterLocation = FindFirstEmptyCluster(Storage); // Get the first usable cluster(Empty cluster)
    unsigned int DirectoryLocation;
    // Location of directory that's contains the file
    // Get VBR(First sector of the physical/logical storage)
    GetVBR(Storage , &(VBR));
    WriteClusterInfo(Storage , EmptyClusterLocation , 0xFFFF , &(VBR));
    // Initialize SFN Entry
    memset(&(SFNEntry) , 0 , sizeof(struct SFNEntry));
    // Create SFN Name (Ex : HELLOW~1.TXT)
    CreateSFNName(SFNName , DirectoryName , 1);
    memcpy(SFNEntry.FileName , SFNName , 12);

    // Write data to SFN Entry
    SFNEntry.FileSize = 0;     // Set file size to zero for now.
    SFNEntry.Attribute = 0x10; // normal file
    SFNEntry.StartingClusterLow = EmptyClusterLocation & 0xFFFF;
    SFNEntry.StartingClusterHigh = (EmptyClusterLocation << 16) & 0xFFFF;
    // To-do : Create universal clock for entire operating system
    // Get location of the directory that contains the file - which is get by name of the file
    DirectoryLocation = GetDirectoryLocation(Storage , DirectoryName);
    
    // If the directory is not found, process error
    if(DirectoryLocation == 0xFFFFFFFF) {
        return false;
    }

    // Write file entry to directory address, LFN comes before SFN.
    WriteLFNEntry(Storage , DirectoryLocation , DirectoryName);
    WriteSFNEntry(Storage , DirectoryLocation , &(SFNEntry));
    SFNEntry.Attribute = 0x10;
    memcpy(SFNEntry.FileName , ".          " , 11);
    SFNEntry.FileSize = 0;
    SFNEntry.LastAccessedDate = 0;
    SFNEntry.LastWrittenDate = 0;
    SFNEntry.LastWrittenTime = 0;
    SFNEntry.CreatedDate = 0;
    SFNEntry.CreateTime = 0;
    SFNEntry.Reserved = 0;
    WriteSFNEntry(Storage , ClusterToSector(EmptyClusterLocation , &(VBR)) , &(SFNEntry));
    memset(&(SFNEntry) , 0 , sizeof(struct SFNEntry));
    SFNEntry.Attribute = 0x10;
    memcpy(SFNEntry.FileName , "..         " , 11);
    SFNEntry.LastAccessedDate = 0;
    SFNEntry.LastWrittenDate = 0;
    SFNEntry.LastWrittenTime = 0;
    SFNEntry.CreatedDate = 0;
    SFNEntry.CreateTime = 0;
    if(DirectoryLocation != GetRootDirectoryLocation(&(VBR))) {
        SFNEntry.StartingClusterLow = SectorToCluster(DirectoryLocation , &(VBR)) & 0xFFFF;
        SFNEntry.StartingClusterHigh = (SectorToCluster(DirectoryLocation , &(VBR)) << 16) & 0xFFFF;
    }
    WriteSFNEntry(Storage , ClusterToSector(EmptyClusterLocation , &(VBR)) , &(SFNEntry));
    return false;
}

// Watch this if you're sleepy in 3 am lol..
// https://www.youtube.com/watch?v=qfumU7wLVd4

struct FileInfo *FAT16::Driver::OpenFile(struct Storage *Storage , const char *FileName , int OpenOption) {
    int i = 0;
    int j = 0;
    int DirectoryCount;
    int DirectoryLocation;
    int FileClusterLocation;
    char **Directories;
    struct VBR VBR;
    struct SFNEntry SFNEntry;
    struct FileInfo *FileInfo;
    GetVBR(Storage , &(VBR));
    DirectoryCount = GetDirectoryCount(FileName);
    DirectoryLocation = GetRootDirectoryLocation(&(VBR)); // Default directory location(Root Directory)
    // If directory count is zero, set first directory as the file name
    if(DirectoryCount == 0) {
        if(GetSFNEntry(Storage , DirectoryLocation , FileName , &(SFNEntry)) == false) {
            return 0x00;
        }
        FileClusterLocation = (SFNEntry.StartingClusterHigh << 16)|SFNEntry.StartingClusterLow;
        FileInfo = (struct FileInfo *)MemoryManagement::Allocate(sizeof(struct FileInfo));
        WriteFileInfo(FileInfo , SFNEntry , FileName , DirectoryLocation , &(VBR) , Storage , OpenOption);
        return FileInfo;
    }
    else {
        Directories = (char **)MemoryManagement::Allocate((DirectoryCount+1)*sizeof(char *)); 
        // Parse directory(Get list of directories) from file name
        ParseDirectories(FileName , Directories);
        for(i = 0; i < DirectoryCount; i++) {
            GetSFNEntry(Storage , DirectoryLocation , Directories[i] , &(SFNEntry));
            FileClusterLocation = (SFNEntry.StartingClusterHigh << 16)|SFNEntry.StartingClusterLow;
            DirectoryLocation = ClusterToSector(FileClusterLocation , &(VBR));
            // Go to the directories, find the directory
        }
    }
    // If there's no file in the directory -> Error(No SFN entry)
    if(GetSFNEntry(Storage , DirectoryLocation , Directories[i] , &(SFNEntry)) == false) {
        for(j = 0; j < DirectoryCount; j++) {
            MemoryManagement::Free(Directories[j]);
        }
        MemoryManagement::Free(Directories);
        return 0x00;
    }
    for(j = 0; j < DirectoryCount; j++) {
        MemoryManagement::Free(Directories[j]);
    }
    MemoryManagement::Free(Directories);
    FileClusterLocation = (SFNEntry.StartingClusterHigh << 16)|SFNEntry.StartingClusterLow;
    FileInfo = (struct FileInfo *)MemoryManagement::Allocate(sizeof(struct FileInfo));
    WriteFileInfo(FileInfo , SFNEntry , FileName , DirectoryLocation , &(VBR) , Storage , OpenOption);
    
    return FileInfo;
}

int FAT16::Driver::CloseFile(struct FileInfo *FileInfo) {
    return 0;
}

int FAT16::Driver::RemoveFile(struct FileInfo *FileInfo) {
    return 0;
}

// What I'm thinking is that we actually don't need thoses garbage stuffs...
// We have to make it more modulized - Throw WriteOption ...
// Make it more modulized    
int FAT16::Driver::WriteFile(struct FileInfo *FileInfo , unsigned long Size , void *Buffer) {
    int i = 0;
    int j = 0;
    int DotIndex;
    int k;
    unsigned int CurrentCluster; // Final Cluster of the file
    unsigned short NextCluster;
    unsigned short StartingCluster = 0;
    unsigned short StartingClusterIndex = 0;
    unsigned short ClusterCount = 0;
    unsigned short FileClusterSize;
    unsigned char *Data;
    char *LastName;
    char SFNName[12];
    struct SFNEntry NewSFNEntry;

    unsigned int DirectoryLocation;
    struct VBR VBR;
    if(FileInfo->OpenOption == FILESYSTEM_OPEN_READ) {
        return 0;
    }
    GetVBR(FileInfo->Storage , &(VBR));
    StartingClusterIndex = (FileInfo->FileOffset/(VBR.BytesPerSector*VBR.SectorsPerCluster));
    ClusterCount = ((FileInfo->FileOffset+Size)/(VBR.BytesPerSector*VBR.SectorsPerCluster))+((FileInfo->FileOffset+Size)%(VBR.BytesPerSector*VBR.SectorsPerCluster) != 0);
    FileClusterSize = (FileInfo->FileSize/(VBR.BytesPerSector*VBR.SectorsPerCluster))+(FileInfo->FileSize%(VBR.BytesPerSector*VBR.SectorsPerCluster) != 0);
    if(FileInfo->FileSize == 0) {
        FileClusterSize = 1;
    }
    CurrentCluster = SectorToCluster(FileInfo->Location , &(VBR));
    printf("Read %d clusters, from cluster #%d\n" , ClusterCount , StartingClusterIndex);
    printf("File address : %d cluster(%d sector)\n" , CurrentCluster , FileInfo->Location);
    while(1) {
        if(i == StartingClusterIndex) {
            StartingCluster = CurrentCluster;
        }
        NextCluster = FAT16::FindNextCluster(FileInfo->Storage , CurrentCluster , &(VBR));
        printf("NextCluster : %d\n" , NextCluster);
        if(NextCluster == 0xFFFF) {
            break;
        }
        CurrentCluster = NextCluster;
        i++;
    }
    Data = (unsigned char *)MemoryManagement::Allocate(ClusterCount*VBR.BytesPerSector*VBR.SectorsPerCluster);
    ReadCluster(FileInfo->Storage , StartingCluster , ClusterCount , Data , &(VBR));
    if(FileClusterSize*VBR.BytesPerSector*VBR.SectorsPerCluster < FileInfo->FileOffset+Size) {
        printf("Need to create new clusters\n");
        ExtendCluster(FileInfo->Storage , CurrentCluster , StartingCluster+ClusterCount-FileClusterSize , &(VBR));
        // Bug : I think Data overwrites some part or something..? idk
        printf("Extended cluster\n");
        printf("New cluster count : %d\n" , ClusterCount);
        printf("StartingClusterIndex : %d\n" , StartingCluster);
    }
    printf("%d\n" , (FileInfo->FileOffset-(StartingClusterIndex*VBR.BytesPerSector*VBR.SectorsPerCluster)));
    memcpy(Data+((FileInfo->FileOffset-(StartingClusterIndex*VBR.BytesPerSector*VBR.SectorsPerCluster))) , Buffer , Size);
    WriteCluster(FileInfo->Storage , StartingCluster , ClusterCount , Data , &(VBR));
    if(FileInfo->FileSize < FileInfo->FileOffset+Size) {
        FileInfo->FileSize = FileInfo->FileOffset+Size;
    }
    FileInfo->FileOffset += Size;
    LastName = (char *)MemoryManagement::Allocate(strlen(FileInfo->FileName)+1);
    k = strlen(FileInfo->FileName);
    for(i = 0; i < k-1; i++) {
        if(FileInfo->FileName[i] == '/') {
            DotIndex = i;
            break;
        }
    }
    for(i = strlen(FileInfo->FileName)-1; i >= 0; i--) {
        if(FileInfo->FileName[i] == '/') {
            break;
        }
        LastName[j++] = FileInfo->FileName[strlen(FileInfo->FileName)-i+DotIndex];
    }
    LastName[j] = 0;
    DirectoryLocation = GetDirectoryLocation(FileInfo->Storage , FileInfo->FileName);
    if(GetSFNEntry(FileInfo->Storage , DirectoryLocation , LastName , &(NewSFNEntry)) == false) {
        MemoryManagement::Free(LastName);
        MemoryManagement::Free(Data);
    }
    NewSFNEntry.FileSize = FileInfo->FileSize;
    CreateSFNName(SFNName , LastName , 1);
    if(RewriteSFNEntry(FileInfo->Storage , DirectoryLocation , SFNName , &(NewSFNEntry)) == false) {
        printf("Failed :(\n");
    }

    MemoryManagement::Free(LastName);
    MemoryManagement::Free(Data);
    return Size;
}

int FAT16::Driver::ReadFile(struct FileInfo *FileInfo , unsigned long Size , void *Buffer) {
    struct VBR VBR;
    unsigned int ReadSize;
    unsigned int SectorAddress = FileInfo->Location+(FileInfo->FileOffset/FileInfo->BlockSize);
    unsigned int SectorCountToRead = Size/FileInfo->BlockSize+(((Size%FileInfo->BlockSize) == 0) ? 0 : 1);
    unsigned char *TempBuffer = (unsigned char *)MemoryManagement::Allocate(SectorCountToRead*FileInfo->BlockSize);
    if(FileInfo->Storage == 0x00) {
        return 0;
    }
    GetVBR(FileInfo->Storage , &(VBR));
    if((ReadSize = ReadCluster(FileInfo->Storage , SectorToCluster(SectorAddress , &(VBR)) , SectorCountToRead*VBR.SectorsPerCluster , TempBuffer , &(VBR))) == (SectorCountToRead*FileInfo->BlockSize)) {
        memcpy(Buffer , TempBuffer , ReadSize);
        FileInfo->FileOffset += ReadSize;
        MemoryManagement::Free(TempBuffer);
        return ReadSize;
    }
    FileInfo->FileOffset += Size;
    memcpy(Buffer , TempBuffer , Size);
    MemoryManagement::Free(TempBuffer);
    return Size;
}

int FAT16::Driver::ReadDirectory(struct FileInfo *FileInfo , struct FileInfo *FileList) {
    return 0;
}

int FAT16::Driver::GetFileCountInDirectory(struct FileInfo *FileInfo) {
    return 0;
}

int FAT16::Driver::WriteDirectoryData(struct FileInfo *FileInfo) {
    return 0;
}


void FAT16::WriteVBR(struct VBR *VBR , struct Storage *Storage , const char *OEMID , const char *VolumeLabel , const char *FileSystem) {
    unsigned int TotalSectorCount = ((Storage->Type == Storage::StorageType::Logical) ? (Storage->LogicalPartitionInfo.EndAddressLBA-Storage->LogicalPartitionInfo.StartAddressLBA) : Storage->PhysicalInfo.Geometry.TotalSectorCount);
    unsigned int RootDirectorySectorCount;
    VBR->BytesPerSector = Storage->PhysicalInfo.Geometry.BytesPerSector;
    if(TotalSectorCount > 65535) {
        VBR->TotalSector16 = 0;
        VBR->TotalSector32 = TotalSectorCount;
    }
    else {
        VBR->TotalSector16 = (unsigned short)TotalSectorCount;
        VBR->TotalSector32 = 0;
    }
    VBR->NumberOfFAT = 2;
    VBR->MediaType = 0xF8; // Fixed : 0xF8 , Movable : 0xF0
    VBR->HiddenSectors = 0; // Hidden Sectors : 0
    VBR->JumpCode[0] = 0xEB;
    VBR->JumpCode[1] = 0x3C;
    VBR->JumpCode[2] = 0x90;
    memcpy(VBR->OEMID , OEMID , 8);
    memcpy(VBR->VolumeLabel , VolumeLabel , 11);
    memcpy(VBR->FileSystemType , FileSystem , 8);
    VBR->Reserved = 0;
    VBR->SerialNumber = 0x31415926;
    VBR->BootSignature = 0x27;
    VBR->SectorPerTrack = Storage->PhysicalInfo.Geometry.CHS_Sectors;
    VBR->NumberOfHeads = Storage->PhysicalInfo.Geometry.CHS_Heads;
    // determine cluster size
    VBR->SectorsPerCluster = 0;
    printf("TotalSectorCount : %d\n" , TotalSectorCount);
    if(TotalSectorCount < 32768) { // 7MB~16MB : 2KB
        VBR->SectorsPerCluster = 4;
    }
    if((TotalSectorCount >= 32768) && (TotalSectorCount < 65536)) { // 17MB~32MB : 512B
        VBR->SectorsPerCluster = 1;
    }
    if((TotalSectorCount >= 65536) && (TotalSectorCount < 131072)) { // 33MB~64MB : 1KB
        VBR->SectorsPerCluster = 2;
    }
    if((TotalSectorCount >= 131072) && (TotalSectorCount < 262144)) { // 65~128MB : 2KB
        VBR->SectorsPerCluster = 4;
    }
    if((TotalSectorCount >= 262144) && (TotalSectorCount < 524288)) { // 129MB~256MB : 4KB
        VBR->SectorsPerCluster = 8;
    }
    if((TotalSectorCount >= 524288) && (TotalSectorCount < 1048576)) { // 257~512MB : 8KB
        VBR->SectorsPerCluster = 16;
    }
    if((TotalSectorCount >= 1048576) && (TotalSectorCount < 2097152)) { // 513~1024MB : 16KB
        VBR->SectorsPerCluster = 32;
    }
    if((TotalSectorCount >= 2097152) && (TotalSectorCount < 4194304)) { // 1025~2048MB : 32KB
        VBR->SectorsPerCluster = 64;
    }
    if((TotalSectorCount >= 4194304) && (TotalSectorCount < 8388608)) { // 2049~4096MB : 64KB
        VBR->SectorsPerCluster = 128;
    }
    VBR->ReservedSectorCount = 1; // temporarily set to 1
    // determine root directory entry count
    // used some of code from :
    // https://github.com/Godzil/dosfstools/blob/master/src/mkdosfs.c, line 603
    switch(TotalSectorCount) {
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
        default:
            VBR->RootDirectoryEntryCount = 512; // Root directory entry size : 32 sectors
            break;
        // If I remove this the code works fine.. in WINDOWS.
    }
    // VBR->FATSize16 = ((TotalSectorCount/VBR->SectorsPerCluster)*sizeof(unsigned short)/Storage->PhysicalInfo.Geometry.BytesPerSector)-1;
    RootDirectorySectorCount = ((VBR->RootDirectoryEntryCount*32)+(VBR->BytesPerSector-1))/VBR->BytesPerSector;
    unsigned int T1 = TotalSectorCount-(VBR->ReservedSectorCount+RootDirectorySectorCount);
    unsigned int T2 = (256*VBR->SectorsPerCluster)+VBR->NumberOfFAT;
    VBR->FATSize16 = (T1+(T2-1))/T2;
    printf("FATSize16                : %d\n" , VBR->FATSize16);
    printf("Root Directory Location : %d\n" , VBR->ReservedSectorCount+(VBR->FATSize16*VBR->NumberOfFAT));
    printf("Root Directory Size     : %d\n" , ((((VBR->RootDirectoryEntryCount*32)+(VBR->BytesPerSector))/VBR->BytesPerSector)-1));
}

unsigned int FAT16::ReadCluster(struct Storage *Storage , unsigned long ClusterNumber , unsigned long ClusterCountToRead , unsigned char *Data , struct VBR *VBR) {
    unsigned long i;
    unsigned long NextClusterAddress = ClusterNumber;
    printf("Reading Cluster, Cluster count to read : %d\n" , ClusterCountToRead);
    printf("Cluster Number : %d\n" , ClusterNumber);
    printf("Sector Address : %d\n" , ClusterToSector(ClusterNumber , VBR));
    for(i = 0; i < ClusterCountToRead; i++) {
        if(Storage->Driver->ReadSector(Storage , ClusterToSector(NextClusterAddress , VBR) , VBR->SectorsPerCluster , (Data+(i*VBR->SectorsPerCluster*VBR->BytesPerSector))) != VBR->SectorsPerCluster*VBR->BytesPerSector) {
            break;
        }
        NextClusterAddress = FindNextCluster(Storage , NextClusterAddress , VBR);
        if(NextClusterAddress == 0xFFFF) {
            break;
        }
    }
    return i;
}

unsigned int FAT16::WriteCluster(struct Storage *Storage , unsigned long ClusterNumber , unsigned long ClusterCountToRead , unsigned char *Data , struct VBR *VBR) {
    unsigned long i;
    unsigned long NextClusterAddress = ClusterNumber;
    for(i = 0; i < ClusterCountToRead; i++) {
        printf("CurrentIndex : 0x%X\n" , (Data+(i*VBR->SectorsPerCluster*VBR->BytesPerSector)));
        if(Storage->Driver->WriteSector(Storage , ClusterToSector(NextClusterAddress , VBR) , VBR->SectorsPerCluster , (Data+(i*VBR->SectorsPerCluster*VBR->BytesPerSector))) != VBR->SectorsPerCluster*VBR->BytesPerSector) {
            break;
        }
        printf("I : %d , NextClusterAddress : %d (%dsector)\n" , i , NextClusterAddress , ClusterToSector(NextClusterAddress , VBR));
        NextClusterAddress = FindNextCluster(Storage , NextClusterAddress , VBR);
        if(NextClusterAddress == 0xFFFF) {
            break;
        }
    }
    return i;
}

unsigned int FAT16::FindFirstEmptyCluster(struct Storage *Storage) {
    int i;
    struct VBR VBR;
    GetVBR(Storage , &(VBR));
    for(i = 3; i < VBR.FATSize16/VBR.SectorsPerCluster; i++) { // starting from Cluster #3
        if(FindNextCluster(Storage , i , &(VBR)) == 0x00) {
            return i;
        }
    }
    return 0xFFFFFFFF;
}

unsigned int FAT16::GetFATAreaLocation(struct VBR *VBR) {
    return VBR->ReservedSectorCount;
}

/// @brief Get sector number of the root directory
/// @param VBR VBR of the sector
/// @return Sector number of the root directory
unsigned int FAT16::GetRootDirectoryLocation(struct VBR *VBR) {
    return VBR->ReservedSectorCount+(VBR->FATSize16*VBR->NumberOfFAT);
}

/// @brief Get size of the root directory "in sector"
/// @param VBR VBR of the storage
/// @return Sector size of the root directory
unsigned int FAT16::GetRootDirectorySize(struct VBR *VBR) {
    return ((((VBR->RootDirectoryEntryCount*32))/VBR->BytesPerSector));
}

/// @brief Get sector number of the data area
/// @param VBR VBR of the storage
/// @return Sector number of the data area
unsigned int FAT16::GetDataAreaLocation(struct VBR *VBR) {
    return FAT16::GetRootDirectoryLocation(VBR)+FAT16::GetRootDirectorySize(VBR);
}

// Return absolute size of directory(which means it returns size of directory "in bytes")
// Also writes cluster size of the directory
// -> ERROR

/// @brief Provide the basic information of a directory, specified by DirectorySectorAddress
/// @param Storage Targetted storage
/// @param DirectorySectorAddress Sector address of the directory
/// @param DirectoryClusterSize (Return) the cluster size of directory
/// @return Number of entries that the directory has
unsigned int FAT16::GetDirectoryInfo(struct Storage *Storage , unsigned int DirectorySectorAddress , unsigned int *DirectoryClusterSize) {
    int EntryCount = 0;
    int ClusterCount = 1;
    int Offset = 0;
    struct VBR VBR;
    GetVBR(Storage , &(VBR)); // (this causes somehow modify LFNEntry[0] (in line 550~556).. I can't figure it why....)
    int NextSectorAddress = DirectorySectorAddress;
    unsigned char *Directory;
    SFNEntry *Entry;
    if(DirectorySectorAddress == GetRootDirectoryLocation(&(VBR))) {
        *DirectoryClusterSize = GetRootDirectorySize(&(VBR))/VBR.SectorsPerCluster;
        Directory = (unsigned char *)MemoryManagement::Allocate(GetRootDirectorySize(&(VBR))*VBR.BytesPerSector);
        
        Storage->Driver->ReadSector(Storage , DirectorySectorAddress , GetRootDirectorySize(&(VBR)) , Directory);
        
        while(1) {
            if(Offset >= (GetRootDirectorySize(&(VBR))*VBR.BytesPerSector)) {
                break;
            }
            Entry = (SFNEntry *)((unsigned long)(Directory+Offset));
            if(Entry->Attribute == 0) {
                break;
            }
            EntryCount++;
            Offset += sizeof(SFNEntry);
        }
        MemoryManagement::Free(Directory);
        return EntryCount;
    }
    // Problem : Faulty memory allocation system, it seems it allocates pre-allocated areas that still are using by other things..
    Directory = (unsigned char *)MemoryManagement::Allocate(VBR.SectorsPerCluster*VBR.BytesPerSector);
    //---------------------------- !!! vptr corruption !!! ----------------------------//
    
    // First off, how do I treat root directory??
    printf("DirectorySectorAddress : %d\n" , DirectorySectorAddress);
    printf("Directory : 0x%X~0x%X\n" , Directory , Directory+(VBR.SectorsPerCluster*VBR.BytesPerSector));
    Storage->Driver->ReadSector(Storage , DirectorySectorAddress , VBR.SectorsPerCluster , Directory);
    
    while(1) {
        Entry = (SFNEntry *)((unsigned long)(Directory+Offset));
        if(Entry->Attribute == 0) {
            break;
        }
        EntryCount++;
        Offset += sizeof(SFNEntry);
        if(Offset >= VBR.BytesPerSector) {
            NextSectorAddress++;
        }
        if(NextSectorAddress-DirectorySectorAddress >= VBR.SectorsPerCluster) {
            DirectorySectorAddress = FindNextCluster(Storage , SectorToCluster(NextSectorAddress , &(VBR)) , &(VBR));
            Offset = 0;
            Storage->Driver->ReadSector(Storage , NextSectorAddress , 4 , Directory);
            ClusterCount++;
        }
    }
    *DirectoryClusterSize = ClusterCount;
    MemoryManagement::Free(Directory);
    return EntryCount;
}


unsigned int FAT16::ClusterToSector(unsigned int ClusterNumber , struct VBR *VBR) {
    return ((ClusterNumber-2)*VBR->SectorsPerCluster)+GetRootDirectoryLocation(VBR)+GetRootDirectorySize(VBR);
}

unsigned int FAT16::SectorToCluster(unsigned int SectorNumber , struct VBR *VBR) {
    return ((SectorNumber-GetRootDirectoryLocation(VBR)-GetRootDirectorySize(VBR))/VBR->SectorsPerCluster)+2;
}

bool FAT16::GetVBR(struct Storage *Storage , struct VBR *VBR) { // Faulty GetVBR or.. bad usage of it???
    int i;
    unsigned char BootRecord[512];
    memset(BootRecord , 0 , 512);
    Storage->Driver->ReadSector(Storage , 0 , 1 , BootRecord);
    if(BootRecord[0] == 0) {
        while(BootRecord[0] == 0) {
            Storage->Driver->ReadSector(Storage , 0 , 1 , BootRecord);
        }
    }
    memcpy(VBR , BootRecord , sizeof(struct VBR));
    return true;
}

unsigned int FAT16::FindNextCluster(struct Storage *Storage , unsigned int Cluster , struct VBR *VBR) {
    int SectorAddress = (int)(Cluster/256)+VBR->ReservedSectorCount;
    unsigned char FATArea[512];
    Storage->Driver->ReadSector(Storage , SectorAddress , 1 , FATArea);
    return (FATArea[((Cluster%256)*2)])+(FATArea[((Cluster%256)*2)+1] << 8);
}

void FAT16::WriteClusterInfo(struct Storage *Storage , unsigned int Cluster , unsigned short ClusterInfo , struct VBR *VBR) {
    unsigned int SectorAddress = (unsigned int)((Cluster/256)+VBR->ReservedSectorCount);
    unsigned short *FATArea;
    FATArea = (unsigned short *)MemoryManagement::Allocate(512);
    Storage->Driver->ReadSector(Storage , SectorAddress , 1 , FATArea);
    FATArea[(Cluster%256)] = ClusterInfo;
    Storage->Driver->WriteSector(Storage , SectorAddress , 1 , FATArea);
    Storage->Driver->WriteSector(Storage , SectorAddress+VBR->FATSize16 , 1 , FATArea);
    MemoryManagement::Free(FATArea);
}

void FAT16::ExtendCluster(struct Storage *Storage , unsigned int EndCluster , unsigned int ClusterCount , struct VBR *VBR) {
    int i;
    unsigned int CurrentCluster = EndCluster;
    unsigned int NextCluster;
    for(i = 0; i < ClusterCount-1; i++) {
        NextCluster = FindFirstEmptyCluster(Storage);
        WriteClusterInfo(Storage , CurrentCluster , NextCluster , VBR);
        CurrentCluster = NextCluster;
    }
    WriteClusterInfo(Storage , CurrentCluster , 0xFFFF , VBR);
}

/// @brief Create 8.3 file name of original name
/// @param SFNName Output 8.3 name
/// @param LFNName original long file name
/// @param Number Number of the name
void FAT16::CreateSFNName(char *SFNName , const char *LFNName , int Number) {
    int i;
    int j;
    int DotIndex = strlen(LFNName)-1;
    char *Buffer;
    char NumberString[32];
    // Get index of dot
    if(strlen(LFNName) == 0) {
        return;
    }
    Buffer = (char *)MemoryManagement::Allocate(strlen(LFNName));
    memset(SFNName , ' ' , 11);
    for(i = j = 0; i < strlen(LFNName); i++) {
        if(LFNName[i] != ' ') {
            Buffer[j++] = LFNName[i]; // Remove space
        }
    }
    Buffer[j] = 0x00;
    for(i = 0; Buffer[i] != 0; i++) {
        if(Buffer[i] == '.') {
            DotIndex = i;
        }
        if((Buffer[i] >= 'a') && (Buffer[i] <= 'z')) {
            Buffer[i] = (Buffer[i]-'a')+'A';
        }
    }
    if(DotIndex > 8) {
        for(i = 0; i < 6; i++) {
            SFNName[i] = Buffer[i];
        }
        SFNName[6] = 0x00;
        sprintf(NumberString , "%d" , Number);
        strcat(SFNName , "~");
        strcat(SFNName , NumberString);
    }
    else {
        for(i = 0; i < DotIndex; i++) {
            SFNName[i] = Buffer[i];
        }
        SFNName[8] = 0x00;
    }
    if(DotIndex != strlen(LFNName)-1) {
        strcat(SFNName , Buffer+DotIndex+1);
    }
    else {
        for(i = 0; i < 13-strlen(SFNName); i++) {
            strcat(SFNName , " ");
        }
    }
    MemoryManagement::Free(Buffer);
}

/// @brief Create SFN name for Volume Entry
/// @param SFNName Output 8.3 name
/// @param LFNName original long file name
void FAT16::CreateVolumeLabelName(char *SFNName , const char *LFNName) {
    int i;
    int j;
    char *Buffer;
    if(strlen(LFNName) == 0) {
        return;
    }
    Buffer = (char *)MemoryManagement::Allocate(strlen(LFNName));
    memset(SFNName , ' ' , 11);
    for(i = j = 0; i < strlen(LFNName); i++) {
        if(LFNName[i] != ' ') {
            Buffer[j++] = LFNName[i]; // Remove space
        }
    }
    Buffer[j] = 0x00;
    for(i = 0; Buffer[i] != 0; i++) {
        if((Buffer[i] >= 'a') && (Buffer[i] <= 'z')) {
            Buffer[i] = (Buffer[i]-'a')+'A';
        }
    }
    memcpy(SFNName , Buffer , ((strlen(LFNName) >= 11) ? 11 : strlen(LFNName)));
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
    printf("DirectoryEntryCount : %d\n" , DirectoryEntryCount);
    // error
    printf("Location to write : %d\n" , DirectoryAddress);
    
    Cluster = (unsigned char *)MemoryManagement::Allocate(VBR.BytesPerSector*VBR.SectorsPerCluster);

    // If root directory, take care of it differently. 
    if(DirectoryAddress == GetRootDirectoryLocation(&(VBR))) {
        SectorNumber = (DirectoryEntryCount*sizeof(struct SFNEntry))/VBR.BytesPerSector;
        
        // error, Storage->Driver->ReadSector -> Invalid Opcode
        Storage->Driver->ReadSector(Storage , SectorAddress+SectorNumber , 1 , Cluster);
        memcpy((Cluster+((DirectoryEntryCount*sizeof(struct SFNEntry))%VBR.BytesPerSector)) , Entry , sizeof(struct SFNEntry));
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
    MemoryManagement::Free(Cluster);
    
    return true;
}

/// @brief Automatically adds LFN Entry to directory, write LFN entry using given file name
/// @param Storage Target storage
/// @param DirectoryAddress Sector Address of directory
/// @param FileName Name of file
/// @return Always return true
bool FAT16::WriteLFNEntry(struct Storage *Storage , unsigned int DirectoryAddress , const char *FileName) {
    int i;
    int j;
    int NameOffset = 0;
    int Checksum;
    int RequiredLFNEntry = (strlen(FileName)/13)+((strlen(FileName)%13 == 0) ? 0 : 1);
    struct LFNEntry *LFNEntry;
    char SFNName[13];

// cluster for other directory
    unsigned char *Cluster;
    unsigned int ClusterAddress;
    unsigned int ClusterNumber;
    unsigned int ClusterCount;

    unsigned int NextCluster;
    unsigned int DirectoryClusterSize;
    int DirectoryEntryCount;
    struct VBR VBR;

// sector for root directory
    unsigned int SectorAddress = DirectoryAddress;
    unsigned int SectorNumber;
    unsigned int SectorCount;
    unsigned int DirectorySectorSize;

    GetVBR(Storage , &(VBR));
    CreateSFNName(SFNName , FileName , 1); // To-do : number
    Checksum = GetSFNChecksum(SFNName);
    // Create LFN Entries
    printf("SFN file name   : %s\n" , SFNName);
    printf("LFN Entry count : %d\n" , RequiredLFNEntry);
    if(RequiredLFNEntry == 1) {
        return false;
    }
    LFNEntry = (struct LFNEntry *)MemoryManagement::Allocate((RequiredLFNEntry*sizeof(struct LFNEntry)));
    memset(LFNEntry , 0 , RequiredLFNEntry*sizeof(struct LFNEntry));
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
    printf("Directory Entry Count : %d\n" , DirectoryEntryCount);
    printf("LFNEntry              : 0x%X~0x%X\n" , LFNEntry , ((unsigned long)LFNEntry)+(RequiredLFNEntry*sizeof(struct LFNEntry)));
    printf("LFNEntry[0].Attribute : 0x%X\n" , LFNEntry[0].Attribute);
    printf("Directory Address that's gonna be... : %d\n" , DirectoryAddress);
    DirectoryEntryCount = GetDirectoryInfo(Storage , DirectoryAddress , &(DirectoryClusterSize));
    // If root directory, take care of it differently. 
    if(DirectoryAddress == GetRootDirectoryLocation(&(VBR))) {
        SectorNumber = (DirectoryEntryCount*sizeof(struct SFNEntry))/VBR.BytesPerSector;
        // Convert the offset of directory that's going to read - to sector number
        SectorCount = ((RequiredLFNEntry*sizeof(struct SFNEntry))/VBR.BytesPerSector)
                     +(((RequiredLFNEntry*sizeof(SFNEntry))%VBR.BytesPerSector == 0) ? 0 : 1);
        // Convert the offset of Entry number to sector number
        Cluster = (unsigned char *)MemoryManagement::Allocate(VBR.BytesPerSector*SectorCount);
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
        Cluster = (unsigned char *)MemoryManagement::Allocate(VBR.SectorsPerCluster*VBR.BytesPerSector*ClusterCount);
        for(i = 0; i < ClusterNumber; i++) {
            NextCluster = FAT16::FindNextCluster(Storage , ClusterAddress , &(VBR));
            if(NextCluster == 0xFFFF) {
                break;
            }
            ClusterAddress = NextCluster;
        }
        ReadCluster(Storage , ClusterAddress , ClusterCount , Cluster , &(VBR));
        
        memcpy(&(Cluster[(DirectoryEntryCount*sizeof(struct SFNEntry))%(VBR.BytesPerSector*VBR.SectorsPerCluster)]) , 
        LFNEntry , sizeof(struct SFNEntry)*RequiredLFNEntry);
        WriteCluster(Storage , ClusterAddress , ClusterCount , Cluster , &(VBR));
    }
    MemoryManagement::Free(LFNEntry);
    return true;
}

/// @brief Navigate and find SFN entry from given information, and rewrite(overwrite) it to given SFN entry.
/// @param Storage Target storage
/// @param DirectoryAddress Sector address of the directory
/// @param SFNName File name in 8.3 file name
/// @param NewSFNEntry New SFN entry to overwrite
/// @return If it's successfully overwritten, returns true, if not, returns false.
bool FAT16::RewriteSFNEntry(struct Storage *Storage , unsigned int DirectoryAddress , const char *SFNName , struct SFNEntry *NewSFNEntry) {
    int i;
    int Offset = 0;
    int ClusterNumber = 0;
    int EntryCount = 0;
    unsigned int DirectoryClusterSize;
    struct SFNEntry *SFNEntry;
    struct VBR VBR;
    unsigned char *Directory;
    EntryCount = GetDirectoryInfo(Storage , DirectoryAddress , &(DirectoryClusterSize));
    GetVBR(Storage , &(VBR));
    if(DirectoryAddress == GetRootDirectoryLocation(&(VBR))) {
        Directory = (unsigned char *)MemoryManagement::Allocate(GetRootDirectorySize(&(VBR))*VBR.BytesPerSector);
        Storage->Driver->ReadSector(Storage , DirectoryAddress , GetRootDirectorySize(&(VBR)) , Directory);
        for(i = 0; i < GetRootDirectorySize(&(VBR))*VBR.BytesPerSector/sizeof(struct SFNEntry); i++) {
            if(memcmp(((struct SFNEntry *)(Directory+(Offset)))->FileName , SFNName , 11) == 0) {
                memcpy((struct SFNEntry *)(Directory+(Offset)) , NewSFNEntry , sizeof(struct SFNEntry));
                Storage->Driver->WriteSector(Storage , DirectoryAddress , GetRootDirectorySize(&(VBR)) , Directory);
                MemoryManagement::Free(Directory);
                return true;
            }
            Offset += sizeof(struct SFNEntry);
        }
        MemoryManagement::Free(Directory);
        return false;
    }
    ClusterNumber = SectorToCluster(DirectoryAddress , &(VBR));
    for(i = 0; i < DirectoryClusterSize; i++) {
        Directory = (unsigned char *)MemoryManagement::Allocate(VBR.SectorsPerCluster*VBR.BytesPerSector);
        ReadCluster(Storage , ClusterNumber , 1 , Directory , &(VBR));
        for(Offset = 0; Offset < VBR.SectorsPerCluster*VBR.BytesPerSector; Offset += sizeof(struct SFNEntry)) {
            if(((struct SFNEntry *)(Directory+(Offset)))->Attribute == 0) {
                MemoryManagement::Free(Directory);
                return false;
            }
            if(memcmp(((struct SFNEntry *)(Directory+(Offset)))->FileName , SFNName , 11) == 0) {
                memcpy((struct SFNEntry *)(Directory+(Offset)) , NewSFNEntry , sizeof(struct SFNEntry));
                WriteCluster(Storage , ClusterNumber , 1 , Directory , &(VBR));
                MemoryManagement::Free(Directory);
                return true;
            }
        }
        ClusterNumber = FindNextCluster(Storage , ClusterNumber , &(VBR));
    }
    MemoryManagement::Free(Directory);
    return false;
}

/// @brief Search Entry of SFN from file name
/// @param Storage Target storage
/// @param DirectoryAddress Address of the directory
/// @param FileName Name of the file that wil get from it
/// @param Destination Pointer of SFN Entry to contain the information
/// @return If the file exists, return true, if not, return false
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
    Directory = (unsigned char *)MemoryManagement::Allocate(DirectoryClusterSize*VBR.SectorsPerCluster*VBR.BytesPerSector);
    if(DirectoryAddress == GetRootDirectoryLocation(&(VBR))) {
        Storage->Driver->ReadSector(Storage , DirectoryAddress , GetRootDirectorySize(&(VBR)) , Directory);
    }
    else {
        ReadCluster(Storage , SectorToCluster(DirectoryAddress , &(VBR)) , DirectoryClusterSize , Directory , &(VBR));
    }
    char TemporaryFileName[EntryCount*(5+6+2)];
    if((strlen(FileName) <= 11)) {
        CreateSFNName(TemporaryFileName , FileName , 1);
        printf("8.3 file name : %s\n" , TemporaryFileName);
        for(i = 0; i < EntryCount; i++) {
            SFNEntry = (struct SFNEntry *)(Directory+Offset);
            if(memcmp(TemporaryFileName , SFNEntry->FileName , 11) == 0) {
                memcpy(Destination , SFNEntry , sizeof(struct SFNEntry));
                MemoryManagement::Free(Directory);
                return true;
            }
            Offset += sizeof(struct SFNEntry);
        }
        MemoryManagement::Free(Directory);
        return false;
    }
    for(i = 0; i < EntryCount; i++) {
        SFNEntry = (struct SFNEntry *)(Directory+Offset);
        LFNEntry = (struct LFNEntry *)(Directory+Offset);
        if(LFNEntry->Attribute == 0x0F) {
            LFNEntryCount = LFNEntry->SequenceNumber^0x40;
            if(GetFileNameFromLFN(TemporaryFileName , LFNEntry) == 0) {
                Offset += sizeof(struct SFNEntry);
                continue;
            }
            if(memcmp(FileName , TemporaryFileName , strlen(FileName)) == 0) {
                Offset += LFNEntryCount*sizeof(struct LFNEntry);
                memcpy(Destination , (struct SFNEntry *)(Directory+Offset) , sizeof(struct SFNEntry));
                MemoryManagement::Free(Directory);
                return true;
            }
            memset(TemporaryFileName , 0 , strlen(TemporaryFileName));
            Offset += sizeof(struct LFNEntry)*LFNEntryCount;
        }
        else {
            Offset += sizeof(struct SFNEntry);
        }
    }
    MemoryManagement::Free(Directory);
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
    if(Entries[0].SequenceNumber == 0xE5) {
        return 0;
    }
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

/// @brief Parse name of the directories from a full file name
/// Precondition : Pointer of directories should be already allocated.
/// @param FileName A full file name that consists of full directory paths
/// @param Directories A pointer of buffer where the list of directory is saved
/// @return Number of directories
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
            Directories[j] = (char *)MemoryManagement::Allocate(i-PreviousPoint+1);
            strncpy(Directories[j] , FileName+PreviousPoint , i-PreviousPoint);
            printf("Directories[%d] : %s\n" , j , Directories[j]);
            PreviousPoint = i+1;
            j++;
        }
    }
    Directories[j] = (char *)MemoryManagement::Allocate(i-PreviousPoint+1);
    strncpy(Directories[j] , FileName+PreviousPoint , i-PreviousPoint);
    printf("Directories[%d] : %s\n" , j , Directories[j]);
    return DirectoryCount;
}

/// @brief Get location of the directory that consists the file
/// @param Storage Pointer of the storage
/// @param FileName Full file name
/// @return Return *sector location* of the directory
unsigned int FAT16::GetDirectoryLocation(struct Storage *Storage , const char *FileName) {
    int i;
    int DirectoryLocation;
    int DirectoryCount = 0;
    char **DirectoryList;
    struct SFNEntry SFNEntry;
    struct VBR VBR;
    GetVBR(Storage , &(VBR));

    DirectoryLocation = GetRootDirectoryLocation(&(VBR));
    DirectoryCount = GetDirectoryCount(FileName);
    if(DirectoryCount == 0) {
        return GetRootDirectoryLocation(&(VBR));
    }
    DirectoryList = (char **)MemoryManagement::Allocate((DirectoryCount+1)*sizeof(char *));
    ParseDirectories(FileName , DirectoryList);
    for(i = 0; i < DirectoryCount; i++) {
        if((GetSFNEntry(Storage , DirectoryLocation , DirectoryList[i] , &(SFNEntry)) == false)||(SFNEntry.Attribute != 0x10)) {
            // free
            for(i = 0; i < DirectoryCount; i++) {
                MemoryManagement::Free(DirectoryList[i]);
            }
            MemoryManagement::Free(DirectoryList);
            return 0xFFFFFFFF;
        }
        DirectoryLocation = ClusterToSector(((SFNEntry.StartingClusterHigh << 16)|SFNEntry.StartingClusterLow) , &(VBR));
    }
    for(i = 0; i < DirectoryCount; i++) {
        MemoryManagement::Free(DirectoryList[i]);
    }
    MemoryManagement::Free(DirectoryList);
    return DirectoryLocation;
}

void FAT16::WriteFileInfo(struct FileInfo *FileInfo , struct SFNEntry SFNEntry , const char *FileName , unsigned int SubdirectoryLocation , struct VBR *VBR , struct Storage *Storage , int OpenOption) {
    FileInfo->BlockSize = 512;
    FileInfo->FileName = (char *)MemoryManagement::Allocate(strlen(FileName)+1);
    strcpy(FileInfo->FileName , FileName);
    FileInfo->FileSize = SFNEntry.FileSize;
    FileInfo->CreatedDate = SFNEntry.CreatedDate;
    FileInfo->CreatedTime = SFNEntry.CreateTime;
    FileInfo->LastAccessedDate = SFNEntry.LastAccessedDate;
    FileInfo->LastWrittenDate = SFNEntry.LastWrittenDate;
    FileInfo->LastWrittenTime = SFNEntry.LastWrittenTime;
    FileInfo->Location = ClusterToSector(((SFNEntry.StartingClusterHigh << 16)|SFNEntry.StartingClusterLow) , VBR);
    FileInfo->SubdirectoryLocation = SubdirectoryLocation;
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
    FileInfo->Storage = Storage;
    FileInfo->OpenOption = OpenOption;
}
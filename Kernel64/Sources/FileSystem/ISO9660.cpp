#include <FileSystem/ISO9660.hpp>

bool ISO9660::Register(void) {
    struct ISO9660::Driver *ISO9660Driver = new ISO9660::Driver;
    return FileSystem::Register(ISO9660Driver , "ISO9660");
}

bool ISO9660::Driver::Check(struct Storage *Storage) {
    struct PrimaryVolumeDescriptor PVD;
    if(Storage->PhysicalInfo.Geometry.BytesPerSector != ISO9660_BYTES_PER_SECTOR) {
        return false;
    }
    Storage->Driver->ReadSector(Storage , ISO9660_PVD_LOCATION , 1 , &(PVD)); // linking error I guess
    if(memcmp(PVD.StandardIdentifier , "CD001" , 5) != 0) {
        return false;
    }
    return true;
}

bool ISO9660::Driver::CreateFile(struct Storage  *Storage , const char *FileName) {
    return false;
}

bool ISO9660::Driver::CreateDir(struct Storage  *Storage , const char *DirectoryName) {
    return false;
}

struct FileInfo *ISO9660::Driver::OpenFile(struct Storage  *Storage , const char *FileName , int OpenOption) {
    unsigned short Date;
    unsigned short Time;
    struct FileInfo *FileInfo;
    printf("Open File , File name : %s\n" , FileName);
    struct DirectoryRecord *DirectoryRecord;
    DirectoryRecord = (struct DirectoryRecord *)MemoryManagement::Allocate(sizeof(struct DirectoryRecord));
    if(GetFileRecord(Storage , GetRootDirectorySector(Storage) , FileName , DirectoryRecord) == false) {
        printf("File not found.\n");
        MemoryManagement::Free(DirectoryRecord);
        return 0x00;
    }
    FileInfo = (struct FileInfo *)MemoryManagement::Allocate(sizeof(struct FileInfo));
    FileInfo->BlockSize = ISO9660_BYTES_PER_SECTOR;
    FileInfo->Location = DirectoryRecord->LocationL;
    FileInfo->FileType = DirectoryRecord->FileFlags;
    FileInfo->FileSize = DirectoryRecord->DataLengthL;
    FileInfo->FileOffset = 0x00;

    Date = ((DirectoryRecord->Year+1900-1980) & 0b111111) << (4+5);
    Date |= (DirectoryRecord->Month & 0b1111) << 5;
    Date |= DirectoryRecord->Day & 0b11111;

    Time = (DirectoryRecord->Hour & 0b11111) << (6+5);
    Time |= (DirectoryRecord->Minute & 0b111111) << 5;
    Time |= (DirectoryRecord->Second & 0b11111);

    FileInfo->CreatedDate = Date;
    FileInfo->CreatedTime = Time;
    FileInfo->LastAccessedDate = Date;
    FileInfo->LastWrittenDate = Date;
    FileInfo->LastWrittenTime = Time;
    FileInfo->Storage = Storage;
    MemoryManagement::Free(DirectoryRecord);
    return FileInfo;
}

int ISO9660::Driver::CloseFile(struct FileInfo *FileInfo) {
    printf("Close File\n");
    return 1;
}

int ISO9660::Driver::RemoveFile(struct FileInfo *FileInfo) {
    printf("Remove File\n");
    return 1;
}

int ISO9660::Driver::WriteFile(struct FileInfo *FileInfo , unsigned long Size , void *Buffer) {
    printf("Write File\n");
    return 1;
}

int ISO9660::Driver::ReadFile(struct FileInfo *FileInfo , unsigned long Size , void *Buffer) {
    unsigned int ReadSize;
    unsigned int SectorAddress = FileInfo->Location+(FileInfo->FileOffset/FileInfo->BlockSize);
    unsigned int SectorCountToRead = Size/FileInfo->BlockSize+(((Size%FileInfo->BlockSize) == 0) ? 0 : 1);
    unsigned char *TempBuffer = (unsigned char *)MemoryManagement::Allocate(SectorCountToRead*FileInfo->BlockSize);
    if((ReadSize = FileInfo->Storage->Driver->ReadSector(FileInfo->Storage , SectorAddress , SectorCountToRead , TempBuffer)) == (SectorCountToRead*FileInfo->BlockSize)) {
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

int ISO9660::Driver::ReadDirectory(struct FileInfo *FileInfo , struct FileInfo *FileList) {
    return 1;
}

int ISO9660::Driver::GetFileCountInDirectory(struct FileInfo *FileInfo) {
    return 1;
}

bool ISO9660::Driver::GetFileRecord(struct Storage *Storage , unsigned long DirectoryAddress , const char *Name , struct DirectoryRecord *DirectoryRecord) {
    int i;
    int Offset = 0;
    char *FileName;
    int FileNameLength;
    unsigned char Data[ISO9660_BYTES_PER_SECTOR];
    struct PathTableEntry *Directory; // Description->PathTableLocationL
    struct DirectoryRecord *DirectoryFileEntry;
    Directory = (struct PathTableEntry*)MemoryManagement::Allocate(sizeof(struct PathTableEntry));
    DirectoryFileEntry = (struct DirectoryRecord*)MemoryManagement::Allocate(sizeof(struct DirectoryRecord));
    Storage->Driver->ReadSector(Storage , DirectoryAddress , 1 , Data);
    memcpy(Directory , Data , sizeof(PathTableEntry));
    Storage->Driver->ReadSector(Storage , Directory->Location , 1 , Data);
    printf("File Record Location : %d\n" , Directory->Location);
    while(1) {
        memcpy(DirectoryFileEntry , Data+Offset , sizeof(struct DirectoryRecord));
        if(DirectoryFileEntry->VolumeSequenceNumberL != 1) {
            break;
        }
        // printf("File List : %s\n" , (const char*)(Data+Offset+sizeof(struct DirectoryRecord)));
        if(memcmp((const char*)(Data+Offset+sizeof(struct DirectoryRecord)) , Name , DirectoryFileEntry->DirectoryLength-sizeof(struct DirectoryRecord)) == 0) {
            memcpy(DirectoryRecord , DirectoryFileEntry , sizeof(struct DirectoryRecord));
            MemoryManagement::Free(Directory);
            MemoryManagement::Free(DirectoryFileEntry);
            return true;
        }
        Offset += DirectoryFileEntry->DirectoryLength;
    }
    MemoryManagement::Free(Directory);
    MemoryManagement::Free(DirectoryFileEntry);
    return false;
}

unsigned int ISO9660::Driver::GetRootDirectorySector(struct Storage *Storage) {
    struct PrimaryVolumeDescriptor PVD;
    Storage->Driver->ReadSector(Storage , 0x10 , 1 , &(PVD));
    return PVD.PathTableLocationL;
}
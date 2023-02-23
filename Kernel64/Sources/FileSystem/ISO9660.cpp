#include <FileSystem/ISO9660.hpp>

using namespace Kernel;
using namespace Kernel::FileSystem;

bool ISO9660::Register(void) {
    FileSystem::Standard *ISO9660FileSystem = FileSystem::AssignSystem(
    ISO9660::Check , 
    
    ISO9660::OpenFile , 
    ISO9660::CloseFile , 
    ISO9660::RemoveFile , 

    ISO9660::WriteFile , 
    ISO9660::ReadFile , 
    
    ISO9660::ReadDirectory , 
    ISO9660::GetFileCountInDirectory);
    return FileSystem::Register(ISO9660FileSystem , "ISO9660");
}

bool ISO9660::Check(Drivers::StorageSystem::Storage *Storage) {
    struct PrimaryVolumeDescriptor PVD;
    if(Storage->Geometry.BytesPerSector != ISO9660_BYTES_PER_SECTOR) {
        return false;
    }
    Storage->Driver->ReadSectorFunction(Storage , ISO9660_PVD_LOCATION , 1 , &(PVD)); // linking error I guess
    if(memcmp(PVD.StandardIdentifier , "CD001" , 5) != 0) {
        return false;
    }
    return true;
}

FileSystem::FileInfo *ISO9660::OpenFile(Drivers::StorageSystem::Storage *Storage , const char *FileName) {
    unsigned short Date;
    unsigned short Time;
    FileSystem::FileInfo *FileInfo;
    Kernel::printf("Open File , File name : %s\n" , FileName);
    struct DirectoryRecord *DirectoryRecord;
    DirectoryRecord = (struct DirectoryRecord *)Kernel::MemoryManagement::Allocate(sizeof(struct DirectoryRecord));
    if(GetFileRecord(Storage , GetRootDirectorySector(Storage) , FileName , DirectoryRecord) == false) {
        Kernel::printf("File not found.\n");
        return 0x00;
    }
    FileInfo = (FileSystem::FileInfo *)Kernel::MemoryManagement::Allocate(sizeof(FileSystem::FileInfo));
    FileInfo->BlockSize = ISO9660_BYTES_PER_SECTOR;
    FileInfo->SectorAddress = DirectoryRecord->LocationL;
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
    FileInfo->StorageID = Storage->ID;
    FileInfo->StorageDriverID = Storage->Driver->ID;
    Kernel::MemoryManagement::Free(DirectoryRecord);
    return FileInfo;
}

int ISO9660::CloseFile(Drivers::StorageSystem::Storage *Storage , FileSystem::FileInfo *FileInfo) {
    Kernel::printf("Close File\n");
    return 1;
}

int ISO9660::RemoveFile(Drivers::StorageSystem::Storage *Storage , FileSystem::FileInfo *FileInfo) {
    Kernel::printf("Remove File\n");
    return 1;
}

int ISO9660::WriteFile(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo , unsigned long Size , void *Buffer) {
    Kernel::printf("Write File\n");
    return 1;
}

int ISO9660::ReadFile(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo , unsigned long Size , void *Buffer) {
    unsigned int ReadSize;
    unsigned int SectorAddress = FileInfo->SectorAddress+(FileInfo->FileOffset/FileInfo->BlockSize);
    unsigned int SectorCountToRead = Size/FileInfo->BlockSize+(((Size%FileInfo->BlockSize) == 0) ? 0 : 1);
    unsigned char *TempBuffer = (unsigned char *)Kernel::MemoryManagement::Allocate(SectorCountToRead*FileInfo->BlockSize);
    if((ReadSize = Storage->Driver->ReadSectorFunction(Storage , SectorAddress , SectorCountToRead , TempBuffer)) == (SectorCountToRead*FileInfo->BlockSize)) {
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

int ISO9660::ReadDirectory(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo , struct FileSystem::FileInfo *FileList) {
    return 1;
}

int ISO9660::GetFileCountInDirectory(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo) {
    return 1;
}

bool ISO9660::GetFileRecord(Drivers::StorageSystem::Storage *Storage , unsigned long DirectoryAddress , const char *Name , struct DirectoryRecord *DirectoryRecord) {
    int i;
    int Offset = 0;
    char *FileName;
    int FileNameLength;
    unsigned char Data[ISO9660_BYTES_PER_SECTOR];
    struct PathTableEntry *Directory; // Description->PathTableLocationL
    struct DirectoryRecord *DirectoryFileEntry;
    Directory = (struct PathTableEntry*)Kernel::MemoryManagement::Allocate(sizeof(struct PathTableEntry));
    DirectoryFileEntry = (struct DirectoryRecord*)Kernel::MemoryManagement::Allocate(sizeof(struct DirectoryRecord));
    Storage->Driver->ReadSectorFunction(Storage , DirectoryAddress , 1 , Data);
    memcpy(Directory , Data , sizeof(PathTableEntry));
    Storage->Driver->ReadSectorFunction(Storage , Directory->Location , 1 , Data);
    Kernel::printf("File Record Location : %d\n" , Directory->Location);
    while(1) {
        memcpy(DirectoryFileEntry , Data+Offset , sizeof(struct DirectoryRecord));
        if(DirectoryFileEntry->VolumeSequenceNumberL != 1) {
            break;
        }
        if(memcmp((const char*)(Data+Offset+sizeof(struct DirectoryRecord)) , Name , DirectoryFileEntry->DirectoryLength-sizeof(struct DirectoryRecord)) == 0) {
            memcpy(DirectoryRecord , DirectoryFileEntry , sizeof(struct DirectoryRecord));
            Kernel::MemoryManagement::Free(Directory);
            Kernel::MemoryManagement::Free(DirectoryFileEntry);
            return true;
        }
        Offset += DirectoryFileEntry->DirectoryLength;
    }
    Kernel::MemoryManagement::Free(Directory);
    Kernel::MemoryManagement::Free(DirectoryFileEntry);
    return false;
}

unsigned int ISO9660::GetRootDirectorySector(Drivers::StorageSystem::Storage *Storage) {
    struct PrimaryVolumeDescriptor PVD;
    Storage->Driver->ReadSectorFunction(Storage , 0x10 , 1 , &(PVD));
    return PVD.PathTableLocationL;
}
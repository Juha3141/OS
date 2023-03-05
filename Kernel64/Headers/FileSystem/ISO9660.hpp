#ifndef _ISO9660_HPP_
#define _ISO9660_HPP_

#include <Drivers/DeviceDriver.hpp>
#include <Drivers/StorageDriver.hpp>
#include <Drivers/FileSystemDriver.hpp>

#define ISO9660_BYTES_PER_SECTOR 2048
#define ISO9660_PVD_LOCATION     0x10

namespace Kernel {
    namespace FileSystem {
        namespace ISO9660 {
            struct PathTableEntry {
                unsigned char DirectoryLengthIdentifier;
                unsigned char ExtAttributeRecordLength;
                unsigned int Location;
                unsigned short ParentDirectoryNumber;/*
                char *DirectoryIdentifier;
                unsigned char Padding;*/
            };
            struct DirectoryRecord {
                unsigned char DirectoryLength;
                unsigned char ExtendedAttributeRecordLength;
                unsigned int LocationL;
                unsigned int LocationB;
                unsigned int DataLengthL;
                unsigned int DataLengthB;
                unsigned char Year;
                unsigned char Month;
                unsigned char Day;
                unsigned char Hour;
                unsigned char Minute;
                unsigned char Second;

                char OffsetGMT;
                unsigned char FileFlags;
                unsigned char FileUnitSize;
                unsigned char InterleaveGapSize;
                unsigned short VolumeSequenceNumberL;
                unsigned short VolumeSequenceNumberB;
                unsigned char FileIdentifierLength;
                /*
                unsigned char *FileIdentifier;
                unsigned char *Padding;
                unsigned char *SystemUse;*/
            };
            struct PrimaryVolumeDescriptor {
                unsigned char VolumeDescriptionType;
                char StandardIdentifier[5];
                unsigned char Version;
                unsigned char Unused;
                char SystemIdentifier[32];
                char VolumeIdentifier[32];
                unsigned char Unused2[8];
                unsigned int VolumeSpaceSizeL;
                unsigned int VolumeSpaceSizeB;
                unsigned char Unused3[32];
                unsigned short VolumeSetSizeL;
                unsigned short VolumeSetSizeB;
                unsigned short VolumeSequenceNumberL;
                unsigned short VolumeSequenceNumberB;
                unsigned short LogicalBlockSizeL;
                unsigned short LogicalBlockSizeB;
                unsigned int PathTableSizeL;
                unsigned int PathTableSizeB;
                unsigned int PathTableLocationL;
                unsigned int PathTableLocationL_opt;
                unsigned int PathTableLocationM;
                unsigned int PathTableLocationM_opt;
                struct DirectoryRecord RootDirectoryRecord;
                char VolumeSizeIdentifier[128];
                char PublisherIdentifier[128];
                char DataPreparerIdentifier[128];
                char ApplicationIdentifier[128];
                char CopyrightFileIdentifier[37];
                char AbstractFileIdentifier[37];
                char BibliographicalFileIdentifier[37];
                char CreationDateTime[17];
                char ModificationDateTime[17];
                char ExpirationDateTime[17];
                char EffectiveDateTime[17];
                unsigned char FileStructureVersion;
                unsigned char Reserved1;
                char ApplicationUse[512];
                char Reserved2[653];
            };
            bool Register(void);
            
            bool Check(Drivers::StorageSystem::Storage *Storage); // true : The storage has this file system, false : The storage has different file system.

            bool CreateFile(Drivers::StorageSystem::Storage *Storage , const char *FileName);
            bool CreateDir(Drivers::StorageSystem::Storage *Storage , const char *DirectoryName);
            FileSystem::FileInfo *OpenFile(Drivers::StorageSystem::Storage *Storage , const char *FileName);
            int CloseFile(Drivers::StorageSystem::Storage *Storage , FileSystem::FileInfo *FileInfo);
            int RemoveFile(Drivers::StorageSystem::Storage *Storage , FileSystem::FileInfo *FileInfo);
            
            int WriteFile(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo , unsigned long Size , void *Buffer);
            int ReadFile(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo , unsigned long Size , void *Buffer);

            int ReadDirectory(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo , struct FileSystem::FileInfo *FileList);
            int GetFileCountInDirectory(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo);
            
            int WriteDirectoryData(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo);

            bool GetFileRecord(Drivers::StorageSystem::Storage *Storage , unsigned long DirectoryAddress , const char *FileName , struct DirectoryRecord *DirectoryRecord);
            unsigned int GetRootDirectorySector(Drivers::StorageSystem::Storage *StorageSystem);
        }
    }
}

#endif
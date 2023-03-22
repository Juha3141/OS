#ifndef _FAT16_HPP_
#define _FAT16_HPP_

#include <Drivers/DeviceDriver.hpp>
#include <Drivers/StorageDriver.hpp>
#include <Drivers/FileSystemDriver.hpp>

namespace Kernel {
    namespace FileSystem {
        namespace FAT16 {
            struct VBR {
                unsigned char JumpCode[3];
                unsigned char OEMID[8];
                unsigned short BytesPerSector;
                unsigned char SectorsPerCluster;
                unsigned short ReservedSectorCount;
                unsigned char NumberOfFAT; // 2
                unsigned short RootDirectoryEntryCount; // 2880
                unsigned short TotalSector16;
                unsigned char MediaType;
                unsigned short FATSize16; // ((TotalSector16/SectorsPerCluster)*2)/BytesPerSector (certified)
                unsigned short SectorPerTrack;
                unsigned short NumberOfHeads;
                unsigned int HiddenSectors;
                unsigned int TotalSector32;
                unsigned char INT0x13DriveNumber;
                unsigned char Reserved;
                unsigned char BootSignature;
                unsigned int SerialNumber;
                unsigned char VolumeLabel[11];
                unsigned char FileSystemType[8]; // "FAT16   "
            };
            struct SFNEntry {
                unsigned char FileName[8];
                unsigned char Extension[3];
                unsigned char Attribute;
                unsigned short Reserved;
                unsigned short CreateTime;
                unsigned short CreatedDate;
                unsigned short LastAccessedDate;
                unsigned short StartingClusterHigh;
                unsigned short LastWrittenTime;
                unsigned short LastWrittenDate;
                unsigned short StartingClusterLow;
                unsigned int FileSize;
            };
            struct LFNEntry {
                unsigned char SequenceNumber;
                unsigned short FileName1[5];
                unsigned char Attribute;
                unsigned char Reserved;
                unsigned char Checksum;
                unsigned short FileName2[6];
                unsigned short FirstClusterLow;
                unsigned short FileName3[2];
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

            // Internal use function

            void WriteVBR(struct VBR *VBR , Drivers::StorageSystem::StorageGeometry *Geometry);
            unsigned int ReadCluster(Drivers::StorageSystem::Storage *Storage , unsigned long ClusterNumber , unsigned long ClusterCountToRead , unsigned char *Data , struct VBR *VBR);
            unsigned int FindFirstEmptyCluster(Drivers::StorageSystem::Storage *Storage);
            unsigned int GetFATAreaLocation(struct VBR *VBR);
            unsigned int GetRootDirectoryLocation(struct VBR *VBR);
            unsigned int GetRootDirectorySize(struct VBR *VBR);
            unsigned int GetDataAreaLocation(struct VBR *VBR);

            unsigned int GetDirectoryInfo(Drivers::StorageSystem::Storage *Storage , unsigned int DirectorySectorAddress , unsigned int *DirectoryClusterSize);
            unsigned int ClusterToSector(unsigned int ClusterNumber , struct VBR *VBR);
            unsigned int SectorToCluster(unsigned int SectorNumber , struct VBR *VBR);
            bool GetVBR(Drivers::StorageSystem::Storage *Storage , struct VBR *VBR);

            unsigned int FindNextCluster(Drivers::StorageSystem::Storage *Storage , unsigned int Cluster , struct VBR *VBR);

            bool GetSFNEntry(Drivers::StorageSystem::Storage *Storage , unsigned int DirectoryAddress , const char *FileName , struct SFNEntry *Destination);

            // File name whatever
            int GetFileNameFromLFN(char *FileName , struct LFNEntry *Entries);

            int GetDirectoryCount(const char *FileName);
            int ParseDirectories(const char *FileName , char **DirectoryNames);

            void WriteFileInfo(struct FileInfo *FileInfo , struct SFNEntry SFNEntry , const char *FileName , struct VBR *VBR);
        }
    }
}

#endif
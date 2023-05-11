#ifndef _FAT16_HPP_
#define _FAT16_HPP_

#include <Drivers/StorageDriver.hpp>
#include <Drivers/FileSystemDriver.hpp>

namespace Kernel {
    // Internal use function
    namespace FAT16 {
        struct Driver : public FileSystemDriver {
            bool Check(struct Storage *Storage); // true : The storage has this file system, false : The storage has different file system.

            bool CreateFile(struct Storage *Storage , const char *FileName);
            bool CreateDir(struct Storage *Storage , const char *DirectoryName);
            struct FileInfo *OpenFile(struct Storage *Storage , const char *FileName);
            int CloseFile(struct Storage *Storage , struct FileInfo *FileInfo);
            int RemoveFile(struct Storage *Storage , struct FileInfo *FileInfo);
            
            int WriteFile(struct Storage *Storage , struct FileInfo *FileInfo , unsigned long Size , void *Buffer);
            int ReadFile(struct Storage *Storage , struct FileInfo *FileInfo , unsigned long Size , void *Buffer);

            int ReadDirectory(struct Storage *Storage , struct FileInfo *FileInfo , struct FileInfo *FileList);
            int GetFileCountInDirectory(struct Storage *Storage , struct FileInfo *FileInfo);
                
            int WriteDirectoryData(struct Storage *Storage , struct FileInfo *FileInfo);
        };
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
        
        void WriteVBR(struct VBR *VBR , struct StorageGeometry *Geometry , const char *OEMID , const char *VolumeLabel , const char *FileSystem);
        unsigned int ReadCluster(struct Storage *Storage , unsigned long ClusterNumber , unsigned long ClusterCountToRead , unsigned char *Data , struct VBR *VBR);
        unsigned int WriteCluster(struct Storage *Storage , unsigned long ClusterNumber , unsigned long ClusterCountToRead , unsigned char *Data , struct VBR *VBR);
        
        unsigned int FindFirstEmptyCluster(struct Storage *Storage);
        unsigned int GetFATAreaLocation(struct VBR *VBR);
        unsigned int GetRootDirectoryLocation(struct VBR *VBR);
        unsigned int GetRootDirectorySize(struct VBR *VBR);
        unsigned int GetDataAreaLocation(struct VBR *VBR);
        
        unsigned int GetDirectoryInfo(struct Storage *Storage , unsigned int DirectorySectorAddress , unsigned int *DirectoryClusterSize);
        unsigned int ClusterToSector(unsigned int ClusterNumber , struct VBR *VBR);
        unsigned int SectorToCluster(unsigned int SectorNumber , struct VBR *VBR);
        bool GetVBR(struct Storage *Storage , struct VBR *VBR);
         
        unsigned int FindNextCluster(struct Storage *Storage , unsigned int Cluster , struct VBR *VBR);
        void WriteClusterInfo(struct Storage *Storage , unsigned int Cluster , unsigned short ClusterInfo , struct VBR *VBR);
        
        void CreateSFNName(char *SFNName , const char *LFNName , int Number);
        unsigned char GetSFNChecksum(const char *SFNName);
        bool WriteSFNEntry(struct Storage *Storage , unsigned int DirectoryAddress , struct SFNEntry *Entry);
        bool WriteLFNEntry(struct Storage *Storage , unsigned int DirectoryAddress , const char *FileName);
        bool GetSFNEntry(struct Storage *Storage , unsigned int DirectoryAddress , const char *FileName , struct SFNEntry *Destination);
        
        // File name whatever
        int GetFileNameFromLFN(char *FileName , struct LFNEntry *Entries);
        
        int GetDirectoryCount(const char *FileName);
        int ParseDirectories(const char *FileName , char **DirectoryNames);
         
        void WriteFileInfo(struct FileInfo *FileInfo , struct SFNEntry SFNEntry , const char *FileName , struct VBR *VBR);
    };
}

#endif
#ifndef _FAT16_H_
#define _FAT16_H_

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

void GetVBR(struct VBR *VBR);

unsigned int GetFATAreaLocation(struct VBR *VBR);
unsigned int GetRootDirectoryLocation(struct VBR *VBR);
unsigned int GetRootDirectorySize(struct VBR *VBR);
unsigned int GetDataAreaLocation(struct VBR *VBR);

unsigned int ClusterToSector(unsigned int ClusterNumber , struct VBR *VBR);
unsigned int SectorToCluster(unsigned int SectorNumber , struct VBR *VBR);

unsigned int FindNextCluster(unsigned int Cluster , struct VBR *VBR);

#endif
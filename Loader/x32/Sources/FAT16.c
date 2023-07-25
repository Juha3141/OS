#include <FAT16.h>
#include <BIOS.h>

// excerpted from the FAT16.cpp driver

unsigned int GetFATAreaLocation(struct VBR *VBR) {
    return VBR->ReservedSectorCount;
}

/// @brief Get sector number of the root directory
/// @param VBR VBR of the sector
/// @return Sector number of the root directory
unsigned int GetRootDirectoryLocation(struct VBR *VBR) {
    return VBR->ReservedSectorCount+(VBR->FATSize16*VBR->NumberOfFAT);
}

/// @brief Get size of the root directory "in sector"
/// @param VBR VBR of the storage
/// @return Sector size of the root directory
unsigned int GetRootDirectorySize(struct VBR *VBR) {
    return ((((VBR->RootDirectoryEntryCount*32))/VBR->BytesPerSector));
}

/// @brief Get sector number of the data area
/// @param VBR VBR of the storage
/// @return Sector number of the data area
unsigned int GetDataAreaLocation(struct VBR *VBR) {
    return GetRootDirectoryLocation(VBR)+GetRootDirectorySize(VBR);
}

unsigned int ClusterToSector(unsigned int ClusterNumber , struct VBR *VBR) {
    return ((ClusterNumber-2)*VBR->SectorsPerCluster)+GetRootDirectoryLocation(VBR)+GetRootDirectorySize(VBR);
}

unsigned int SectorToCluster(unsigned int SectorNumber , struct VBR *VBR) {
    return ((SectorNumber-GetRootDirectoryLocation(VBR)-GetRootDirectorySize(VBR))/VBR->SectorsPerCluster)+2;
}

void GetVBR(struct VBR *VBR) { // Faulty GetVBR or.. bad usage of it???
    int i;
    unsigned char BootRecord[512];
    memset(BootRecord , 0 , 512);
    
    BIOSINT_ReadSector(0 , 1 , BootRecord);

    if(BootRecord[0] == 0) {
        while(BootRecord[0] == 0) {
            BIOSINT_ReadSector(0 , 1 , BootRecord);
        }
    }
    memcpy(VBR , BootRecord , sizeof(struct VBR));
    return;
}

unsigned int FindNextCluster(unsigned int Cluster , struct VBR *VBR) {
    int SectorAddress = (int)(Cluster/256)+VBR->ReservedSectorCount;
    unsigned char FATArea[512];
    BIOSINT_ReadSector(SectorAddress , 1 , FATArea);
    return (FATArea[((Cluster%256)*2)])+(FATArea[((Cluster%256)*2)+1] << 8);
}
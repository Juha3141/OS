#include <Drivers/BootRAMDisk.hpp>

bool Kernel::Drivers::BootRAMDisk::PostInitialization(void) {
    unsigned long *Signature = (unsigned long *)BOOTRAMDISK_ADDRESS;
    if(Signature[0] != BOOTRAMDISK_SIGNATURE) {
        return false;
    }
    return true;
}

bool Kernel::Drivers::BootRAMDisk::GetGeometry(StorageSystem::StorageGeometry *Geometry) {
    memset(Geometry , 0 , sizeof(StorageSystem::StorageGeometry));
    Geometry->BytesPerSector = BOOTRAMDISK_BYTES_PER_SECTOR;
    Geometry->TotalSectorCount = ((BOOTRAMDISK_ENDADDRESS-BOOTRAMDISK_ADDRESS)/BOOTRAMDISK_BYTES_PER_SECTOR);
    Geometry->BytesPerTrack     = 0x00;
    Geometry->CylindersCount    = 0x00;
    Geometry->SectorsPerTrack   = 0x00;
    Geometry->TracksPerCylinder = 0x00;
    memcpy(Geometry->Manufacturer , "SHECKLEBERG BY ALLISON" , 22);
    memcpy(Geometry->Model , "INFAMOUS VIRTUAL RAM DISK" , 25);
    return true;
} 

unsigned long Kernel::Drivers::BootRAMDisk::ReadSector(unsigned long SectorAddress , unsigned long Count , void *Buffer) {
    unsigned long Temporary;
    unsigned long MemoryAddress;
    unsigned long StartAddress = (BOOTRAMDISK_ADDRESS+(SectorAddress*BOOTRAMDISK_BYTES_PER_SECTOR));
    for(MemoryAddress = StartAddress; MemoryAddress < (StartAddress+(Count*BOOTRAMDISK_BYTES_PER_SECTOR)); MemoryAddress += 8) {
        if(MemoryAddress >= BOOTRAMDISK_ENDADDRESS) {
            break;
        }
        *((unsigned long*)Buffer) = *((unsigned long *)MemoryAddress);
        Buffer += 8;
    }
    return (MemoryAddress-StartAddress)*BOOTRAMDISK_BYTES_PER_SECTOR;
}

unsigned long Kernel::Drivers::BootRAMDisk::WriteSector(unsigned long SectorAddress , unsigned long Count , void *Buffer) {
    unsigned long Temporary;
    unsigned long MemoryAddress;
    unsigned long StartAddress = (BOOTRAMDISK_ADDRESS+(SectorAddress*BOOTRAMDISK_BYTES_PER_SECTOR));
    for(MemoryAddress = StartAddress; MemoryAddress < (StartAddress+(Count*BOOTRAMDISK_BYTES_PER_SECTOR)); MemoryAddress += 8) {
        if(MemoryAddress >= BOOTRAMDISK_ENDADDRESS) {
            break;
        }
        *((unsigned long *)MemoryAddress) = *((unsigned long*)Buffer);
        Buffer += 8;
    }
    return (MemoryAddress-StartAddress)*BOOTRAMDISK_BYTES_PER_SECTOR;
}
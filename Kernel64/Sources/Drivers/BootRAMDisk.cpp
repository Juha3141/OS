#include <Drivers/BootRAMDisk.hpp>

using namespace Kernel;
using namespace Kernel::Drivers;

void BootRAMDisk::Register(void) {
    StorageSystem::Driver *BootRAMDiskDriver
     = StorageSystem::Assign(
        BootRAMDisk::PreInitialization , 
        BootRAMDisk::ReadSector , 
        BootRAMDisk::WriteSector , 
        BootRAMDisk::GetGeometry);
    StorageSystem::RegisterStorageDriver(BootRAMDiskDriver , "ramdisk");
}

bool BootRAMDisk::PreInitialization(StorageSystem::Driver *Driver) {
    StorageSystem::Storage *Storage;
    unsigned long *Signature = (unsigned long *)BOOTRAMDISK_ADDRESS;
    if(Signature[0] != BOOTRAMDISK_SIGNATURE) {
        return false;
    }
    Storage = StorageSystem::Assign(0 , 0 , 0 , 0);
    StorageSystem::RegisterStorage("ramdisk" , Storage);
    return true;
}

bool BootRAMDisk::GetGeometry(StorageSystem::Storage *Storage , StorageSystem::StorageGeometry *Geometry) {
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

unsigned long BootRAMDisk::ReadSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
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

unsigned long BootRAMDisk::WriteSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
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
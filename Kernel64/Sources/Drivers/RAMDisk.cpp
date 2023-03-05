#include <Drivers/RAMDisk.hpp>

using namespace Kernel;
using namespace Kernel::Drivers;

StorageSystem::Storage *RAMDisk::CreateRAMDisk(unsigned long TotalSectorCount , unsigned long BytesPerSector , unsigned long Address) {
    StorageSystem::Storage *Storage = StorageSystem::AssignStorage(0 , 2 , 0 , 1);
    Storage->Flags[0] = TotalSectorCount;
    Storage->Flags[1] = BytesPerSector;

    // Warning : You have to calculate precisely how much you're gonna use the memory,
    // otherwise your code have potential danger of corrupting memory node.

    if(Address == 0x00) {
        Storage->Resources[0] = (unsigned long)Kernel::MemoryManagement::Allocate(TotalSectorCount*BytesPerSector);
    }
    else {
        Storage->Resources[0] = Address;
    }
    StorageSystem::RegisterStorage("rd" , Storage);
    return Storage;
}

// unsigned long TotalSectorCount , unsigned long BytesPerSector , unsigned long Address=0xFFFFFFFFFFFFFFFF
void RAMDisk::Register(void) {
    StorageSystem::Driver *Driver = 
    StorageSystem::AssignDriver(
        RAMDisk::PreInitialization , 
        RAMDisk::ReadSector , 
        RAMDisk::WriteSector , 
        RAMDisk::GetGeometry);
    StorageSystem::RegisterStorageDriver(Driver , "rd");
    // Flag 0 : Total Sector Count
    // Flags 1 : Bytes Per Sector
    // Resource 0 : Physical Location
}

bool RAMDisk::PreInitialization(StorageSystem::Driver *Driver) {
    return true;
}

bool RAMDisk::GetGeometry(StorageSystem::Storage *Storage , StorageSystem::StorageGeometry *Geometry) {
    memset(Geometry , 0 , sizeof(StorageSystem::StorageGeometry));
    Geometry->TotalSectorCount = Storage->Flags[0];
    Geometry->BytesPerSector = Storage->Flags[1];
    Geometry->BytesPerTrack     = 0x00;
    Geometry->CylindersCount    = 0x00;
    Geometry->SectorsPerTrack   = 0x00;
    Geometry->TracksPerCylinder = 0x00;
    memcpy(Geometry->Manufacturer , "SHECKLEBERG BY ALLISON" , 22);
    memcpy(Geometry->Model , "INFAMOUS VIRTUAL RAM DISK" , 25);
    return true;
}

unsigned long RAMDisk::ReadSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
    unsigned long Temporary;
    unsigned long MemoryAddress;
    unsigned long StartAddress = (Storage->Resources[0]+(SectorAddress*Storage->Flags[1]));
    // Flag 0 : Total Sector Count
    // Flags 1 : Bytes Per Sector
    // Resource 0 : Physical Location
    for(MemoryAddress = StartAddress; MemoryAddress < (StartAddress+(Count*Storage->Flags[1])); MemoryAddress += 8) {
        if(MemoryAddress >= (Storage->Resources[0]+(Storage->Flags[0]*Storage->Flags[1]))) {
            break;
        }
        *((unsigned long*)Buffer) = *((unsigned long *)MemoryAddress);
        Buffer += 8;
    }
    return (MemoryAddress-StartAddress);
}

unsigned long RAMDisk::WriteSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
    unsigned long Temporary;
    unsigned long MemoryAddress;
    unsigned long StartAddress = (Storage->Resources[0]+(SectorAddress*Storage->Flags[1]));
    // Flag 0 : Total Sector Count
    // Flags 1 : Bytes Per Sector
    // Resource 0 : Physical Location
    for(MemoryAddress = StartAddress; MemoryAddress < (StartAddress+(Count*Storage->Flags[1])); MemoryAddress += 8) {
        if(MemoryAddress >= (Storage->Resources[0]+(Storage->Flags[0]*Storage->Flags[1]))) {
            break;
        }
        *((unsigned long *)MemoryAddress) = *((unsigned long*)Buffer);
        Buffer += 8;
    }
    return (MemoryAddress-StartAddress);
}
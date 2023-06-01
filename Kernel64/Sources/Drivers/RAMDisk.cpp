#include <Drivers/RAMDisk.hpp>

struct Storage *RAMDiskDriver::CreateRAMDisk(unsigned long TotalSectorCount , unsigned long BytesPerSector , unsigned long Address) {
    struct Storage *Storage = StorageSystem::Assign(0 , 2 , 0 , 1 , Storage::StorageType::Physical);
    Storage->PhysicalInfo.Flags[0] = TotalSectorCount;
    Storage->PhysicalInfo.Flags[1] = BytesPerSector;

    // Warning : You have to calculate precisely how much you're gonna use the memory,
    // otherwise your code have potential danger of corrupting memory node.

    if(Address == 0x00) {
        Storage->PhysicalInfo.Resources[0] = (unsigned long)MemoryManagement::Allocate(TotalSectorCount*BytesPerSector);
    }
    else {
        Storage->PhysicalInfo.Resources[0] = Address;
    }
    if(StorageSystem::RegisterStorage("rd" , Storage) == false) {
        printf("Failed registering storage\n");
    }
    return Storage;
}

// unsigned long TotalSectorCount , unsigned long BytesPerSector , unsigned long Address=0xFFFFFFFFFFFFFFFF
void RAMDiskDriver::Register(void) {
    class RAMDiskDriver *Driver = new RAMDiskDriver;
    StorageSystem::RegisterDriver(Driver , "rd");
    // Flag 0 : Total Sector Count
    // Flags 1 : Bytes Per Sector
    // Resource 0 : Physical Location
}
bool RAMDiskDriver::PreInitialization(void) {
    return true;
}

unsigned long RAMDiskDriver::ReadSector(struct Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) { 
    unsigned long Offset = 0;
    unsigned long Temporary;
    unsigned long MemoryAddress;
    unsigned long StartAddress = (Storage->PhysicalInfo.Resources[0]+(SectorAddress*Storage->PhysicalInfo.Flags[1]));
    // Flag 0 : Total Sector Count
    // Flags 1 : Bytes Per Sector
    // Resource 0 : Physical Location
    for(MemoryAddress = StartAddress; MemoryAddress < (StartAddress+(Count*Storage->PhysicalInfo.Flags[1])); MemoryAddress += 8) {
        if(MemoryAddress >= (Storage->PhysicalInfo.Resources[0]+(Storage->PhysicalInfo.Flags[0]*Storage->PhysicalInfo.Flags[1]))) {
            break;
        }
        *((unsigned long *)(Buffer+Offset)) = *((unsigned long *)MemoryAddress);
        Offset += 8;
    }
    return (MemoryAddress-StartAddress);
}
unsigned long RAMDiskDriver::WriteSector(struct Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
    unsigned long Offset = 0;
    unsigned long Temporary;
    unsigned long MemoryAddress;
    unsigned long StartAddress = (Storage->PhysicalInfo.Resources[0]+(SectorAddress*Storage->PhysicalInfo.Flags[1]));
    // Flag 0 : Total Sector Count
    // Flags 1 : Bytes Per Sector
    // Resource 0 : Physical Location
    for(MemoryAddress = StartAddress; MemoryAddress < (StartAddress+(Count*Storage->PhysicalInfo.Flags[1])); MemoryAddress += 8) {
        if(MemoryAddress >= (Storage->PhysicalInfo.Resources[0]+(Storage->PhysicalInfo.Flags[0]*Storage->PhysicalInfo.Flags[1]))) {
            break;
        }
        *((unsigned long *)MemoryAddress) =  *((unsigned long *)(Buffer+Offset));
        Offset += 8;
    }
    return (MemoryAddress-StartAddress);
}

bool RAMDiskDriver::GetGeometry(struct Storage *Storage , struct StorageGeometry *Geometry) {
    memset(Geometry , 0 , sizeof(struct StorageGeometry));
    Geometry->TotalSectorCount = Storage->PhysicalInfo.Flags[0];
    Geometry->BytesPerSector = Storage->PhysicalInfo.Flags[1];
    Geometry->CHS_Cylinders   = 1024;
    Geometry->CHS_Heads       = 16;
    Geometry->CHS_Sectors     = 63;
    memcpy(Geometry->Manufacturer , "SHECKLEBERG BY ALLISON" , 22);
    memcpy(Geometry->Model , "INFAMOUS VIRTUAL RAM DISK" , 25);
    return true;
}
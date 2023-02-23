#include <FileSystem/MBR.hpp>

using namespace Kernel;
using namespace Kernel::Drivers;

bool MBR::Identifier::Detect(void) {
    int i;
    // Problem : PartitionTable demands more memory than 512B, but I allocated only 512Bytes, so PartitionTable exceeded its demand, 
    // And violated memory policy. I think that the problem could be ReadSector, because it does not read only 512 bytes requested, it reads more.
    // So to solve this problem, we have to rewrite ReadSector code to make "only" read requested areas.
    if(StorageDriver == 0x00) {
        //Kernel::MemoryManagement::Free(PartitionTable);
        return false;
    }
    if(Storage->Geometry.BytesPerSector != 512) {
        //Kernel::MemoryManagement::Free(PartitionTable);
        return false;
    }
    if(StorageDriver->ReadSectorFunction(Storage , 0 , 1 , PartitionTable) != 512) {
        //Kernel::MemoryManagement::Free(PartitionTable);
        return false;
    }
    if(PartitionTable->Signature != 0xAA55) { // To-do : Improve MBR detection system
        //Kernel::MemoryManagement::Free(PartitionTable);
        return false;
    }
    for(i = 0; i < 4; i++) {
        if(PartitionTable->Entries[i].PartitionType == 0xEE) { // GPT, not MBR
            //Kernel::MemoryManagement::Free(PartitionTable);
            return false;   
        }
        if(PartitionTable->Entries[i].PartitionType != 0x00) {
            Kernel::printf("MBR Detected\n");
            //Kernel::MemoryManagement::Free(PartitionTable);
            return true;
        }
    }
    Kernel::MemoryManagement::Free(PartitionTable);
    return false;
}

StorageSystem::Partition *MBR::Identifier::GetPartition(void) {
    int i;
    struct PartitionTable *PartitionTable = (struct PartitionTable *)Kernel::MemoryManagement::Allocate(512);
    if(StorageDriver->ReadSectorFunction(Storage , 0 , 1 , PartitionTable) != 512) {
        return 0x00;
    }
    Partitions = (StorageSystem::Partition *)Kernel::MemoryManagement::Allocate(4*sizeof(struct StorageSystem::Partition));
    PartitionCount = 0;
    for(i = 0; i < 4; i++) {
        if(PartitionTable->Entries[i].PartitionType != 0) {
            Partitions[PartitionCount].StartAddressLBA = PartitionTable->Entries[i].StartingLBA;
            Partitions[PartitionCount].EndAddressLBA = PartitionTable->Entries[i].StartingLBA+PartitionTable->Entries[i].SizeInSector;
            Partitions[PartitionCount].PartitionType = PartitionTable->Entries[i].PartitionType;
            Partitions[PartitionCount].IsBootable = ((PartitionTable->Entries[i].BootableFlag == 0x00) ? false : true);
            PartitionCount++;
        }
    }
    Kernel::printf("(MBR) PartitionCount : %d (Driver %s , Storage %d)\n" , PartitionCount , StorageDriver->DriverName , Storage->ID);
    return Partitions;
}
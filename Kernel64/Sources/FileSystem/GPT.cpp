#include <FileSystem/GPT.hpp>

using namespace Kernel;
using namespace Kernel::Drivers;

bool GPT::Identifier::Detect(void) {
    int i;
    if(StorageDriver == 0x00) {
        //Kernel::MemoryManagement::Free(Header);
        return false;
    }
    if(Storage->Geometry.BytesPerSector != 512) {
        //Kernel::MemoryManagement::Free(Header);
        return false;
    }
    if(StorageDriver->ReadSectorFunction(Storage , 1 , 1 , Header) != 512) {
        //Kernel::MemoryManagement::Free(Header);
        return false;
    }
    if(memcmp(Header->Signature , "EFI PART" , 8) != 0) {
        //Kernel::MemoryManagement::Free(Header);
        return false;
    }
    Kernel::printf("GPT Detected\n");
    return true;
}

StorageSystem::Partition *GPT::Identifier::GetPartition(void) {
    int i = 0;
    int j;
    unsigned int AllocateSize = Header->NumberOfPartitionEntries*sizeof(struct PartitionTableEntry);
    struct StorageSystem::Partition *Pointer;
    if((AllocateSize%512) != 0x00) {
        AllocateSize = (((unsigned int)(AllocateSize/512))+1)*512;
    }
    Partitions = (StorageSystem::Partition *)Kernel::MemoryManagement::Allocate(128*sizeof(struct StorageSystem::Partition));
    PartitionTableEntry = (struct PartitionTableEntry *)Kernel::MemoryManagement::Allocate(AllocateSize);
    StorageDriver->ReadSectorFunction(Storage , Header->PartitionTableEntryLBA ,  AllocateSize/512 , PartitionTableEntry);
    if((PartitionTableEntry[0].PartitionTypeGUID[0] == 0) && (PartitionTableEntry[0].PartitionTypeGUID[1] == 0)
    && (PartitionTableEntry[0].PartitionTypeGUID[2] == 0) && (PartitionTableEntry[0].PartitionTypeGUID[3] == 0)) {
        i = 1;
    }
    PartitionCount = 0;
    for(j = 0; i < Header->NumberOfPartitionEntries; i++) {
        if((PartitionTableEntry[i].PartitionTypeGUID[0] == 0) && (PartitionTableEntry[i].PartitionTypeGUID[1] == 0)
        && (PartitionTableEntry[i].PartitionTypeGUID[2] == 0) && (PartitionTableEntry[i].PartitionTypeGUID[3] == 0)) {
            break;
        }
        Partitions[PartitionCount].StartAddressLBA = PartitionTableEntry[i].StartAddressLBA;
        Partitions[PartitionCount].EndAddressLBA = PartitionTableEntry[i].EndAddressLBA;
        Partitions[PartitionCount].PartitionType = PartitionTableEntry[i].AttributeFlag;
        if(Partitions[PartitionCount].PartitionType == GPT_PARTITION_LEGACY_BOOTABLE) {
            Partitions[PartitionCount].IsBootable = true;
        }
        else {
            Partitions[PartitionCount].IsBootable = false;
        }
        PartitionCount += 1;
    }
    return Partitions;
}
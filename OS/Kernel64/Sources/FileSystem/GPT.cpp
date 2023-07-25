#include <FileSystem/GPT.hpp>


bool GPT::Identifier::Detect(void) {
    int i;
    if(Driver == 0x00) {
        return false;
    }
    if(Storage->PhysicalInfo.Geometry.BytesPerSector != 512) {
        return false;
    }
	Header = (struct GPTHeader *)MemoryManagement::Allocate(128);
    if(Storage->Driver->ReadSector(Storage , 1 , 1 , Header) != 512) {
        return false;
    }
    if(memcmp(Header->Signature , "EFI PART" , 8) != 0) {
        //MemoryManagement::Free(Header);
        return false;
    }
    printf("GPT Detected\n");
    return true;
}

struct Partition *GPT::Identifier::GetPartition(void) {
    int i = 0;
    int j;
    unsigned int AllocateSize = Header->NumberOfPartitionEntries*sizeof(struct PartitionTableEntry);
    struct Partition *Pointer;
    if((AllocateSize%512) != 0x00) {
        AllocateSize = (((unsigned int)(AllocateSize/512))+1)*512;
    }
    Partitions = (Partition *)MemoryManagement::Allocate(128*sizeof(struct Partition));
    PartitionTableEntry = (struct PartitionTableEntry *)MemoryManagement::Allocate(AllocateSize);
    Storage->Driver->ReadSector(Storage , Header->PartitionTableEntryLBA ,  AllocateSize/512 , PartitionTableEntry);
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
        PartitionCount++;
    }
    return Partitions;
}

bool GPT::Identifier::CreatePartition(struct Partition Partition) {
    return false; // not implemented yet!
}
#include <FileSystem/MBR.hpp>

bool MBR::Identifier::Detect(void) {
    int i;
    // Problem : PartitionTable demands more memory than 512B, but I allocated only 512Bytes, so PartitionTable exceeded its demand, 
    // And violated memory policy. I think that the problem could be ReadSector, because it does not read only 512 bytes requested, it reads more.
    // So to solve this problem, we have to rewrite ReadSector code to make "only" read requested areas.
    if(Driver == 0x00) {
        //MemoryManagement::Free(PartitionTable);
        return false;
    }
    if(Storage->PhysicalInfo.Geometry.BytesPerSector != 512) {
        //MemoryManagement::Free(PartitionTable);
        return false;
    }
    if(Storage->Driver->ReadSector(Storage , 0 , 1 , PartitionTable) != 512) {
        //MemoryManagement::Free(PartitionTable);
        return false;
    }
    if(PartitionTable->Signature != 0xAA55) { // To-do : Improve MBR detection system
        //MemoryManagement::Free(PartitionTable);
        return false;
    }
    for(i = 0; i < 4; i++) {
        // If storage is logical storage, and it's not extended partition -> No partition found.
        if((Storage->Type == Storage::StorageType::Logical) && (PartitionTable->Entries[i].PartitionType != 0x0F) && (PartitionTable->Entries[i].PartitionType != 0x05)) {
            return false;
        }
        if(PartitionTable->Entries[i].PartitionType == 0xEE) { // GPT, not MBR
            //MemoryManagement::Free(PartitionTable);
            return false;   
        }
        if(PartitionTable->Entries[i].PartitionType != 0x00) {
            printf("MBR Detected\n");
            //MemoryManagement::Free(PartitionTable);
            return true;
        }
    }
    return false;
}

struct Partition *MBR::Identifier::GetPartition(void) {
    int i;
    struct PartitionTable *PartitionTable = (struct PartitionTable *)MemoryManagement::Allocate(512);
    printf("GetPartition from MBR\n");
    if(Storage->Driver->ReadSector(Storage , 0 , 1 , PartitionTable) != 512) {
        return 0x00;
    }
    Partitions = (struct Partition *)MemoryManagement::Allocate(4*sizeof(struct Partition));
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
    return Partitions;
}

bool MBR::Identifier::CreatePartition(struct Partition Partition) {
    int i;
    unsigned long TotalSectorCount = 0;
    struct PartitionTable *PartitionTable = (struct PartitionTable *)MemoryManagement::Allocate(512);
    this->GetPartition();
    if(PartitionCount == 4) {
        return false;
    }
    for(i = 0; i < PartitionCount; i++) {
        TotalSectorCount += (Partitions[i].EndAddressLBA-Partitions[i].StartAddressLBA);
    }
    if((TotalSectorCount > Partitions[0].StartAddressLBA+1) && (TotalSectorCount-Partitions[0].StartAddressLBA-1 >= Storage->PhysicalInfo.Geometry.TotalSectorCount)) {
        return false;
    }
    if(Driver->ReadSector(Storage , 0 , 1 , PartitionTable) != 512) {
        return false;
    }
    PartitionTable->Entries[PartitionCount].BootableFlag = ((Partition.IsBootable == true) ? 0x80 : 0x00);
    PartitionTable->Entries[PartitionCount].StartingLBA = Partition.StartAddressLBA;
    PartitionTable->Entries[PartitionCount].SizeInSector = Partition.EndAddressLBA-Partition.StartAddressLBA;
    PartitionTable->Entries[PartitionCount].PartitionType = Partition.PartitionType;
    // Later
    PartitionTable->Entries[PartitionCount].StartingCHS[0] = (Partition.StartAddressLBA)/(Storage->PhysicalInfo.Geometry.CHS_Heads*Storage->PhysicalInfo.Geometry.CHS_Sectors);
    PartitionTable->Entries[PartitionCount].StartingCHS[1] = ((Partition.StartAddressLBA)/Storage->PhysicalInfo.Geometry.CHS_Sectors)/Storage->PhysicalInfo.Geometry.CHS_Heads;
    PartitionTable->Entries[PartitionCount].StartingCHS[2] = ((Partition.StartAddressLBA)%Storage->PhysicalInfo.Geometry.CHS_Sectors)+1;

    PartitionTable->Entries[PartitionCount].EndingCHS[0] = (Partition.EndAddressLBA)/(Storage->PhysicalInfo.Geometry.CHS_Heads*Storage->PhysicalInfo.Geometry.CHS_Sectors);
    PartitionTable->Entries[PartitionCount].EndingCHS[1] = ((Partition.EndAddressLBA)/Storage->PhysicalInfo.Geometry.CHS_Sectors)/Storage->PhysicalInfo.Geometry.CHS_Heads;
    PartitionTable->Entries[PartitionCount].EndingCHS[2] = ((Partition.EndAddressLBA)%Storage->PhysicalInfo.Geometry.CHS_Sectors)+1;
    
    if(Driver->WriteSector(Storage , 0 , 1 , PartitionTable) != 512) {
        return false;
    }
    return true;
}
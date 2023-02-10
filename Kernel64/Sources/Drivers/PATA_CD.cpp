#include <Drivers/PATA_CD.hpp>

using namespace Kernel;
using namespace Kernel::Drivers;

void PATA_CD::Register(void) {
    StorageSystem::Driver *PATA_CDDriver
     = StorageSystem::Assign(
        PATA_CD::PreInitialization , 
        PATA_CD::ReadSector , 
        PATA_CD::WriteSector , 
        PATA_CD::GetGeometry);
    StorageSystem::RegisterStorageDriver(PATA_CDDriver , "idecd");
}

bool PATA_CD::PreInitialization(StorageSystem::Driver *Driver) {
    int i;
    bool Primary = true;
    StorageSystem::Storage *Storages[4];
    StorageSystem::StorageGeometry Geometries[4];
    for(i = 0; i < 2; i++) {
        Storages[i] = StorageSystem::Assign(2 , 1 , 0 , 0);
        Storages[i]->Ports[0] = PATA_PRIMARY_BASE;
        Storages[i]->Ports[1] = PATA_DEVICECONTROL_PRIMARY_BASE;
        Storages[i]->Flags[0] = Primary;
        Primary = false;
        if(StorageSystem::RegisterStorage(Driver , Storages[i]) == false) {
            Kernel::printf("Device not found in ide_cd%d\n" , i);
        }
    }
    Primary = true;
    for(i = 2; i < 4; i++) {
        Storages[i] = StorageSystem::Assign(2 , 1 , 0 , 0);
        Storages[i]->Ports[0] = PATA_SECONDARY_BASE;
        Storages[i]->Ports[1] = PATA_DEVICECONTROL_SECONDARY_BASE;
        Storages[i]->Flags[0] = Primary;
        Primary = false;
        if(StorageSystem::RegisterStorage(Driver , Storages[i]) == false) {
            Kernel::printf("Device not found in ide_cd%d\n" , i);
        }
    }
    return true;
}

// We need to detect two device types, ATA HDD Drive and ATA CDROM 
// unsigned short BasePort , unsigned short DeviceControlPort , bool Primary
bool PATA_CD::GetGeometry(StorageSystem::Storage *Storage , StorageSystem::StorageGeometry *Geometry) {
    // Seperate to one getting CDROM, one getting HDD
    int i;
    int j;
    unsigned short Status;
    unsigned short Data[256];
    unsigned short BasePort = Storage->Ports[0];
    unsigned short DeviceControlPort = Storage->Ports[1];
    struct CDGeometry CDGeometry;

    if(Storage->PortsCount != 2) {
        return false;
    }
    if(Storage->Flags[0] == true) {
        IO::Write(BasePort+PATA_PORT_DRIVE_SELECT , 0xE0); // Primary
        Kernel::printf("Reading Primary\n");
    }
    else {
        IO::Write(BasePort+PATA_PORT_DRIVE_SELECT , 0xF0); // Secondary
        Kernel::printf("Reading Secondary\n");
    }
    IO::WriteWord(BasePort+PATA_PORT_COMMAND_IO , 0xA1); // IDENTIFY
    do {
        Status = IO::ReadWord(BasePort+PATA_PORT_COMMAND_IO);
    }while((Status & 0x80) != 0x00);
    for(i = 0; i < 256; i++) {
        Data[i] = IO::ReadWord(BasePort+PATA_PORT_DATA);
    }
    memcpy(&(CDGeometry) , Data , 256);
    if((CDGeometry.Config & 0x1F00) != 0x500) {
        return false;
    }
    Kernel::printf("Config : 0x%X\n" , CDGeometry.Config);
    for(i = j = 0; i < 20; i++) {
        Geometry->Model[j++] = Data[27+i] >> 8;
        Geometry->Model[j++] = Data[27+i] & 0xFF;
    }
    Geometry->Model[j++] = 0x00;
    Kernel::printf("Model Number : %s\n" , Geometry->Model);
    GetCDROMSize(Storage);
    Geometry->BytesPerSector = 2048;
    Geometry->BytesPerTrack = 0;
    Geometry->CylindersCount = 0;
    Geometry->TracksPerCylinder = 0;
    return true;
}

unsigned long PATA_CD::GetCDROMSize(StorageSystem::Storage *Storage) {
    int i;
    unsigned char Command[12];
    unsigned char Status;
    unsigned int TotalBlock;
    unsigned int BlockSize;
    unsigned short BasePort = Storage->Ports[0];
    unsigned short DeviceControlPort = Storage->Ports[1];
    if(Storage->Flags[0] == true) {
        IO::WriteWord(BasePort+PATA_PORT_DRIVE_SELECT , 0xE0);
    }
    else {
        IO::WriteWord(BasePort+PATA_PORT_DRIVE_SELECT , 0xF0);
    }
    IO::WriteWord(BasePort+PATA_PORT_COMMAND_IO , 0xA0);
    memset(Command , 0 , sizeof(Command));
    Command[0] = 0x25;
    for(i = 0; i < sizeof(Command); i++) {
        IO::WriteWord(BasePort+PATA_PORT_DATA , Command[i]);
    }
    
    Status = IO::Read(BasePort+PATA_PORT_COMMAND_IO);

    TotalBlock = IO::ReadWord(BasePort+PATA_PORT_DATA) << 16;
    TotalBlock |= IO::ReadWord(BasePort+PATA_PORT_DATA);
    BlockSize = IO::ReadWord(BasePort+PATA_PORT_DATA) << 16;
    BlockSize |= IO::ReadWord(BasePort+PATA_PORT_DATA);
    
    IO::Write(BasePort+PATA_PORT_COMMAND_IO , Status);
    
    Kernel::printf("Total Block Count : %d\n" , TotalBlock);
    Kernel::printf("Block Size        : %d\n" , BlockSize);
    return TotalBlock;
}
// thies code needs to be fixed, and also, storage system manager should be fixed, because one driver can't handle many storage systems.
// fix the storage system so that one driver can handle many drivers. Think about usb driver...

//fixed it !

unsigned long PATA_CD::ReadSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
    return 0;
}

unsigned long PATA_CD::WriteSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
    return 0;
} 
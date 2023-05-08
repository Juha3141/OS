#include <Drivers/PATA_CD.hpp>

using namespace Kernel;
using namespace Kernel::Drivers;

void PATA_CD::Register(void) {
    StorageSystem::Driver *PATA_CDDriver
     = StorageSystem::AssignDriver(
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
        Storages[i] = StorageSystem::AssignStorage(2 , 1 , 0 , 0);
        Storages[i]->Ports[0] = PATA_PRIMARY_BASE;
        Storages[i]->Ports[1] = PATA_DEVICECONTROL_PRIMARY_BASE;
        Storages[i]->Flags[0] = Primary;
        Primary = false;
        if(StorageSystem::RegisterStorage(Driver , Storages[i]) == false) {
            Kernel::printf("Device not found in idecd%d\n" , i);
        }
    }
    Primary = true;
    for(i = 2; i < 4; i++) {
        Storages[i] = StorageSystem::AssignStorage(2 , 1 , 0 , 0);
        Storages[i]->Ports[0] = PATA_SECONDARY_BASE;
        Storages[i]->Ports[1] = PATA_DEVICECONTROL_SECONDARY_BASE;
        Storages[i]->Flags[0] = Primary;
        Primary = false;
        if(StorageSystem::RegisterStorage(Driver , Storages[i]) == false) {
            Kernel::printf("Device not found in idecd%d\n" , i);
        }
    }
    return true;
}

bool PATA_CD::Wait(unsigned short BasePort) {
    unsigned char Status;
    int i = 0;
    do {
        Status = IO::Read(BasePort+PATA_PORT_COMMAND_IO);
        if((Status & PATA_STATUS_ERROR) == PATA_STATUS_ERROR) {
            return false;
        }
        if(Status == 0x00) {
            return false;
        }
        if((Status & PATA_STATUS_DRQ) == PATA_STATUS_DRQ) {
            break;
        }
        i++;
        if(i >= 4096) {
            return false;
        }
    }while((Status & PATA_STATUS_BUSY) == PATA_STATUS_BUSY);
    return true;
}

bool PATA_CD::SendCommand(unsigned short BasePort , unsigned char *Command) {
    int i;
    IO::Write(BasePort+PATA_PORT_COMMAND_IO , 0xA0);
    if(PATA_CD::Wait(BasePort) == false) {
        return false;
    }
    for(i = 0; i < 6; i++) {
        IO::WriteWord(BasePort+PATA_PORT_DATA , ((unsigned short *)Command)[i]);
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
    }
    else {
        IO::Write(BasePort+PATA_PORT_DRIVE_SELECT , 0xF0); // Secondary
    }
    IO::Write(BasePort+PATA_PORT_COMMAND_IO , 0xA1); // IDENTIFY
    
    PATA_CD::Wait(BasePort);
    for(i = 0; i < 256; i++) {
        Data[i] = IO::ReadWord(BasePort+PATA_PORT_DATA);
    }
    memcpy(&(CDGeometry) , Data , 256);
    if((CDGeometry.Config & 0x1F00) != 0x500) {
        return false;
    }
    for(i = j = 0; i < 20; i++) {
        Geometry->Model[j++] = Data[27+i] >> 8;
        Geometry->Model[j++] = Data[27+i] & 0xFF;
    }
    Geometry->Model[j++] = 0x00;
    Kernel::printf("Model Number : %s\n" , Geometry->Model);
    if(GetCDROMSize(Storage , Geometry) == false) {
        return false;
    }
    return true;
}

bool PATA_CD::GetCDROMSize(StorageSystem::Storage *Storage , StorageSystem::StorageGeometry *Geometry) {
    int i;
    unsigned char Command[12] = {0x25 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00};
    unsigned short BasePort = Storage->Ports[0];
    unsigned char ReceivedData[8];
    if(Storage->Flags[0] == true) {
        IO::Write(BasePort+PATA_PORT_DRIVE_SELECT , 0xE0); // Primary
    }
    else {
        IO::Write(BasePort+PATA_PORT_DRIVE_SELECT , 0xF0); // Secondary
    }
    IO::Write(BasePort+PATA_PORT_FEATURES , 0);
    IO::Write(BasePort+PATA_PORT_LBAMIDDLE , 0x08);
    IO::Write(BasePort+PATA_PORT_LBAHIGH , 0x08);
    SendCommand(BasePort , Command);
    if(PATA_CD::Wait(BasePort) == false) {
        return false;
    }
    for(i = 0; i < 4; i++) {
        ((unsigned short *)(&(ReceivedData)))[i] = IO::ReadWord(BasePort+PATA_PORT_DATA);
    }
    Geometry->TotalSectorCount = (ReceivedData[0] << 24)|(ReceivedData[1] << 16)|(ReceivedData[2] << 8)|ReceivedData[3];
    Geometry->TotalSectorCount++;
    Geometry->BytesPerSector = (ReceivedData[4] << 24)|(ReceivedData[5] << 16)|(ReceivedData[6] << 8)|ReceivedData[7];
    Kernel::printf("Total sector count : %d\n" , Geometry->TotalSectorCount);
    Kernel::printf("Bytes per sector   : %d\n" , Geometry->BytesPerSector);
    // ok good now you need to clean this code up
    return true;
}
// thies code needs to be fixed, and also, storage system manager should be fixed, because one driver can't handle many storage systems.
// fix the storage system so that one driver can handle many drivers. Think about usb driver...

//fixed it !

unsigned long PATA_CD::ReadSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
    unsigned long i;
    unsigned int j;
    unsigned int TransferedSize;
    unsigned char Command[12] = {0xA8 , 0
                                      , (SectorAddress >> 24) & 0xFF // read
                                      , (SectorAddress >> 16) & 0xFF
                                      , (SectorAddress >> 8) & 0xFF
                                      , SectorAddress & 0xFF
                                      , (Count >> 24) & 0xFF
                                      , (Count >> 16) & 0xFF
                                      , (Count >> 8) & 0xFF
                                      , Count & 0xFF , 0x00 , 0x00};
    unsigned short BasePort = Storage->Ports[0];
    if(Storage->Flags[0] == true) {
        IO::Write(BasePort+PATA_PORT_DRIVE_SELECT , 0xE0); // Primary
    }
    else {
        IO::Write(BasePort+PATA_PORT_DRIVE_SELECT , 0xF0); // Secondary
    }
    if(PATA_CD::Wait(BasePort) == false) {
        return 0;
    }
    IO::Write(BasePort+PATA_PORT_ERROR , 0x00);
    IO::Write(BasePort+PATA_PORT_LBAMIDDLE , (Storage->Geometry.BytesPerSector) & 0xFF);
    IO::Write(BasePort+PATA_PORT_LBAHIGH , (Storage->Geometry.BytesPerSector >> 8) & 0xFF);
    SendCommand(BasePort , Command);
    for(i = 0; i < Count; i++) {
        if(PATA_CD::Wait(BasePort) == false) {
            return i*Storage->Geometry.BytesPerSector;
        }
        TransferedSize = IO::Read(BasePort+PATA_PORT_LBAMIDDLE)|(IO::Read(BasePort+PATA_PORT_LBAHIGH) << 8);
        for(j = 0; j < TransferedSize; j += 2) {
            *((unsigned short *)(((unsigned char *)Buffer)+((i*Storage->Geometry.BytesPerSector)+j))) = IO::ReadWord(BasePort+PATA_PORT_DATA);
            // error
        }
        //Kernel::printf("0x%X " , )
    }
    return Count*Storage->Geometry.BytesPerSector;
}

unsigned long PATA_CD::WriteSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
    return 0;
} 
#include <Drivers/IDE.hpp>

static bool PrimaryInterruptFlag = false;
static bool SecondaryInterruptFlag = false;

void IDEDriver::Register(void) {
    IDEDriver *Driver = new IDEDriver;
    IO::Write(IDE_DEVICECONTROL_PRIMARY_BASE+IDE_PORT_DIGITAL_OUTPUT , 0);
    IO::Write(IDE_DEVICECONTROL_SECONDARY_BASE+IDE_PORT_DIGITAL_OUTPUT , 0);
    StorageSystem::RegisterDriver(Driver , "idehd");
}

bool IDEDriver::PreInitialization(void) {
    int i;
    bool Master = true;
    struct Storage *Storages[4];
    struct StorageGeometry Geometries[4];
    for(i = 0; i < 2; i++) {
        Storages[i] = StorageSystem::Assign(2 , 1 , 0 , 0 , Storage::StorageType::Physical);
        Storages[i]->PhysicalInfo.Ports[0] = IDE_PRIMARY_BASE;
        Storages[i]->PhysicalInfo.Ports[1] = IDE_DEVICECONTROL_PRIMARY_BASE;
        Storages[i]->PhysicalInfo.Flags[0] = Master;
        if(Master == true) {
            Master = false;
        }
        if(StorageSystem::RegisterStorage(this , Storages[i]) == false) {
            printf("Device not found in idehd%d\n" , i);
        }
    }
    Master = true;
    for(i = 2; i < 4; i++) {
        Storages[i] = StorageSystem::Assign(2 , 1 , 0 , 0 , Storage::StorageType::Physical);
        Storages[i]->PhysicalInfo.Ports[0] = IDE_SECONDARY_BASE;
        Storages[i]->PhysicalInfo.Ports[1] = IDE_DEVICECONTROL_SECONDARY_BASE;
        Storages[i]->PhysicalInfo.Flags[0] = Master;
        if(Master == true) {
            Master = false;
        }
        if(StorageSystem::RegisterStorage(this , Storages[i]) == false) {
            printf("Device not found in idehd%d\n" , i);
        }
    }
    return true;
}

bool IDEDriver::Wait(unsigned short BasePort) {
    unsigned short Status;
    do {
        Status = IO::Read(BasePort+IDE_PORT_COMMAND_IO);
        if((Status & IDE_STATUS_ERROR) == IDE_STATUS_ERROR) {
            return false;
        }
        if(Status == 0x00) {
            return false;
        }
        if((Status & IDE_STATUS_DRQ) == IDE_STATUS_DRQ) {
            break;
        }
    }while((Status & IDE_STATUS_BUSY) == IDE_STATUS_BUSY);
    return true;
}

// We need to detect two device types, ATA HDD Drive and ATA CDROM 
// unsigned short BasePort , unsigned short DeviceControlPort , bool Primary
bool IDEDriver::GetGeometry(struct Storage *Storage , StorageGeometry *Geometry) {
    // Flags[0] : Determins whether device is primary or secondary
    int i;
    int j;
    unsigned char Status;
    unsigned short Temporary;
    unsigned short BasePort = Storage->PhysicalInfo.Ports[0];
    struct IDEGeometry IDEGeometry;
    if(Storage->PhysicalInfo.Flags[0] == true) { // true : master
        IO::Write(BasePort+IDE_PORT_DRIVE_SELECT , 0xE0); // Master : 0xA0
    }
    else {
        IO::Write(BasePort+IDE_PORT_DRIVE_SELECT , 0xF0); // Slave : 0xB0
    }
    // Ports[0] : Base port
    // Ports[1] : Device control base port
    IO::Write(BasePort+IDE_PORT_SECTOR_COUNT , 0x00);
    IO::Write(BasePort+IDE_PORT_LBALOW , 0x00);
    IO::Write(BasePort+IDE_PORT_LBAMIDDLE , 0x00);
    IO::Write(BasePort+IDE_PORT_LBAHIGH , 0x00);

    IO::Write(BasePort+IDE_PORT_COMMAND_IO , 0xEC); // 0xEC : Identify
    if(this->Wait(BasePort) == false) {
        return false;
    }
    for(i = 0; i < 256; i++) {
        ((unsigned short *)&(IDEGeometry))[i] = IO::ReadWord(BasePort+IDE_PORT_DATA);
    }
    for(i = 0; i < 20; i++) {
        Temporary = IDEGeometry.Model[i];
        IDEGeometry.Model[i] = (Temporary >> 8)|((Temporary & 0xFF) << 8);
    }
    memcpy(Geometry->Model , IDEGeometry.Model , 20*sizeof(unsigned short));
    Geometry->Model[41] = 0x00;
    Geometry->BytesPerSector = 512;
    Geometry->CHS_Cylinders = 1024;
    Geometry->CHS_Heads = 256;
    Geometry->CHS_Sectors = 63;
    Geometry->TotalSectorCount = IDEGeometry.TotalSectors;
    return true;
}
// thies code needs to be fixed, and also, storage system manager should be fixed, because one driver can't handle many storage systems.
// fix the storage system so that one driver can handle many drivers. Think about usb driver...

//fixed it !

unsigned long IDEDriver::ReadSector(struct Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
    int i;
    int j;
    bool Use28bitPIO = true;
    unsigned short BasePort = Storage->PhysicalInfo.Ports[0];
    unsigned short Status;
    unsigned long Address;
    if(Storage->PhysicalInfo.Flags[0] == true) { // true : master
        IO::Write(BasePort+IDE_PORT_DRIVE_SELECT , 0xE0); // Master : 0xA0
    }
    else {
        IO::Write(BasePort+IDE_PORT_DRIVE_SELECT , 0xF0); // Slave : 0xB0
    }
    
    if(SectorAddress > 0x10000000) {
        IO::Write(BasePort+IDE_PORT_SECTOR_COUNT , (Count >> 8) & 0xFF);
        IO::Write(BasePort+IDE_PORT_LBALOW , (SectorAddress >> 24) & 0xFF);
        IO::Write(BasePort+IDE_PORT_LBAMIDDLE , (SectorAddress >> 32) & 0xFF);
        IO::Write(BasePort+IDE_PORT_LBAHIGH , (SectorAddress >> 48) & 0xFF);
        Use28bitPIO = false;
    }
    // when accessing address is below 128GB, use 28bit PIO, because it's faster than 48bit PIO
    IO::Write(BasePort+IDE_PORT_SECTOR_COUNT , Count & 0xFF);
    IO::Write(BasePort+IDE_PORT_LBALOW , SectorAddress & 0xFF);
    IO::Write(BasePort+IDE_PORT_LBAMIDDLE , (SectorAddress >> 8) & 0xFF);
    IO::Write(BasePort+IDE_PORT_LBAHIGH , (SectorAddress >> 16) & 0xFF);
    
    // Read Sector(0x20) for 28bit, Read Sector EXT(0x24) for 48bit
    IO::Write(BasePort+IDE_PORT_COMMAND_IO , (Use28bitPIO == true) ? 0x20 : 0x24);
    for(i = 0; i < Count; i++) {
        if(this->Wait(BasePort) == false) {
            return i*512;
        }
        for(j = 0; j < 512; j += 2) {
            Address = (unsigned long)Buffer;
            Address += (i*512)+j;
            *((unsigned short *)Address) = IO::ReadWord(BasePort+IDE_PORT_DATA);
        }
    }
    return Count*512;
}

unsigned long IDEDriver::WriteSector(struct Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {

    int i;
    int j;
    bool Use28bitPIO = true;
    unsigned short BasePort = Storage->PhysicalInfo.Ports[0];
    unsigned short Status;
    unsigned long Address;
    if(Storage->PhysicalInfo.Flags[0] == true) { // true : master
        IO::Write(BasePort+IDE_PORT_DRIVE_SELECT , 0xE0); // Master : 0xA0
    }
    else {
        IO::Write(BasePort+IDE_PORT_DRIVE_SELECT , 0xF0); // Slave : 0xB0
    }
    
    if(SectorAddress > 0x10000000) {
        IO::Write(BasePort+IDE_PORT_SECTOR_COUNT , (Count >> 8) & 0xFF);
        IO::Write(BasePort+IDE_PORT_LBALOW , (SectorAddress >> 24) & 0xFF);
        IO::Write(BasePort+IDE_PORT_LBAMIDDLE , (SectorAddress >> 32) & 0xFF);
        IO::Write(BasePort+IDE_PORT_LBAHIGH , (SectorAddress >> 48) & 0xFF);
        Use28bitPIO = false;
    }
    // when accessing address is below 128GB, use 28bit PIO, because it's faster than 48bit PIO
    IO::Write(BasePort+IDE_PORT_SECTOR_COUNT , Count & 0xFF);
    IO::Write(BasePort+IDE_PORT_LBALOW , SectorAddress & 0xFF);
    IO::Write(BasePort+IDE_PORT_LBAMIDDLE , (SectorAddress >> 8) & 0xFF);
    IO::Write(BasePort+IDE_PORT_LBAHIGH , (SectorAddress >> 16) & 0xFF);
    
    // Write Sector(0x20) for 38bit, Read Sector EXT(0x34) for 48bit
    IO::Write(BasePort+IDE_PORT_COMMAND_IO , (Use28bitPIO == true) ? 0x30 : 0x34);
    for(i = 0; i < Count; i++) {
        if(this->Wait(BasePort) == false) {
            return i*512;
        }
        for(j = 0; j < 512; j += 2) {
            Address = (unsigned long)Buffer;
            Address += (i*512)+j;
            IO::WriteWord(BasePort+IDE_PORT_DATA , *((unsigned short *)Address));
        }
    }
    return Count*512;
} 

void IDE::MainInterruptHandler(bool Primary) {
    if(Primary == true) {
        PrimaryInterruptFlag = true;
        SecondaryInterruptFlag = false;
    }
    else {
        PrimaryInterruptFlag = false;
        SecondaryInterruptFlag = true;
    }
    LocalAPIC::SendEOI();
}


void IDE_CDDriver::Register(void) {
    class IDE_CDDriver *Driver = new IDE_CDDriver;
    StorageSystem::RegisterDriver(Driver , "idecd");
}

bool IDE_CDDriver::PreInitialization(void) {
    int i;
    bool Primary = true;
    struct Storage *Storages[4];
    struct StorageGeometry Geometries[4];
    for(i = 0; i < 2; i++) {
        Storages[i] = StorageSystem::Assign(2 , 1 , 0 , 0 , Storage::StorageType::Physical);
        Storages[i]->PhysicalInfo.Ports[0] = IDE_PRIMARY_BASE;
        Storages[i]->PhysicalInfo.Ports[1] = IDE_DEVICECONTROL_PRIMARY_BASE;
        Storages[i]->PhysicalInfo.Flags[0] = Primary;
        Primary = false;
        if(StorageSystem::RegisterStorage(this , Storages[i]) == false) {
            printf("Device not found in idecd%d\n" , i);
        }
    }
    Primary = true;
    for(i = 2; i < 4; i++) {
        Storages[i] = StorageSystem::Assign(2 , 1 , 0 , 0 , Storage::StorageType::Physical);
        Storages[i]->PhysicalInfo.Ports[0] = IDE_SECONDARY_BASE;
        Storages[i]->PhysicalInfo.Ports[1] = IDE_DEVICECONTROL_SECONDARY_BASE;
        Storages[i]->PhysicalInfo.Flags[0] = Primary;
        Primary = false;
        if(StorageSystem::RegisterStorage(this , Storages[i]) == false) {
            printf("Device not found in idecd%d\n" , i);
        }
    }
    return true;
}

bool IDE_CDDriver::SendCommand(unsigned short BasePort , unsigned char *Command) {
    int i;
    IO::Write(BasePort+IDE_PORT_COMMAND_IO , 0xA0);
    if(IDEDriver::Wait(BasePort) == false) {
        return false;
    }
    for(i = 0; i < 6; i++) {
        IO::WriteWord(BasePort+IDE_PORT_DATA , ((unsigned short *)Command)[i]);
    }
    return true;
}

// We need to detect two device types, ATA HDD Drive and ATA CDROM 
// unsigned short BasePort , unsigned short DeviceControlPort , bool Primary
bool IDE_CDDriver::GetGeometry(struct Storage *Storage , struct StorageGeometry *Geometry) {
    // Seperate to one getting CDROM, one getting HDD
    int i;
    int j;
    unsigned short Status;
    unsigned short Data[256];
    unsigned short BasePort = Storage->PhysicalInfo.Ports[0];
    unsigned short DeviceControlPort = Storage->PhysicalInfo.Ports[1];
    struct CDGeometry CDGeometry;

    if(Storage->PhysicalInfo.PortsCount != 2) {
        return false;
    }
    if(Storage->PhysicalInfo.Flags[0] == true) {
        IO::Write(BasePort+IDE_PORT_DRIVE_SELECT , 0xE0); // Primary
    }
    else {
        IO::Write(BasePort+IDE_PORT_DRIVE_SELECT , 0xF0); // Secondary
    }
    IO::Write(BasePort+IDE_PORT_COMMAND_IO , 0xA1); // IDENTIFY
    
    IDEDriver::Wait(BasePort);
    for(i = 0; i < 256; i++) {
        Data[i] = IO::ReadWord(BasePort+IDE_PORT_DATA);
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
    printf("Model Number : %s\n" , Geometry->Model);
    if(GetCDROMSize(Storage , Geometry) == false) {
        return false;
    }
    return true;
}

bool IDE_CDDriver::GetCDROMSize(struct Storage *Storage , struct StorageGeometry *Geometry) {
    int i;
    unsigned char Command[12] = {0x25 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00};
    unsigned short BasePort = Storage->PhysicalInfo.Ports[0];
    unsigned char ReceivedData[8];
    if(Storage->PhysicalInfo.Flags[0] == true) {
        IO::Write(BasePort+IDE_PORT_DRIVE_SELECT , 0xE0); // Primary
    }
    else {
        IO::Write(BasePort+IDE_PORT_DRIVE_SELECT , 0xF0); // Secondary
    }
    IO::Write(BasePort+IDE_PORT_FEATURES , 0);
    IO::Write(BasePort+IDE_PORT_LBAMIDDLE , 0x08);
    IO::Write(BasePort+IDE_PORT_LBAHIGH , 0x08);
    SendCommand(BasePort , Command);
    if(IDEDriver::Wait(BasePort) == false) {
        return false;
    }
    for(i = 0; i < 4; i++) {
        ((unsigned short *)(&(ReceivedData)))[i] = IO::ReadWord(BasePort+IDE_PORT_DATA);
    }
    Geometry->TotalSectorCount = (ReceivedData[0] << 24)|(ReceivedData[1] << 16)|(ReceivedData[2] << 8)|ReceivedData[3];
    Geometry->TotalSectorCount++;
    Geometry->BytesPerSector = (ReceivedData[4] << 24)|(ReceivedData[5] << 16)|(ReceivedData[6] << 8)|ReceivedData[7];
    printf("Total sector count : %d\n" , Geometry->TotalSectorCount);
    printf("Bytes per sector   : %d\n" , Geometry->BytesPerSector);
    // ok good now you need to clean this code up
    return true;
}
// thies code needs to be fixed, and also, storage system manager should be fixed, because one driver can't handle many storage systems.
// fix the storage system so that one driver can handle many drivers. Think about usb driver...

//fixed it !

unsigned long IDE_CDDriver::ReadSector(struct Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
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
    unsigned short BasePort = Storage->PhysicalInfo.Ports[0];
    if(Storage->PhysicalInfo.Flags[0] == true) {
        IO::Write(BasePort+IDE_PORT_DRIVE_SELECT , 0xE0); // Primary
    }
    else {
        IO::Write(BasePort+IDE_PORT_DRIVE_SELECT , 0xF0); // Secondary
    }
    if(IDEDriver::Wait(BasePort) == false) {
        return 0;
    }
    IO::Write(BasePort+IDE_PORT_ERROR , 0x00);
    IO::Write(BasePort+IDE_PORT_LBAMIDDLE , (Storage->PhysicalInfo.Geometry.BytesPerSector) & 0xFF);
    IO::Write(BasePort+IDE_PORT_LBAHIGH , (Storage->PhysicalInfo.Geometry.BytesPerSector >> 8) & 0xFF);
    SendCommand(BasePort , Command);
    for(i = 0; i < Count; i++) {
        if(IDEDriver::Wait(BasePort) == false) {
            return i*Storage->PhysicalInfo.Geometry.BytesPerSector;
        }
        TransferedSize = IO::Read(BasePort+IDE_PORT_LBAMIDDLE)|(IO::Read(BasePort+IDE_PORT_LBAHIGH) << 8);
        for(j = 0; j < TransferedSize; j += 2) {
            *((unsigned short *)(((unsigned char *)Buffer)+((i*Storage->PhysicalInfo.Geometry.BytesPerSector)+j))) = IO::ReadWord(BasePort+IDE_PORT_DATA);
            // error
        }
        //printf("0x%X " , )
    }
    return Count*Storage->PhysicalInfo.Geometry.BytesPerSector;
}

unsigned long IDE_CDDriver::WriteSector(struct Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
    return 0;
} 
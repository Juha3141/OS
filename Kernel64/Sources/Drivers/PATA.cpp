#include <Drivers/PATA.hpp>
#include <Drivers/PATA_CD.hpp>

using namespace Kernel;
using namespace Kernel::Drivers;

bool PrimaryInterruptFlag = false;
bool SecondaryInterruptFlag = false;

void PATA::Register(void) {
    StorageSystem::Driver *PATADriver
     = StorageSystem::Assign(
        PATA::PreInitialization , 
        PATA::ReadSector , 
        PATA::WriteSector , 
        PATA::GetGeometry);
    StorageSystem::RegisterStorageDriver(PATADriver , "idehd");
    IO::Write(PATA_DEVICECONTROL_PRIMARY_BASE+PATA_PORT_DIGITAL_OUTPUT , 0);
    IO::Write(PATA_DEVICECONTROL_SECONDARY_BASE+PATA_PORT_DIGITAL_OUTPUT , 0);
    Kernel::Drivers::PATA_CD::Register();
    
    PIC::Unmask(32+14);
    PIC::Unmask(32+15);
}

bool PATA::PreInitialization(StorageSystem::Driver *Driver) {
    int i;
    bool Master = true;
    StorageSystem::Storage *Storages[4];
    StorageSystem::StorageGeometry Geometries[4];
    for(i = 0; i < 2; i++) {
        Storages[i] = StorageSystem::Assign(2 , 1 , 0 , 0);
        Storages[i]->Ports[0] = PATA_PRIMARY_BASE;
        Storages[i]->Ports[1] = PATA_DEVICECONTROL_PRIMARY_BASE;
        Storages[i]->Flags[0] = Master;
        if(Master == true) {
            Master = false;
        }
        if(PATA::GetGeometry(Storages[i] , &(Geometries[i])) == true) {
            StorageSystem::RegisterStorage(Driver , Storages[i]);
        }
        else {
            Kernel::printf("Device not found in idehd%d\n" , i);
        }
    }
    Master = true;
    for(i = 2; i < 4; i++) {
        Storages[i] = StorageSystem::Assign(2 , 1 , 0 , 0);
        Storages[i]->Ports[0] = PATA_SECONDARY_BASE;
        Storages[i]->Ports[1] = PATA_DEVICECONTROL_SECONDARY_BASE;
        Storages[i]->Flags[0] = Master;
        if(Master == true) {
            Master = false;
        }
        if(PATA::GetGeometry(Storages[i] , &(Geometries[i])) == true) {
            StorageSystem::RegisterStorage(Driver , Storages[i]);
        }
        else {
            Kernel::printf("Device not found in idehd%d\n" , i);
        }
    }
    return true;
}

// We need to detect two device types, ATA HDD Drive and ATA CDROM 
// unsigned short BasePort , unsigned short DeviceControlPort , bool Primary
bool PATA::GetGeometry(StorageSystem::Storage *Storage , StorageSystem::StorageGeometry *Geometry) {
    // Flags[0] : Determins whether device is primary or secondary
    int i;
    int j;
    unsigned char Status;
    unsigned short Temporary;
    struct HDDGeometry HDDGeometry;
    if(Storage->Flags[0] == true) { // true : master
        IO::Write(Storage->Ports[0]+PATA_PORT_DRIVE_SELECT , 0xA0); // Master : 0xA0
    }
    else {
        IO::Write(Storage->Ports[0]+PATA_PORT_DRIVE_SELECT , 0xB0); // Slave : 0xB0
    }
    // Ports[0] : Base port
    // Ports[1] : Device control base port
    IO::Write(Storage->Ports[0]+PATA_PORT_SECTOR_COUNT , 0x00);
    IO::Write(Storage->Ports[0]+PATA_PORT_LBALOW , 0x00);
    IO::Write(Storage->Ports[0]+PATA_PORT_LBAMIDDLE , 0x00);
    IO::Write(Storage->Ports[0]+PATA_PORT_LBAHIGH , 0x00);

    IO::Write(Storage->Ports[0]+PATA_PORT_COMMAND_IO , 0xEC); // 0xEC : Identify
    do {
        Status = IO::Read(Storage->Ports[0]+PATA_PORT_COMMAND_IO);
        if((Status & PATA_STATUS_ERROR) == PATA_STATUS_ERROR) {
            return false;
        }
        if(Status == 0x00) {
            return false;
        }
        if((Status & PATA_STATUS_DRQ) == PATA_STATUS_DRQ) {
            break;
        }
    }while((Status & PATA_STATUS_BUSY) == PATA_STATUS_BUSY);
    for(i = 0; i < 256; i++) {
        ((unsigned short *)&(HDDGeometry))[i] = IO::ReadWord(Storage->Ports[0]+PATA_PORT_DATA);
    }
    for(i = 0; i < 20; i++) {
        Temporary = HDDGeometry.Model[i];
        HDDGeometry.Model[i] = (Temporary >> 8)|((Temporary & 0xFF) << 8);
    }
    memcpy(Geometry->Model , HDDGeometry.Model , 20*sizeof(unsigned short));
    Geometry->Model[41] = 0x00;
    Geometry->BytesPerSector = 512;
    Geometry->BytesPerTrack = 0;
    Geometry->CylindersCount = 0;
    Geometry->TracksPerCylinder = 0;
    Geometry->TotalSectorCount = HDDGeometry.TotalSectors;
    return true;
}
// thies code needs to be fixed, and also, storage system manager should be fixed, because one driver can't handle many storage systems.
// fix the storage system so that one driver can handle many drivers. Think about usb driver...

//fixed it !

unsigned long PATA::ReadSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
    return 0;
}

unsigned long PATA::WriteSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
    return 0;
} 

void PATA::MainInterruptHandler(bool Primary) {
    if(Primary == true) {
        PrimaryInterruptFlag = true;
        SecondaryInterruptFlag = false;
        PIC::SendEOI(32+15); // Primary, IRQ 14
    }
    else {
        PrimaryInterruptFlag = false;
        SecondaryInterruptFlag = true;
        PIC::SendEOI(32+14); // Secondary, IRQ 15
    }
}
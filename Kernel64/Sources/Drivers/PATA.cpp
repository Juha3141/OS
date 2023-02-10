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
    bool Primary = true;
    StorageSystem::Storage *Storages[4];
    StorageSystem::StorageGeometry Geometries[4];
    for(i = 0; i < 2; i++) {
        Storages[i] = StorageSystem::Assign(2 , 1 , 0 , 0);
        Storages[i]->Ports[0] = PATA_PRIMARY_BASE;
        Storages[i]->Ports[1] = PATA_DEVICECONTROL_PRIMARY_BASE;
        Storages[i]->Flags[0] = Primary;
        if(Primary == true) {
            Primary = false;
        }
        if(PATA::GetGeometry(Storages[i] , &(Geometries[i])) == true) {
            StorageSystem::RegisterStorage(Driver , Storages[i]);
        }
        else {
            Kernel::printf("Device not found in idehd%d\n" , i);
        }
    }
    Primary = true;
    for(i = 2; i < 4; i++) {
        Storages[i] = StorageSystem::Assign(2 , 1 , 0 , 0);
        Storages[i]->Ports[0] = PATA_SECONDARY_BASE;
        Storages[i]->Ports[1] = PATA_DEVICECONTROL_SECONDARY_BASE;
        Storages[i]->Flags[0] = Primary;
        if(Primary == true) {
            Primary = false;
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
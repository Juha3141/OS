#include <Drivers/AHCI.hpp>
#include <Drivers/PCI.hpp>

using namespace Kernel;
using namespace Kernel::Drivers;

bool AHCI::ReadRegister(unsigned int Register , void *Data) {
    static int HeaderType = -1;
    static unsigned long HeaderAddress;
    unsigned int ABAR;
    PCI::DeviceManager *DeviceManager = PCI::DeviceManager::GetInstance();
    if(HeaderType == -1) {
        HeaderType = DeviceManager->GetDeviceByID(&(HeaderAddress) , 0x01 , 0x06);
        if(HeaderType == -1) {
            return false;
        }
    }
    ABAR = ((PCI::HeaderType0x00 *)HeaderAddress)->BaseAddress[5];
    Kernel::printf("ABAR : 0x%X\n" , ABAR);
    return true;
}

bool AHCI::WriteRegister(unsigned int Register , void *Data) {
    static int HeaderType = -1;
    unsigned long HeaderAddress;
    PCI::DeviceManager *DeviceManager = PCI::DeviceManager::GetInstance();
    if(HeaderType == -1) {
        HeaderType = DeviceManager->GetDeviceByID(&(HeaderAddress) , 0x01 , 0x06);
        if(HeaderType == -1) {
            return false;
        }
    }
    return true;
}

void AHCI::Register(void) {
    
}

bool AHCI::PreInitialization(void) {
    return true;
}

bool AHCI::GetGeometry(StorageSystem::Storage *Storage , StorageSystem::StorageGeometry *Geometry) {
    return false;
}

unsigned long AHCI::ReadSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
    return 0x00;
}

unsigned long AHCI::WriteSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
    return 0x00;
}
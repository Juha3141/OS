#ifndef _RAMDISK_HPP_
#define _RAMDISK_HPP_

#include <Kernel.hpp>
#include <Drivers/DeviceDriver.hpp>
#include <Drivers/StorageDriver.hpp>

namespace Kernel {
    namespace Drivers {
        namespace RAMDisk {
            StorageSystem::Storage *CreateRAMDisk(unsigned long TotalSectorCount , unsigned long BytesPerSector , unsigned long Address = 0x00);
            void Register(void);
            bool PreInitialization(StorageSystem::Driver *Driver);
            bool GetGeometry(StorageSystem::Storage *Storage , StorageSystem::StorageGeometry *Geometry);
            unsigned long ReadSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer);
            unsigned long WriteSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer);
        }
    }
}

#endif
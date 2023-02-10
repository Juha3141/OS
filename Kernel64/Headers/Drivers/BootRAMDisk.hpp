#ifndef _BOOTRAMDISK_HPP_
#define _BOOTRAMDISK_HPP_

#define BOOTRAMDISK_ADDRESS          0x720000
#define BOOTRAMDISK_ENDADDRESS       0x1720000
#define BOOTRAMDISK_BYTES_PER_SECTOR 2048

#define BOOTRAMDISK_SIGNATURE        0xC001DABFACE101FF

#ifndef KERNEL32

#include <Kernel.hpp>
#include <Drivers/DeviceDriver.hpp>
#include <Drivers/StorageDriver.hpp>

namespace Kernel {
    namespace Drivers {
        namespace BootRAMDisk {
            void Register(void);
            bool PreInitialization(StorageSystem::Driver *Driver);
            bool GetGeometry(StorageSystem::Storage *Storage , StorageSystem::StorageGeometry *Geometry);
            unsigned long ReadSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer);
            unsigned long WriteSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer);
        }
    }
}

#endif

#endif
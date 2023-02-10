#ifndef _ATA_CD_HPP_
#define _ATA_CD_HPP_

#include <Kernel.hpp>
#include <Drivers/PATA.hpp>

namespace Kernel {
    namespace Drivers {
        namespace PATA_CD {
            struct CDGeometry {
                unsigned short Config;
                unsigned short Reserved1[9];
                unsigned short Serial[10];
                unsigned short Reserved2[3];
                unsigned short Firmware[4];
                unsigned short Model[20];
                unsigned short Reserved3[210];
            };
            void Register(void);
            bool PreInitialization(StorageSystem::Driver *Driver);
            bool GetGeometry(StorageSystem::Storage *Storage , StorageSystem::StorageGeometry *Geometry);
            unsigned long ReadSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer);
            unsigned long WriteSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer);

            unsigned long GetCDROMSize(StorageSystem::Storage *Storage);
        }
    }
}

#endif
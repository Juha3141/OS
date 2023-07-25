#ifndef _RAMDISK_HPP_
#define _RAMDISK_HPP_

#include <Kernel.hpp>
#include <StorageDriver.hpp>

struct RAMDiskDriver : public StorageDriver {
    static struct Storage *CreateRAMDisk(unsigned long TotalSectorCount , unsigned long BytesPerSector , unsigned long Address = 0x00);
    static void Register(void);
    bool PreInitialization(void) override;
    unsigned long ReadSector(struct Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) override;
    unsigned long WriteSector(struct Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) override;
    bool GetGeometry(struct Storage *Storage , struct StorageGeometry *Geometry) override;
};

#endif
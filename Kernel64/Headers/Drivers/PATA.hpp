#ifndef _ATA_HPP_
#define _ATA_HPP_

#include <Kernel.hpp>
#include <Drivers/StorageDriver.hpp>

#define PATA_PRIMARY_BASE                 0x1F0
#define PATA_SECONDARY_BASE               0x170
#define PATA_DEVICECONTROL_PRIMARY_BASE   0x3F0
#define PATA_DEVICECONTROL_SECONDARY_BASE 0x370

#define PATA_PORT_DATA           0x00
#define PATA_PORT_ERROR          0x01
#define PATA_PORT_FEATURES       0x01
#define PATA_PORT_SECTOR_COUNT   0x02
#define PATA_PORT_LBALOW         0x03
#define PATA_PORT_LBAMIDDLE      0x04
#define PATA_PORT_LBAHIGH        0x05
#define PATA_PORT_DRIVE_SELECT   0x06
#define PATA_PORT_COMMAND_IO     0x07

#define PATA_PORT_DIGITAL_OUTPUT 0x06
#define PATA_PORT_DRIVE_ADDRESS  0x07

#define PATA_STATUS_ERROR           0b00000001
#define PATA_STATUS_INDEX           0b00000010
#define PATA_STATUS_CORRECT_DATA    0b00000100
#define PATA_STATUS_DATA_REQUEST    0b00001000
#define PATA_STATUS_SEEK_DONE       0b00010000
#define PATA_STATUS_WRITE_FAULT     0b00100000
#define PATA_STATUS_READY           0b01000000
#define PATA_STATUS_BUSY            0b10000000

#define PATA_DRIVER_FLAG_LBA        0b00000100
#define PATA_DRIVER_FLAG_SLAVE      0b00010000

#define PATA_DIGITAL_OUTPUT_INTERRUPT_ENABLE 0b010
#define PATA_DIGITAL_OUTPUT_SOFTWARE_RESET   0b100


#define PATA_STATUS_ERROR 0b00000001
#define PATA_STATUS_INDEX 0b00000010
#define PATA_STATUS_CORR  0b00000100
#define PATA_STATUS_DRQ   0b00001000
#define PATA_STATUS_SRV   0b00010000
#define PATA_STATUS_DF    0b00100000
#define PATA_STATUS_READY 0b01000000
#define PATA_STATUS_BUSY  0b10000000

namespace Kernel {
    namespace Drivers {
        namespace PATA {
            struct HDDGeometry {
                unsigned short Config;
                unsigned short Reserved1[9];
                unsigned short Serial[10];
                unsigned short Reserved2[3];
                unsigned short Firmware[4];
                unsigned short Model[20];
                unsigned short Reserved3[13];
                unsigned int TotalSectors;
                unsigned short Reserved4[196];
            };
            void Register(void);
            bool PreInitialization(StorageSystem::Driver *Driver);
            bool GetGeometry(StorageSystem::Storage *Storage , StorageSystem::StorageGeometry *Geometry);
            unsigned long ReadSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer);
            unsigned long WriteSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer);

            void SetPrimaryInterruptFlag(bool Flag);
            void SetSecondaryInterruptFlag(bool Flag);

            void InterruptHandler_IRQ14(void);
            void InterruptHandler_IRQ15(void);
            void MainInterruptHandler(bool Primary); // IRQ 14 : Primary , IRQ 15 : Secondary
        }
    }
}

#endif
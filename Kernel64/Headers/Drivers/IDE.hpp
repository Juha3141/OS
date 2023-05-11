#ifndef _IDE_HPP_
#define _IDE_HPP_

#include <Drivers/StorageDriver.hpp>
#include <Drivers/FileSystemDriver.hpp>

#define IDE_PRIMARY_BASE                 0x1F0
#define IDE_SECONDARY_BASE               0x170
#define IDE_DEVICECONTROL_PRIMARY_BASE   0x3F0
#define IDE_DEVICECONTROL_SECONDARY_BASE 0x370

#define IDE_PORT_DATA           0x00
#define IDE_PORT_ERROR          0x01
#define IDE_PORT_FEATURES       0x01
#define IDE_PORT_SECTOR_COUNT   0x02
#define IDE_PORT_LBALOW         0x03
#define IDE_PORT_LBAMIDDLE      0x04
#define IDE_PORT_LBAHIGH        0x05
#define IDE_PORT_DRIVE_SELECT   0x06
#define IDE_PORT_COMMAND_IO     0x07

#define IDE_PORT_DIGITAL_OUTPUT 0x06
#define IDE_PORT_DRIVE_ADDRESS  0x07

#define IDE_STATUS_ERROR           0b00000001
#define IDE_STATUS_INDEX           0b00000010
#define IDE_STATUS_CORRECT_DATA    0b00000100
#define IDE_STATUS_DATA_REQUEST    0b00001000
#define IDE_STATUS_SEEK_DONE       0b00010000
#define IDE_STATUS_WRITE_FAULT     0b00100000
#define IDE_STATUS_READY           0b01000000
#define IDE_STATUS_BUSY            0b10000000

#define IDE_DRIVER_FLAG_LBA        0b00000100
#define IDE_DRIVER_FLAG_SLAVE      0b00010000

#define IDE_DIGITAL_OUTPUT_INTERRUPT_ENABLE 0b010
#define IDE_DIGITAL_OUTPUT_SOFTWARE_RESET   0b100


#define IDE_STATUS_ERROR 0b00000001
#define IDE_STATUS_INDEX 0b00000010
#define IDE_STATUS_CORR  0b00000100
#define IDE_STATUS_DRQ   0b00001000
#define IDE_STATUS_SRV   0b00010000
#define IDE_STATUS_DF    0b00100000
#define IDE_STATUS_READY 0b01000000
#define IDE_STATUS_BUSY  0b10000000

    struct IDEDriver : StorageDriver {
        bool PreInitialization(void) override;
        unsigned long ReadSector(struct Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) override;
        unsigned long WriteSector(struct Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) override;
        bool GetGeometry(struct Storage *Storage , struct StorageGeometry *Geometry) override;

        static bool Wait(unsigned short BasePort);
        static void Register(void);
        static bool PrimaryInterruptFlag;
        static bool SecondaryInterruptFlag;

    };
    class IDE_CDDriver : StorageDriver { // inherit from IDEDriver or StorageDriver??
        static void Register(void);
        
        bool PreInitialization(void) override;
        unsigned long ReadSector(struct Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) override;
        unsigned long WriteSector(struct Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) override;
        bool GetGeometry(struct Storage *Storage , struct StorageGeometry *Geometry) override;

        static bool SendCommand(unsigned short BasePort , unsigned char *Command);
        static bool GetCDROMSize(struct Storage *Storage , struct StorageGeometry *Geometry);
    };
    
    struct IDEGeometry {
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
    struct CDGeometry {
        unsigned short Config;
        unsigned short Reserved1[9];
        unsigned short Serial[10];
        unsigned short Reserved2[3];
        unsigned short Firmware[4];
        unsigned short Model[20];
        unsigned short Reserved3[210];
    };
    namespace IDE {
        void MainInterruptHandler(bool Primary);
    }

#endif
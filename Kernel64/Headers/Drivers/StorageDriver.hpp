#ifndef _STORAGEStorageR_HPP_
#define _STORAGEStorageR_HPP_

// I didn't know what block device Storager was, so I named it "Storage Storager".
// How ridiculous..

#include <Kernel.hpp>
#include <Drivers/DeviceDriver.hpp>

#define STORAGESYSTEM_MAXCOUNT 512

#define STORAGESYSTEM_MBR 0x01
#define STORAGESYSTEM_GPT 0x02
#define STORAGESYSTEM_ETC 0x03

namespace Kernel {
    namespace FileSystem {
        struct Standard;
    }
    namespace Drivers {
        namespace StorageSystem {
            struct Partition { // to - do : Detect file system & implement it
                unsigned long StartAddressLBA;
                unsigned long EndAddressLBA;
                unsigned int Partitionype;
                char *PartitionName;

                unsigned char PartitionTypeGUID[16];
                unsigned char UniquePartitionGUID[16];
                struct Partition *NextPartition;
            };
            struct StorageGeometry { // StorageGeometry
                unsigned long CylindersCount;
                unsigned long TracksPerCylinder;
                unsigned long SectorsPerTrack;
                unsigned long BytesPerTrack;
                
                unsigned long BytesPerSector;
                unsigned long TotalSectorCount;

                unsigned char Model[41];
                unsigned char Manufacturer[24];
            };
            struct Driver;
            struct Storage {
                unsigned short *Ports;
                int PortsCount;
                
                unsigned int *IRQs; // if there is no IRQ, this value is -1
                int IRQsCount;

                unsigned long *Flags;
                int FlagsCount;

                unsigned long *Resources;
                int ResourcesCount;

                unsigned short ID; // Storage ID

                struct StorageGeometry Geometry;

                char FileSystemString[24];
                struct FileSystem::Standard *FileSystem;
                struct StorageSystem::Driver *Driver;
                Partition *PartitionNode;
            };
            typedef bool (*StandardPreInitializationFunction)(Driver *Driver);
            typedef unsigned long (*StandardReadSectorFunction)(Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer);
            typedef unsigned long (*StandardWriteSectorFunction)(Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer);
            typedef bool (*StandardGetGeometryFunction)(Storage *Storage , StorageSystem::StorageGeometry *Geometry);
            struct Driver { // Only One storage driver exist.
                StandardPreInitializationFunction PreInitialization;
                StandardReadSectorFunction ReadSectorFunction;
                StandardWriteSectorFunction WriteSectorFunction;
                StandardGetGeometryFunction GetGeometryFunction;
                
                char DriverName[24]; // IDE, RAMDisk, USB, or etc..
                DriverSystemManager<Storage> *StoragesManager;

                unsigned int ID;    // Storage Driver** ID
            };
            // Register Storager while booting -> Start Initialization
            // Initialization : Searches devices and registers it **
            // ReadSector & WriteSector
            // Etc..
            void Initialize(void);
            StorageSystem::Driver *Assign(
            StandardPreInitializationFunction PostInitialization , 
            StandardReadSectorFunction ReadSectorFunction , 
            StandardWriteSectorFunction WriteSectorFunction , 
            StandardGetGeometryFunction GetGeometryFunction);
            bool RegisterStorageDriver(StorageSystem::Driver *StorageDriver , const char *DriverName);
            StorageSystem::Driver *SearchStorageDriver(const char *DriverName);
            StorageSystem::Driver *SearchStorageDriver(unsigned long ID);
            StorageSystem::Driver *DeregisterStorageDriver(const char *DriverName);
            StorageSystem::Driver *DeregisterStorageDriver(unsigned long ID);

            StorageSystem::Storage *Assign(int PortsCount , int FlagsCount , int IRQsCount , int ResourcesCount);
            bool RegisterStorage(StorageSystem::Driver *StorageDriver , StorageSystem::Storage *Storage);
            bool RegisterStorage(const char *DriverName , Storage *Storage);
            StorageSystem::Storage *SearchStorage(const char *DriverName , unsigned long StorageID);
            bool DeregisterStorage(const char *DriverName , unsigned long StorgeID);
        }
    }
}

#endif
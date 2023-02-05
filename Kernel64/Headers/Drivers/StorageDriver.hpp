#ifndef _STORAGEDRIVER_HPP_
#define _STORAGEDRIVER_HPP_

// I didn't know what block device driver was, so I named it "Storage Driver".
// How ridiculous..

#include <Kernel.hpp>

#define STORAGESYSTEM_MAXCOUNT 512

namespace Kernel {
    namespace FileSystem {
        struct Standard;
    }
    namespace Drivers {
        namespace StorageSystem {
            struct StorageGeometry {
                unsigned long CylindersCount;
                unsigned long TracksPerCylinder;
                unsigned long SectorsPerTrack;
                unsigned long BytesPerTrack;
                unsigned long BytesPerSector;
                unsigned long TotalSectorCount;

                unsigned char Model[41];
                unsigned char Manufacturer[24];
            };
            typedef bool (*StandardPostInitializationFunction)(void);
            typedef unsigned long (*StandardReadSectorFunction)(unsigned long SectorAddress , unsigned long Count , void *Buffer);
            typedef unsigned long (*StandardWriteSectorFunction)(unsigned long SectorAddress , unsigned long Count , void *Buffer);
            typedef bool (*StandardGetGeometryFunction)(StorageSystem::StorageGeometry *Geometry);
            struct Standard {
                StandardPostInitializationFunction PostInitialization;
                StandardReadSectorFunction ReadSectorFunction;
                StandardWriteSectorFunction WriteSectorFunction;
                StandardGetGeometryFunction GetGeometryFunction;
                
                char StorageName[24];
                char FileSystemString[24];
                struct FileSystem::Standard *FileSystem;
                struct StorageGeometry Geometry;

                unsigned int ID;
            };
            // Register Driver while booting -> Start Initialization
            // Initialization : Searches devices and registers it **
            // ReadSector & WriteSector
            // Etc..
            void Initialize(void);
            StorageSystem::Standard *AssignSystem(
            StandardPostInitializationFunction PostInitialization , 
            StandardReadSectorFunction ReadSectorFunction , 
            StandardWriteSectorFunction WriteSectorFunction , 
            StandardGetGeometryFunction GetGeometryFunction);
            bool Register(StorageSystem::Standard *StorageSystem , const char *Name);
            StorageSystem::Standard *Search(const char *StorageName);
            StorageSystem::Standard *Search(unsigned long ID);
            StorageSystem::Standard *Deregister(const char *StorageName);
            StorageSystem::Standard *Deregister(unsigned long ID);
        }
    }
}

#endif
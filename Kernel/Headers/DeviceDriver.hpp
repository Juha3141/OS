#ifndef _STORAGESYSTEM_HPP_
#define _STORAGESYSTEM_HPP_

#include <Kernel.hpp>

#define STORAGESYSTEM_DIRECTORY_PARSE_CHARACTER '/'
#define STORAGESYSTEM_ARGUMENT_MAXCOUNT 10

#define FILESYSTEM_FAT12    0
#define FILESYSTEM_FAT16    1
#define FILESYSTEM_FAT32    2
#define FILESYSTEM_ISO9660  3

namespace CommonFileSystem {
    struct FileInfo {
        char StorageName[64];
        char FileName[128];
        unsigned long SectorAddress;
        unsigned long ClusterAddress;
        unsigned char Attribute;
        unsigned short CreatedTime;
        unsigned short CreatedDate;
        unsigned short LastAccessedDate;
        unsigned short LastWrittenTime;
        unsigned short LastWrittenDate;
        unsigned long FileSize;
        
        unsigned long CurrentFileOffset;
    };
}

namespace Kernel {
    namespace Device {
        struct StorageSystemArgument {
            unsigned long Count;
            unsigned long Data[STORAGESYSTEM_ARGUMENT_MAXCOUNT];
        };
        class StorageSystem {
            public:
                void Initialize(unsigned long ReadSectorsFunction , unsigned long WriteSectorsFunction , const char *StroageName , const char *DriveName);
                void (*ReadSector)();
                void (*WriteSector)();

                inline char *GetStorageName(void) {
                    return StorageName;
                }
                inline char *GetDriveName(void) {
                    return DriveName;
                }

                int FileSystem;
            private:
                char StorageName[32];
                char DriveName[32];
        };
    };
};

#endif
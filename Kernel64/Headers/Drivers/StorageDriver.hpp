#ifndef _STORAGEDRIVER_HPP_
#define _STORAGEDRIVER_HPP_

// I didn't know what block device Storager was, so I named it "Storage Storager".
// How ridiculous..

#include <Kernel.hpp>

#define STORAGESYSTEM_MAXCOUNT 512

#define STORAGESYSTEM_MBR 0x01
#define STORAGESYSTEM_GPT 0x02
#define STORAGESYSTEM_ETC 0x03

#define STORAGESYSTEM_INVALIDID 0xFFFFFFFFFFFFFFFF

#define PARTITIONID_PHYSICALDRIVE 0xFFFFFFFF

    class FileSystemDriver;
    struct StorageGeometry {
        // For CHS
        unsigned long CHS_Cylinders;
        unsigned long CHS_Heads;
        unsigned long CHS_Sectors;
        
        // For LBA
        unsigned long BytesPerSector;
        unsigned long TotalSectorCount;

        unsigned char Model[41];
        unsigned char Manufacturer[24];
    };

    struct Partition { // to - do : Detect file system & implement it
        unsigned long StartAddressLBA;
        unsigned long EndAddressLBA;
        unsigned int PartitionType;
        bool IsBootable;

        char PartitionName[72];

        unsigned char PartitionTypeGUID[16];
        unsigned char UniquePartitionGUID[16];
    };

    struct PhysicalStorageInfo {
        // Contains physical information of storage
        unsigned short *Ports;
        int PortsCount;

        unsigned int *IRQs; // if there is no IRQ, this value is -1
        int IRQsCount;

        unsigned long *Flags;
        int FlagsCount;
            
        unsigned long *Resources;
        int ResourcesCount;

        struct StorageGeometry Geometry;
    };

    struct StorageDriver;
    class StorageManager;

    struct Storage {
        // For identification
        // ID of the Storage, all same for same storage
        unsigned long ID;
        // If it's physical drive : PARTITIONID_PHYSICALDRIVE
        unsigned long PartitionID;

        StorageDriver *Driver;
        struct FileSystemDriver *FileSystem;
        char FileSystemString[24];
        
        struct PhysicalStorageInfo PhysicalInfo;
        
        unsigned char PartitionScheme; // MBR or GPT?
        enum StorageType { Physical , Logical } Type;

        // You have to go now...

        // If this storage is a physical storage, use information here : 
        StorageManager *LogicalStorages;
        // If it's logical storage or there's no partition, leave it zero
        // If this is a logical storage, use information here : 
        Partition LogicalPartitionInfo;
    };

    struct StorageDriver {
        friend class StorageDriverManager;
        virtual bool PreInitialization(void) = 0; 
        virtual unsigned long ReadSector(struct Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) = 0;
        virtual unsigned long WriteSector(struct Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) = 0;
        virtual bool GetGeometry(struct Storage *Storage , struct StorageGeometry *Geometry) = 0;
        
        class StorageManager *StorageManager;

        unsigned int DriverID;
        char DriverName[24];
    };
    
    // Driver that universally handle partition storage.
    // Doesn't registered to storage driver manager
    // But it's stored in [Driver] pointer in partition storage.
    template <typename T>class ObjectManager { // Manages [Pointer of] some object
        public:
            virtual void Initialize(int MaxCount) {
                MaxObjectCount = MaxCount;
            }
            virtual unsigned long Register(T *Object) { // returns ID
                unsigned int i;
                if(ObjectContainer == 0x00) {
                    printf("Error : ObjectContainer yet not initialized\n");
                    return STORAGESYSTEM_INVALIDID;
                }
                if(CurrentObjectCount >= MaxObjectCount) {
                    return STORAGESYSTEM_INVALIDID;
                }
                for(i = 0; i < MaxObjectCount; i++) {
                    if(ObjectContainer[i].Using == false) {
                        break;
                    }
                }
                ObjectContainer[i].Using = true;
                if(i >= MaxObjectCount) {
                    return STORAGESYSTEM_INVALIDID; // all object is occupied;
                }
                ObjectContainer[i].Object = Object;
                CurrentObjectCount++;
                return i;
            }
            virtual unsigned long Deregister(T *Object) {
                unsigned int i;
                for(i = 0; i < CurrentObjectCount; i++) {
                    if(ObjectContainer[i].Object == Object) {
                        return i;
                    }
                }
                return STORAGESYSTEM_INVALIDID;
            }
            virtual T *GetObject(unsigned long ID) {
                if(ObjectContainer[ID].Using == false) {
                    return 0x00;
                }
                if(ID >= MaxObjectCount) {
                    return 0x00;
                }
                return ObjectContainer[ID].Object;
            }
            unsigned int MaxObjectCount = 0;
            unsigned int CurrentObjectCount = 0;
        protected:
            struct ObjectContainer {
                T *Object;
                bool Using = false;
            }*ObjectContainer = 0x00;
    };

    class StorageDriverManager : public ObjectManager<StorageDriver> {
        public:
            static StorageDriverManager *GetInstance(void) {
                static class StorageDriverManager *Instance;
                if(Instance == 0x00) {
                    // to-do : change everything to new plz
                    Instance = new StorageDriverManager; // (class StorageDriverManager *)MemoryManagement::Allocate(sizeof(StorageDriverManager));
                }
                return Instance;
            }
            void Initialize(void) {
                ObjectManager<StorageDriver>::Initialize(256);
                ObjectContainer = (struct ObjectManager::ObjectContainer *)MemoryManagement::Allocate(sizeof(struct ObjectManager::ObjectContainer)*MaxObjectCount);
            }
            unsigned long Register(StorageDriver *Driver) {
                Driver->DriverID = ObjectManager<StorageDriver>::Register(Driver);
                return Driver->DriverID;
            }
            StorageDriver *GetObjectByName(const char *DriverName) {
                int i;
                for(i = 0; i < MaxObjectCount; i++) {
                    if(strlen(ObjectContainer[i].Object->DriverName) != strlen(DriverName)) {
                        continue;
                    }
                    if(memcmp(ObjectContainer[i].Object->DriverName , DriverName , strlen(ObjectContainer[i].Object->DriverName)) == 0) {
                        return ObjectContainer[i].Object;
                    }
                }
                return 0x00;
            }
    };

    class StorageManager : public ObjectManager<struct Storage> {
        public:
            void Initialize(void) {
                ObjectManager<struct Storage>::Initialize(256);
                ObjectContainer = (struct ObjectManager::ObjectContainer *)MemoryManagement::Allocate(sizeof(struct ObjectManager::ObjectContainer)*MaxObjectCount);
            }
    };

    namespace StorageSystem {
        // should we just remove this part and use Singleton object???
        // no just use for convenience
        /////////////////////////////////////////////////////////////////////////////////
        void Initialize(void);
        bool RegisterDriver(StorageDriver *Driver , const char *DriverName);
        StorageDriver *SearchDriver(const char *DriverName);
        StorageDriver *SearchDriver(unsigned long ID);
        unsigned long DeregisterDriver(const char *DriverName);
        unsigned long DeregisterDriver(unsigned long ID);
        /////////////////////////////////////////////////////////////////////////////////
        
        bool RegisterStorage(struct StorageDriver *Driver , struct Storage *Storage);
        bool RegisterStorage(unsigned long DriverID , struct Storage *Storage);
        bool RegisterStorage(const char *DriverName , Storage *Storage);
        struct Storage *SearchStorage(const char *DriverName , unsigned long StorageID);
        struct Storage *SearchStorage(unsigned long DriverID , unsigned long StorageID);
        bool DeregisterStorage(const char *DriverName , unsigned long StorgeID);
        
        struct Storage *Assign(int PortsCount , int FlagsCount , int IRQsCount , int ResourcesCount , enum Storage::StorageType Type);
        void AddLogicalDrive(StorageDriver *Driver , struct Storage *Storage , struct Partition *Partitions , int PartitionCount);
    }

    struct PartitionDriver : public StorageDriver {
        void SetSuperDriver(struct StorageDriver *Driver) {
            SuperDriver = Driver;
        }
        bool PreInitialization(void) { 
            printf("It's not allowed to do this!\n");
            return false;
        };
        unsigned long ReadSector(struct Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
            struct Storage *PhysicalStorage;
            if(Storage->Type == Storage::StorageType::Physical) {
                return 0x00;
            }
            if(Storage->Type == Storage::StorageType::Logical) {
                SectorAddress += Storage->LogicalPartitionInfo.StartAddressLBA;
            }
            if(Storage->Driver->DriverID == STORAGESYSTEM_INVALIDID) {
                return 0x00;
            }
            // Get physical storage pointer of logical storage
            PhysicalStorage = SuperDriver->StorageManager->GetObject(Storage->ID);
            if(PhysicalStorage == 0x00) {
                return 0x00;
            }
            return PhysicalStorage->Driver->ReadSector(PhysicalStorage , SectorAddress , Count , Buffer);
        }
        unsigned long WriteSector(struct Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer) {
            struct Storage *PhysicalStorage;
            if(Storage->Type == Storage::StorageType::Physical) {
                return 0x00;
            }
            if(Storage->Type == Storage::StorageType::Logical) {
                SectorAddress += Storage->LogicalPartitionInfo.StartAddressLBA;
            }
            if(Storage->Driver->DriverID == STORAGESYSTEM_INVALIDID) {
                return 0x00;
            }
            PhysicalStorage = SuperDriver->StorageManager->GetObject(Storage->ID);
            if(PhysicalStorage == 0x00) {
                return 0x00;
            }
            return PhysicalStorage->Driver->WriteSector(Storage , SectorAddress , Count , Buffer);
        }
        bool GetGeometry(struct Storage *Storage , struct StorageGeometry *Geometry) {
            if(SuperDriver == 0x00) {
                printf("Warning : SuperDriver yet initialized!\n");
                return false;
            }
            return SuperDriver->GetGeometry(Storage , Geometry);
        }
        struct StorageDriver *SuperDriver;
    };

#endif
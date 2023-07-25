#ifndef _MOUNTSYSTEM_HPP_
#define _MOUNTSYSTEM_HPP_

#include <FileSystemDriver.hpp>
#include <StorageDriver.hpp>

#define MOUNTSYSTEM_MAXCOUNT 256

namespace MountSystem {
    class InterfaceHandler {
        public:
            virtual void *Open(void) = 0;
            virtual int Write(unsigned long Size , void *Buffer) = 0;
            virtual int Read(unsigned long Size , void *Buffer) = 0;
            virtual void Close(/*unsigned long Something*/) = 0;
    };
    struct MountInfo {
        unsigned long MountID; // Same as ObjectManager
        char AccessFileName[100];
        // Original name of file to access
        enum Type {
            StorageLink , SystemInterface
        }Type;
        //   ============ StorageLink part ============
        // File Type : "*.mnt"
        // File Name : Name of the Storage
        struct Storage *TargetStorage;

        // ============ SystemInterface part ============
        // File Type : "*.sys" ?? undetermined yet
        char InterfaceName[100];
        class InterfaceHandler *Handler;
    };
    class MountInfoManager : public ObjectManager<MountInfo> {
        friend class UniversalMountManager;
        public:
            void Initialize(void) {
                ObjectManager<MountInfo>::Initialize(MOUNTSYSTEM_MAXCOUNT);
            }
            unsigned long Register(struct MountInfo *MountInfo) {
                MountInfo->MountID = ObjectManager<struct MountInfo>::Register(MountInfo);
                return MountInfo->MountID;
            }
            struct MountInfo *GetObjectByStorageID(unsigned long StorageID) {
                int i;
                for(i = 0; i < MaxCount; i++) {
                    if(ObjectContainer[i].Object->TargetStorage->ID == StorageID) {
                        return ObjectContainer[i].Object;
                    }
                }
                return 0x00;
            }
            struct MountInfo *GetObjectByInterfaceName(const char *InterfaceName) {
                int i;
                for(i = 0; i < MaxCount; i++) {
                    if(strcmp(ObjectContainer[i].Object->InterfaceName , InterfaceName) == 0) {
                        return ObjectContainer[i].Object;
                    }
                }
                return 0x00;
            }
            struct MountInfo *GetObjectByCompleteName(const char *CompleteFileName) {
                int i;
                for(i = 0; i < MaxCount; i++) {
                    if(ObjectContainer[i].Object == 0x00) {
                        continue;
                    }
                    if(strcmp(ObjectContainer[i].Object->AccessFileName , CompleteFileName) == 0) {
                        return ObjectContainer[i].Object;
                    }
                }
                return 0x00;
            }
    };
    class UniversalMountManager { // Universal Mount Manager
        friend class MountInfoManager;
        public:
            static UniversalMountManager *GetInstance(void) {
                static class UniversalMountManager *Instance;
                if(Instance == 0x00) {
                    Instance = new UniversalMountManager;
                }
                return Instance;
            }

            void Initialize(void) {
                MountedListManager = new MountInfoManager;
                MountedListManager->Initialize();
            }
            struct MountInfo *GetMountInfo(const char *CompleteFileName);
            struct MountInfo *GetMountInfo(unsigned long MountInfoID);

            struct MountInfo *RegisterInterface(const char *InterfaceName , const char *FileName , InterfaceHandler *Handler);
            struct MountInfo *MountStorage(const char *DirectoryName , struct Storage *Storage);
            bool Unmount(struct MountInfo *MountInfo);

            // Für Storage type mount

            // Get Name of mounted file
            bool GetMountedName(char *MountedName , const char *CompleteFileName);
            // Get Storage of mounted file
            struct Storage *GetMountedStorage(const char *CompleteFileName);
        private:
            MountInfoManager *MountedListManager;
    };
    void Initialize(void);
}

#endif
#ifndef _KERNELINTERFACE_HPP_
#define _KERNELINTERFACE_HPP_

#include <Kernel.hpp>
#include <Drivers/StorageDriver.hpp>
#include <Drivers/FileSystemDriver.hpp>

namespace Kernel {
    typedef bool (*InterfaceAccessorOpenFunction)(void);
    typedef bool (*InterfaceAccessorReadFunction)(unsigned char *Output);
    typedef bool (*InterfaceAccessorWriteFunction)(unsigned char *Request);
    typedef bool (*InterfaceAccessorCloseFunction)(void);
    
    struct KernelInterface {
        char *FileName; // full path of the file? or just file name?

        InterfaceAccessorOpenFunction InterfaceAccessorOpen;
        InterfaceAccessorReadFunction InterfaceAccessorRead;
        InterfaceAccessorWriteFunction InterfaceAccessorWrite;
        InterfaceAccessorCloseFunction InterfaceAccessorClose;

        unsigned long ID;

        struct KernelInterface *Next;
    };
    struct KernelInterface *AssignKernelInterface(const char *FileName , 
        InterfaceAccessorOpenFunction InterfaceAccessorOpen , 
        InterfaceAccessorReadFunction InterfaceAccessorRead , 
        InterfaceAccessorWriteFunction InterfaceAccessorWrite , 
        InterfaceAccessorCloseFunction InterfaceAccessorClose);

    class KernelInterfaceManager {
        public:
            static class KernelInterfaceManager *GetInstance(void) {
                static class KernelInterfaceManager *Instance;
                if(Instance == 0x00) {
                    Instance = (class KernelInterfaceManager *)Kernel::MemoryManagement::Allocate(sizeof(struct KernelInterfaceManager));
                }
                return Instance;
            }

            void Initialize(Drivers::StorageSystem::Storage *Storage , const char *DirectoryName) {
                static bool Initialized = false;
                if(Initialized == true) {
                    return;
                }
                this->Storage = Storage;
                if(Storage->FileSystem->CreateDir(Storage , DirectoryName) == false) {
                    Kernel::printf("Warning : Kernel Interface directory \"%s\" already exists.\n" , DirectoryName);
                }
                DirectoryInfo = Storage->FileSystem->OpenFile(Storage , DirectoryName);
                if(DirectoryInfo == 0x00) {
                    Kernel::printf("Critical Error : Failed creating kernel interface directory\n");
                }
                InterfaceIndex = 0;
                StartKernelInterface = 0x00;
                Initialized = true;
            }

            unsigned long RegisterInterface(struct KernelInterface *Interface);
            bool DeregisterInterface(unsigned long ID);
            bool DeregisterInterface(const char *FileName);

            struct KernelInterface *GetInterface(const char *FileName);
            bool IsFileInterface(struct FileSystem::FileInfo *FileInfo);
        private:
            Drivers::StorageSystem::Storage *Storage;
            struct FileSystem::FileInfo *DirectoryInfo;
            struct KernelInterface *StartKernelInterface;
            
            unsigned long InterfaceIndex = 0;
            unsigned long InterfaceCount = 0;
    };
}

#endif
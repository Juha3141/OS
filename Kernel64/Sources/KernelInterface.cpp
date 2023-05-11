#include <KernelInterface.hpp>

using namespace Kernel;

struct KernelInterface *AssignKernelInterface(const char *FileName , 
        InterfaceAccessorOpenFunction InterfaceAccessorOpen , 
        InterfaceAccessorReadFunction InterfaceAccessorRead , 
        InterfaceAccessorWriteFunction InterfaceAccessorWrite , 
        InterfaceAccessorCloseFunction InterfaceAccessorClose) {
    struct KernelInterface *Interface = (struct KernelInterface *)Kernel::MemoryManagement::Allocate(sizeof(struct KernelInterface));
    Interface->FileName = (char *)Kernel::MemoryManagement::Allocate(strlen(FileName)+1);
    strcpy(Interface->FileName , FileName);

    Interface->InterfaceAccessorOpen = InterfaceAccessorOpen;
    Interface->InterfaceAccessorRead = InterfaceAccessorRead;
    Interface->InterfaceAccessorWrite = InterfaceAccessorWrite;
    Interface->InterfaceAccessorClose = InterfaceAccessorClose;

    return Interface;
}

unsigned long KernelInterfaceManager::RegisterInterface(struct KernelInterface *Interface) {
    struct KernelInterface *Pointer = StartKernelInterface;
    if(StartKernelInterface == 0x00) {
        StartKernelInterface = Interface;
        StartKernelInterface->Next = 0x00;
        StartKernelInterface->ID = InterfaceIndex;
        InterfaceCount++;
        return InterfaceIndex++;
    }
    while(Pointer != 0x00) {
        if(Pointer->Next == 0x00) {
            break;
        }
        Pointer = Pointer->Next;
    }
    Pointer->Next = Interface;
    Interface->Next = 0x00;
    Interface->ID = InterfaceIndex;
    InterfaceCount++;
    return InterfaceIndex++;
}

bool KernelInterfaceManager::DeregisterInterface(unsigned long ID) {
    struct KernelInterface *Pointer = StartKernelInterface;
    struct KernelInterface *Previous = 0x00;
    while(Pointer != 0x00) {
        if(Pointer->Next == 0x00) {
            return false;
        }
        if(Pointer->ID == ID) {
            break;
        }
        Previous = Pointer;
        Pointer = Pointer->Next;
    }
    if(Previous == 0x00) {
        StartKernelInterface = StartKernelInterface->Next;
    }
    else {
        Previous->Next = Pointer->Next;
    }
    InterfaceCount++;
    return true;
}

struct KernelInterface *KernelInterfaceManager::GetInterface(const char *FileName) {
    struct KernelInterface *Pointer = StartKernelInterface;
    while(Pointer != 0x00) {
        if(Pointer->Next == 0x00) {
            return 0x00;
        }
        if(memcmp(Pointer->FileName , FileName , strlen(Pointer->FileName)) == 0) {
            if(strlen(Pointer->FileName) != strlen(FileName)) {
                continue;
            }
            break;
        }
        Pointer = Pointer->Next;
    }
    return Pointer;
}

bool KernelInterfaceManager::IsFileInterface(struct FileInfo *FileInfo) {
    if(Storage->ID != FileInfo->StorageID) {
        return false;
    }
    // later!
    return true;
}
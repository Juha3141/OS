#ifndef _FILESYSTEM_DRIVER_HPP_
#define _FILESYSTEM_DRIVER_HPP_

#include <Kernel.hpp>
#include <Drivers/StorageDriver.hpp>

#define FILESYSTEM_FILETYPE_PRESENT     1
#define FILESYSTEM_FILETYPE_READONLY    2
#define FILESYSTEM_FILETYPE_SYSTEM      3
#define FILESYSTEM_FILETYPE_REMOVED     4
#define FILESYSTEM_FILETYPE_HIDDEN      5

struct FileInfo {
    char *FileName;
    unsigned long SectorAddress;        // Essential(Location)
    unsigned long BlockSize;      // Bytes Per Sector

    unsigned char FileType;       // File Type

    unsigned short CreatedTime;     // Follows FAT32 date/time format
    unsigned short CreatedDate;
    unsigned short LastAccessedDate;
    unsigned short LastWrittenTime;
    unsigned short LastWrittenDate;

    unsigned long FileSize;
    unsigned long FileOffset;

    unsigned int StorageID;
    unsigned int StorageDriverID;
};
struct FileSystemDriver {
    friend class FileSystemDriverManager;
    virtual bool Check(struct Storage *Storage) = 0; // true : The storage has this file system, false : The storage has different file system.
    virtual bool CreateFile(struct Storage *Storage , const char *FileName) = 0; // true : The storage has this file system, false : The storage has different file system.
    virtual bool CreateDir(struct Storage *Storage , const char *DirectoryName) = 0;

    virtual struct FileInfo *OpenFile(struct Storage *Storage , const char *FileName) = 0;
    virtual int CloseFile(struct Storage *Storage , struct FileInfo *FileInfo) = 0;
    virtual int RemoveFile(struct Storage *Storage , struct FileInfo *FileInfo) = 0;
        
    virtual int WriteFile(struct Storage *Storage , struct FileInfo *FileInfo , unsigned long Size , void *Buffer) = 0;
    virtual int ReadFile(struct Storage *Storage , struct FileInfo *FileInfo , unsigned long Size , void *Buffer) = 0;
        
    virtual int ReadDirectory(struct Storage *Storage , struct FileInfo *FileInfo , struct FileInfo *FileList) = 0;
    virtual int GetFileCountInDirectory(struct Storage *Storage , struct FileInfo *FileInfo) = 0;

    char FileSystemString[32];
};
class FileSystemManager : public ObjectManager<struct FileSystemDriver> {
    public:
        static FileSystemManager *GetInstance(void) { // Singleton object
            static class FileSystemManager *Instance;
            if(Instance == 0x00) {
                Instance = new FileSystemManager;
            }
            return Instance;
        }
        void Initialize(void) {
            ObjectManager<struct FileSystemDriver>::Initialize(256);
            ObjectContainer = (struct ObjectManager::ObjectContainer *)MemoryManagement::Allocate(sizeof(struct ObjectManager::ObjectContainer)*MaxObjectCount);
        }

        FileSystemDriver *GetObjectByName(const char *FileSystemString) {
            int i;
            for(i = 0; i < MaxObjectCount; i++) {
                if(strlen(ObjectContainer[i].Object->FileSystemString) != strlen(FileSystemString)) {
                    continue;
                }
                if(memcmp(ObjectContainer[i].Object->FileSystemString , FileSystemString , strlen(ObjectContainer[i].Object->FileSystemString)) == 0) {
                    return ObjectContainer[i].Object;
                }
            }
            return 0x00;
        }
        bool Check(int ID , struct Storage *Storage) {
            if(ObjectContainer[ID].Using == false) {
                return false;
            }
            if(ID >= MaxObjectCount) {
                return false;
            }
            return ObjectContainer[ID].Object->Check(Storage);
        }

        unsigned int GetDriverCount(void) {
            return CurrentObjectCount;
        }
};

namespace FileSystem {
    void Initialize(void);

    bool Register(FileSystemDriver *Driver , const char *FileSystemString);
    FileSystemDriver *Search(unsigned long ID);
    FileSystemDriver *Search(const char *FileSystemString);
    FileSystemDriver *DetectFileSystem(struct Storage *Storage);
}

#endif
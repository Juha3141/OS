#ifndef _FILESYSTEM_DRIVER_HPP_
#define _FILESYSTEM_DRIVER_HPP_

#include <Kernel.hpp>
#include <Drivers/StorageDriver.hpp>

#define FILESYSTEM_FILETYPE_PRESENT     1
#define FILESYSTEM_FILETYPE_READONLY    2
#define FILESYSTEM_FILETYPE_SYSTEM      3
#define FILESYSTEM_FILETYPE_SYSDIR      4
#define FILESYSTEM_FILETYPE_REMOVED     5
#define FILESYSTEM_FILETYPE_HIDDEN      6
#define FILESYSTEM_FILETYPE_DIRECTORY   7

#define FILESYSTEM_FILETYPE_INTERFACE   8
#define FILESYSTEM_FILETYPE_MOUNTED     9

#define FILESYSTEM_OPEN_NEW                1 // Just create entirely new file and write
#define FILESYSTEM_OPEN_OVERWRITE          2 // Overwrite from the current offset
#define FILESYSTEM_OPEN_READ               3 // Read only

#define FILESYSTEM_OFFSET_SET               1 // Set the offset to given offset
#define FILESYSTEM_OFFSET_INCREASE          2 // Increase offset by given offset
#define FILESYSTEM_OFFSET_END               3 // 

struct FileInfo {
    char *FileName;
    unsigned long Location;             // Essential(Location) in Sector
    unsigned long SubdirectoryLocation;

    unsigned long BlockSize;            // Bytes Per Sector
    unsigned char FileType;             // File Type
    unsigned char OpenOption;

    unsigned short CreatedTime;     // Follows FAT32 date/time format
    unsigned short CreatedDate;
    unsigned short LastAccessedDate;
    unsigned short LastWrittenTime;
    unsigned short LastWrittenDate;

    unsigned long FileSize;
    unsigned long FileOffset;

    struct Storage *Storage;
};

struct DirectoryInfo {
    char *DirectoryName;
    unsigned long Location;
    unsigned long SubdirectoryLocation;

    unsigned short CreatedTime;     // Follows FAT32 date/time format
    unsigned short CreatedDate;
    unsigned short LastAccessedDate;
    unsigned short LastWrittenTime;
    unsigned short LastWrittenDate;

    unsigned long FileSize;
    unsigned long FileOffset;

    struct Storage *Storage;
};

struct FileSystemDriver {
    friend class FileSystemDriverManager;
    virtual bool Check(struct Storage *Storage) = 0; // true : The storage has this file system, false : The storage has different file system.
    virtual bool CreateFile(struct Storage *Storage , const char *FileName) = 0; // true : The storage has this file system, false : The storage has different file system.
    virtual bool CreateDir(struct Storage *Storage , const char *DirectoryName) = 0;

    virtual void SetOffset(struct FileInfo *FileInfo , int Offset , int OffsetOption) {
        switch(OffsetOption) {
            case FILESYSTEM_OFFSET_SET:
                FileInfo->FileOffset = Offset;
                break;
            case FILESYSTEM_OFFSET_INCREASE:
                if(FileInfo->FileOffset+Offset > FileInfo->FileSize) {
                    FileInfo->FileOffset = FileInfo->FileSize-1;
                    break;
                }
                if(FileInfo->FileOffset+Offset < 0) {
                    FileInfo->FileOffset = 0;
                }
                FileInfo->FileOffset += Offset;
                break;
            case FILESYSTEM_OFFSET_END:
                FileInfo->FileOffset = FileInfo->FileSize-1;
                break;
        }
    }

    virtual struct FileInfo *OpenFile(struct Storage *Storage , const char *FileName , int OpenOption) = 0;
    virtual int CloseFile(struct FileInfo *FileInfo) = 0;
    virtual bool RemoveFile(struct FileInfo *FileInfo) = 0;
        
    virtual int WriteFile(struct FileInfo *FileInfo , unsigned long Size , void *Buffer) = 0;
    virtual int ReadFile(struct FileInfo *FileInfo , unsigned long Size , void *Buffer) = 0;
        
    virtual int ReadDirectory(struct FileInfo *FileInfo , struct FileInfo **FileList) = 0;
    virtual int GetFileCountInDirectory(struct FileInfo *FileInfo) = 0;

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
            ObjectContainer = (struct ObjectManager::ObjectContainer *)MemoryManagement::Allocate(sizeof(struct ObjectManager::ObjectContainer)*MaxCount);
        }

        FileSystemDriver *GetObjectByName(const char *FileSystemString) {
            int i;
            for(i = 0; i < MaxCount; i++) {
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
            if(ID >= MaxCount) {
                return false;
            }
            return ObjectContainer[ID].Object->Check(Storage);
        }

        unsigned int GetDriverCount(void) {
            return Count;
        }
        //////////////  //////////////////
        struct Storage *HeadStorage;

};

namespace FileSystem {
    void Initialize(void);

    bool Register(FileSystemDriver *Driver , const char *FileSystemString);
    FileSystemDriver *Search(unsigned long ID);
    FileSystemDriver *Search(const char *FileSystemString);
    FileSystemDriver *DetectFileSystem(struct Storage *Storage);

    ///////////////////////////////////////////////////////////////////////////

    int GetDirectoryCount(const char *FileName);
    int ParseDirectories(const char *FileName , char **DirectoryNames);
    char ParseCharacter(void);
    char *HeadDirectory(void);
    char *Parse(const char *FileName , char Character);
    void RemoveUnnecessarySpaces(char *String);

    void SetHeadStorage(struct Storage *Storage);

    ///////////////////////////////////////////////////////////////////////////
    bool CreateFile(const char *FileName); // true : The storage has this file system, false : The storage has different file system.
    bool CreateDir(const char *DirectoryName);

    void SetOffset(struct FileInfo *FileInfo , int Offset , int OffsetOption);

    struct FileInfo *OpenFile(const char *FileName , int OpenOption);
    int CloseFile(struct FileInfo *FileInfo);
    bool RemoveFile(struct FileInfo *FileInfo);
        
    int WriteFile(struct FileInfo *FileInfo , unsigned long Size , void *Buffer);
    int ReadFile(struct FileInfo *FileInfo , unsigned long Size , void *Buffer);
        
    int ReadDirectory(struct FileInfo *FileInfo , struct FileInfo **FileList);
    int GetFileCountInDirectory(struct FileInfo *FileInfo);
}

#endif
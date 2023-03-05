#ifndef _FILESYSTEM_DRIVER_HPP_
#define _FILESYSTEM_DRIVER_HPP_

#include <Kernel.hpp>
#include <Drivers/StorageDriver.hpp>

#define FILESYSTEM_FILETYPE_PRESENT     1
#define FILESYSTEM_FILETYPE_READONLY    2
#define FILESYSTEM_FILETYPE_SYSTEM      3
#define FILESYSTEM_FILETYPE_REMOVED     4
#define FILESYSTEM_FILETYPE_HIDDEN      5

namespace Kernel {
    namespace FileSystem {
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

            unsigned int StorageDriverID;
            unsigned int StorageID;
        };
        struct DirectoryInfo {
            FileInfo BasicInfo;
            FileInfo **Tree;
        };
        typedef bool (*StandardCheckFunction)(Drivers::StorageSystem::Storage *Storage); // true : The storage has this file system, false : The storage has different file system.
        typedef bool (*StandardCreateFileFunction)(Drivers::StorageSystem::Storage *Storage , const char *FileName); // true : The storage has this file system, false : The storage has different file system.
        typedef bool (*StandardCreateDirFunction)(Drivers::StorageSystem::Storage *Storage , const char *DirectoryName);

        typedef FileSystem::FileInfo * (*StandardOpenFileFunction)(Drivers::StorageSystem::Storage *Storage , const char *FileName);
        typedef int (*StandardCloseFileFunction)(Drivers::StorageSystem::Storage *Storage , FileSystem::FileInfo *FileInfo);
        typedef int (*StandardRemoveFileFunction)(Drivers::StorageSystem::Storage *Storage , FileSystem::FileInfo *FileInfo);
        
        typedef int (*StandardWriteFileFunction)(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo , unsigned long Size , void *Buffer);
        typedef int (*StandardReadFileFunction)(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo , unsigned long Size , void *Buffer);
        
        typedef int (*StandardReadDirectoryFunction)(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo , struct FileSystem::FileInfo *FileList);
        typedef int (*StandardGetFileCountInDirectoryFunction)(Drivers::StorageSystem::Storage *Storage , struct FileSystem::FileInfo *FileInfo);
        struct Standard {
            char FileSystemString[32];

            StandardCheckFunction Check; // true : The storage has this file system, false : The storage has different file system.
            StandardCreateFileFunction CreateFile;
            StandardCreateDirFunction CreateDir;

            StandardOpenFileFunction OpenFile;
            StandardCloseFileFunction CloseFile;
            StandardRemoveFileFunction RemoveFile;
            
            StandardWriteFileFunction WriteFile;
            StandardReadFileFunction ReadFile;
            
            StandardReadDirectoryFunction ReadDirectory;
            StandardGetFileCountInDirectoryFunction GetFileCountInDirectory;
        };
        void Initialize(void);
        FileSystem::Standard *AssignSystem(
        StandardCheckFunction Check , 
        StandardCreateFileFunction CreateFile , 
        StandardCreateDirFunction CreateDir , 
        
        StandardOpenFileFunction OpenFile , 
        StandardCloseFileFunction CloseFile , 
        StandardRemoveFileFunction RemoveFile , 
        
        StandardWriteFileFunction WriteFile , 
        StandardReadFileFunction ReadFile , 

        StandardReadDirectoryFunction ReadDirectory , 
        StandardGetFileCountInDirectoryFunction GetFileCountInDirectory);

        bool Register(FileSystem::Standard *FileSystem , const char *FileSystemString);
        FileSystem::Standard *Search(unsigned long ID);
        FileSystem::Standard *Search(const char *FileSystemString);
        FileSystem::Standard *DetectFileSystem(Drivers::StorageSystem::Storage *Storage);
    }
}

#endif
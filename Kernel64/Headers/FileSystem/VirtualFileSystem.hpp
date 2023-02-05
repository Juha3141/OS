#ifndef _VIRTUALFILESYSTEM_HPP_
#define _VIRTUALFILESYSTEM_HPP_

#include <Drivers/DeviceDriver.hpp>
#include <Drivers/StorageDriver.hpp>
#include <Drivers/FileSystemDriver.hpp>

namespace Kernel {
    namespace FileSystem {
        FileInfo *OpenFile(const char *FileName , const char *Mode);
        int CloseFile(FileInfo *FileInfo);
        int RemoveFile(FileInfo *FileInfo);
        int WriteFile(FileInfo *FileInfo , unsigned long Size , void *Buffer);
        int ReadFile(FileInfo *FileInfo , unsigned long Size , void *Buffer);
        int SetFileOffset(FileInfo *FileInfo , unsigned long Offset , unsigned int Set);
        
        int ReadDirectory(FileInfo *FileInfo , struct FileSystem::FileInfo *FileList);
        int GetFileCountInDirectory(FileInfo *FileInfo);
        
    };
};

#endif
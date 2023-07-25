#include <Drivers/FileSystemDriver.hpp>
#include <Drivers/StorageDriver.hpp>
#include <FileSystem/MountSystem.hpp>

void FileSystem::Initialize(void) {
    FileSystemManager::GetInstance()->Initialize();
}

bool FileSystem::Register(FileSystemDriver *Driver , const char *FileSystemString) {
    strcpy(Driver->FileSystemString , FileSystemString);
    if(FileSystemManager::GetInstance()->Register(Driver) == false) {
        return false;
    }
    return true;
}

FileSystemDriver *FileSystem::Search(unsigned long ID) {
    return FileSystemManager::GetInstance()->GetObject(ID);
}

FileSystemDriver *FileSystem::Search(const char *FileSystemString) {
    return FileSystemManager::GetInstance()->GetObjectByName(FileSystemString);
}

FileSystemDriver *FileSystem::DetectFileSystem(struct Storage *Storage) {
    int i;
    FileSystemManager *Manager = FileSystemManager::GetInstance();
    for(i = 0; i < Manager->Count; i++) {
        if(Manager->Check(i , Storage) == true) {
            return Manager->GetObject(i);
        }
    }
    return 0x00;
}


// Get number of directory entered from file name
// Basically, you get the number of '/' in the string
int FileSystem::GetDirectoryCount(const char *FileName) {
    int i;
    int DirectoryCount = 0;
    for(i = 0; FileName[i] != 0; i++) {
        if(FileName[i] == '/') {
            DirectoryCount++;  
        }
    }
    return DirectoryCount;
}

/// @brief Parse name of the directories from a full file name
/// Precondition : Pointer of directories should be already allocated.
/// @param FileName A full file name that consists of full directory paths
/// @param Directories A pointer of buffer where the list of directory is saved
/// @return Number of directories
int FileSystem::ParseDirectories(const char *FileName , char **Directories) {
    // Auto-allocates
    int i;
    int j = 0;
    int PreviousPoint = 0;
    int DirectoryCount;
    if((DirectoryCount = GetDirectoryCount(FileName)) == 0) {
        return 0;
    }
    for(i = 0; FileName[i] != 0; i++) {
        if(FileName[i] == '/') {
            Directories[j] = (char *)MemoryManagement::Allocate(i-PreviousPoint+1);
            strncpy(Directories[j] , FileName+PreviousPoint , i-PreviousPoint);
            PreviousPoint = i+1;
            j++;
        }
    }
    Directories[j] = (char *)MemoryManagement::Allocate(i-PreviousPoint+1);
    strncpy(Directories[j] , FileName+PreviousPoint , i-PreviousPoint);
    return DirectoryCount;
}

char FileSystem::ParseCharacter(void) {
    return '/';
}


char *FileSystem::HeadDirectory(void) {
    return "@";
}

char *FileSystem::Parse(const char *FileName , char Character) {
    int i;
    for(i = 0; FileName[i] != 0; i++) {
        if((FileName[i] == Character)) {
            break;
        }
    }
    char *NewParsed;
    NewParsed = (char *)MemoryManagement::Allocate(i+1);
    strncpy(NewParsed , FileName , i);
    return NewParsed;
}


void FileSystem::RemoveUnnecessarySpaces(char *String) {
    int i;
    int j;
    if(String[strlen(String)-1] == ' ') {
        for(i = strlen(String)-1; String[i] == ' '; i--) {
            String[i] = 0x00;
        }
    }
    if(String[0] == ' ') {
        for(i = 0; String[i] == ' '; i++) {
            String[i] = 0x00;
        }
        memcpy(String , String+i , strlen(String));
    }
    for(i = 0; String[i] != 0; i++) {
        if(String[i] == ' ') {
            if(String[i+1] == ' ') {
                while(String[i+1] == ' ') {
                    memcpy(String+i , String+i+1 , strlen(String)-i);
                }
            }
        }
    }
}

void FileSystem::SetHeadStorage(struct Storage *Storage) {
    FileSystemManager::GetInstance()->HeadStorage = Storage;
    MountSystem::UniversalMountManager::GetInstance()->MountStorage(HeadDirectory() , Storage);
}

static struct Storage *GetInfoFromFileName(char *PartialFileName , const char *FileName) {
    struct Storage *Storage;
    int CurrentDirLength = ((TaskManagement::GetCurrentDirectoryLocation() == 0x00) ? 0 : strlen(TaskManagement::GetCurrentDirectoryLocation()));
    char CompleteFileName[strlen(FileName)+1+CurrentDirLength+2];
    memset(CompleteFileName , 0 , sizeof(CompleteFileName));
    if(strncmp(FileName , FileSystem::HeadDirectory() , 1) != 0) {
        strcpy(CompleteFileName , TaskManagement::GetCurrentDirectoryLocation());
        CompleteFileName[strlen(CompleteFileName)+1] = 0x00;
        CompleteFileName[strlen(CompleteFileName)] = FileSystem::ParseCharacter();
    }
    strcat(CompleteFileName , FileName);
    // printf("%s\n" , CompleteFileName);
    if((Storage = MountSystem::UniversalMountManager::GetInstance()->GetMountedStorage(CompleteFileName)) == 0x00) {
        return 0x00;
    }
    if(MountSystem::UniversalMountManager::GetInstance()->GetMountedName(PartialFileName , CompleteFileName) == false) {
        return 0x00;
    }
    /*
    printf("Storage         : 0x%X(%s%d)\n" , Storage , Storage->Driver->DriverName , Storage->ID);
    printf("PartialFileName : \"%s\"\n" , PartialFileName);
    printf("FileSystem : 0x%X(%s)\n" , Storage->FileSystem , Storage->FileSystem->FileSystemString);
    */
    return Storage;
}

// Common use
bool FileSystem::CreateFile(const char *FileName) {
    int CurrentDirLength = ((TaskManagement::GetCurrentDirectoryLocation() == 0x00) ? 0 : strlen(TaskManagement::GetCurrentDirectoryLocation()));
    char PartialFileName[strlen(FileName)+1+CurrentDirLength+2];
    struct Storage *Storage;
    if((Storage = GetInfoFromFileName(PartialFileName , FileName)) == 0) {
        return 0x00;
    }
    if(Storage->FileSystem == 0x00) {
        return 0x00;
    }
    return Storage->FileSystem->CreateFile(Storage , PartialFileName);
}

bool FileSystem::CreateDir(const char *DirectoryName) {
    int CurrentDirLength = ((TaskManagement::GetCurrentDirectoryLocation() == 0x00) ? 0 : strlen(TaskManagement::GetCurrentDirectoryLocation()));
    char PartialFileName[strlen(DirectoryName)+1+CurrentDirLength+2];
    struct Storage *Storage;
    if((Storage = GetInfoFromFileName(PartialFileName , DirectoryName)) == 0) {
        return 0x00;
    }
    if(Storage->FileSystem == 0x00) {
        return 0x00;
    }
    return Storage->FileSystem->CreateDir(Storage , PartialFileName);
}

void FileSystem::SetOffset(struct FileInfo *FileInfo , int Offset , int OffsetOption) {
    FileInfo->Storage->FileSystem->SetOffset(FileInfo , Offset , OffsetOption);
}

struct FileInfo *FileSystem::OpenFile(const char *FileName , int OpenOption) {
    struct FileInfo *File;
    int CurrentDirLength = ((TaskManagement::GetCurrentDirectoryLocation() == 0x00) ? 0 : strlen(TaskManagement::GetCurrentDirectoryLocation()));
    char PartialFileName[strlen(FileName)+1+CurrentDirLength+2];
    struct Storage *Storage;
    if((Storage = GetInfoFromFileName(PartialFileName , FileName)) == 0) {
        return 0x00;
    }
    if(Storage->FileSystem == 0x00) {
        return 0x00;
    }
    return Storage->FileSystem->OpenFile(Storage , PartialFileName , OpenOption);
}

int FileSystem::CloseFile(struct FileInfo *FileInfo) {
    return FileInfo->Storage->FileSystem->CloseFile(FileInfo);
}

bool FileSystem::RemoveFile(struct FileInfo *FileInfo) {
    return FileInfo->Storage->FileSystem->RemoveFile(FileInfo);
}
    
int FileSystem::WriteFile(struct FileInfo *FileInfo , unsigned long Size , void *Buffer) {
    return FileInfo->Storage->FileSystem->WriteFile(FileInfo , Size , Buffer);
}

int FileSystem::ReadFile(struct FileInfo *FileInfo , unsigned long Size , void *Buffer) {
    return FileInfo->Storage->FileSystem->ReadFile(FileInfo , Size , Buffer);
}

int FileSystem::ReadDirectory(struct FileInfo *FileInfo , struct FileInfo **FileList) {
    return FileInfo->Storage->FileSystem->ReadDirectory(FileInfo , FileList);
}

int FileSystem::GetFileCountInDirectory(struct FileInfo *FileInfo) {
    return FileInfo->Storage->FileSystem->GetFileCountInDirectory(FileInfo);
}
#include <Drivers/FileSystemDriver.hpp>
#include <Drivers/StorageDriver.hpp>

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Universal File Convention(Only in this OS)                                                            //
//                                                                                                       //
// [head directory name]/[storage driver name][storage id number][Partition parse character]/[File name] //
// = @/[storage name][id]:[partition id]/[file name]                                                     //
//                                                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////////////

char FileSystem::ParseCharacter(void) {
    return '/';
}

char FileSystem::PartitionParseCharacter(void) {
    return ':';
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

char *FileSystem::GetStorageDriverName(const char *FileName) {
    int i;
    char *FullDrvName;
    if(strncmp(FileName , HeadDirectory() , 1) != 0) {
        return 0x00;
    }
    FileName += (strlen(HeadDirectory())+1);
    for(i = 0; FileName[i] != 0; i++) {
        if((FileName[i] == ParseCharacter())||(FileName[i] == PartitionParseCharacter())) {
            break;
        }
    }
    FullDrvName = (char *)MemoryManagement::Allocate(i+1);
    strncpy(FullDrvName , FileName , i);
    for(i = strlen(FullDrvName)-1; ((FullDrvName[i] >= '0') && (FullDrvName[i] <= '9')); i--) {
        FullDrvName[i] = 0;
    }
    return FullDrvName;
}

int FileSystem::GetStorageID(const char *FileName) {
    int i;
    int j = 0;
    char *FullDrvName;
    int ID;
    if(strncmp(FileName , HeadDirectory() , 1) != 0) {
        return 0x00;
    }
    FileName += (strlen(HeadDirectory())+1);
    for(i = 0; FileName[i] != 0; i++) {
        if((FileName[i] == ParseCharacter())||(FileName[i] == PartitionParseCharacter())) {
            break;
        }
    }
    FullDrvName = (char *)MemoryManagement::Allocate(i+1);
    strncpy(FullDrvName , FileName , i);

    char Number[strlen(FullDrvName)];
    
    for(i = strlen(FullDrvName)-1; ((FullDrvName[i] >= '0') && (FullDrvName[i] <= '9')); i--) {
        Number[strlen(FullDrvName)-(++j)] = FullDrvName[i];
    }
    Number[strlen(FullDrvName)] = 0;
    
    MemoryManagement::Free(FullDrvName);
    if(j == 0) {
        return STORAGESYSTEM_INVALIDID;
    }
    ID = atoi(Number+(strlen(FullDrvName)-j));
    return ID;
}

int FileSystem::GetStoragePartitionID(const char *FileName) {
    int i;
    char *IDPart;
    int ID;
    for(i = 0; FileName[i] != 0; i++) {
        if(FileName[i] == PartitionParseCharacter()) {
            break;
        }
    }
    if(FileName[i] == 0) {
        return STORAGESYSTEM_INVALIDID;
    }
    if(FileName[i+1] == '/') {
        return STORAGESYSTEM_INVALIDID;
    }
    IDPart = FileSystem::Parse(FileName+i+1 , ParseCharacter());
    ID = atoi(IDPart);
    MemoryManagement::Free(IDPart);
    return ID;
}

bool FileSystem::GetPartialFileName(char *PartialFileName , const char *FileName) {
    int i;
    char *IDPart;
    int ID;
    if(strncmp(FileName , HeadDirectory() , 1) != 0) {
        strcpy(PartialFileName , "");
        return false;
    }
    FileName += (strlen(HeadDirectory())+1);
    for(i = 0; FileName[i] != 0; i++) {
        if((FileName[i] == PartitionParseCharacter())||(FileName[i] == ParseCharacter())) {
            break;
        }
    }
    for(; FileName[i] != 0; i++) {
        if((FileName[i] == ParseCharacter())) {
            break;
        }
    }
    if(FileName[i] == 0) {
        strcpy(PartialFileName , "");
        return true;
    }
    else {
        strcpy(PartialFileName , FileName+i+1);
        return true;
    }
}

struct FileInfo *FileSystem::GetHeadDirectory(void) {
    static struct FileInfo *FileInfo = 0x00;
    if(FileInfo == 0x00) {
        FileInfo = (struct FileInfo *)MemoryManagement::Allocate(sizeof(struct FileInfo));
        memset(FileInfo , 0 , sizeof(struct FileInfo));
        FileInfo->FileName = (char *)MemoryManagement::Allocate(strlen(HeadDirectory())+1);
        strcpy(FileInfo->FileName , HeadDirectory());
        FileInfo->FileType = FILESYSTEM_FILETYPE_SYSDIR;
    }
    return FileInfo;
}

struct Storage *FileSystem::GetStorage(const char *FileName) {
    char *DriverName;
    unsigned long StorageID;
    unsigned long PartitionID;
    struct Storage *Storage;
    int DirectoryCount = GetDirectoryCount(FileName);
    if(DirectoryCount == 0) { // bad file name
        return 0x00;
    }
    if((DriverName = GetStorageDriverName(FileName)) == 0x00) {
        return 0x00;
    }
    StorageID = GetStorageID(FileName);
    PartitionID = GetStoragePartitionID(FileName);
    if(StorageID == STORAGESYSTEM_INVALIDID) {
        MemoryManagement::Free(DriverName);
        return 0x00;
    }
    if((Storage = StorageSystem::SearchStorage(DriverName , StorageID)) == 0x00) {
        MemoryManagement::Free(DriverName);
        return 0x00;
    }
    if(PartitionID == STORAGESYSTEM_INVALIDID) {
        if(Storage->FileSystem == 0x00) {
            MemoryManagement::Free(DriverName);
            return 0x00;
        }
    }
    else {
        Storage = Storage->LogicalStorages->GetObject(PartitionID);
    }
    MemoryManagement::Free(DriverName);
    return Storage;
}

static struct Storage *GetInfoFromFileName(char *PartialFileName , const char *FileName) {
    int i;
    int k = 0;
    struct Storage *Storage;
    char FullName[strlen(FileName)+1+strlen(TaskManagement::GetCurrentDirectoryLocation())+2];
    if(memcmp(FileName , FileSystem::HeadDirectory() , 1) != 0) {
        sprintf(FullName , "%s/%s" , TaskManagement::GetCurrentDirectoryLocation() , FileName);

        for(i = (strlen(FileSystem::HeadDirectory())+1); FullName[i] != 0; i++) {
            if((FullName[i] == FileSystem::PartitionParseCharacter())||(FullName[i] == FileSystem::ParseCharacter())) {
                break;
            }
        }
        for(; FullName[i] != 0; i++) {
            if((FullName[i] == FileSystem::ParseCharacter())) {
                break;
            }
        }
        strcpy(PartialFileName , FullName+i+1);
        if((Storage = FileSystem::GetStorage(TaskManagement::GetCurrentDirectoryLocation())) == 0x00) {
            return 0x00;
        }
        return Storage;
    }
    if((Storage = FileSystem::GetStorage(FileName)) == 0x00) {
        return 0x00;
    }
    if(FileSystem::GetPartialFileName(PartialFileName , FileName) == false) {
        return 0x00;
    }
    return Storage;
}

// Common use
bool FileSystem::CreateFile(const char *FileName) {
    char PartialFileName[strlen(FileName)+1+strlen(TaskManagement::GetCurrentDirectoryLocation())+2];
    struct Storage *Storage;
    if(strcmp(FileName , HeadDirectory()) == 0) {
        return GetHeadDirectory();
    }
    if((Storage = GetInfoFromFileName(PartialFileName , FileName)) == 0) {
        return 0x00;
    }
    return Storage->FileSystem->CreateFile(Storage , PartialFileName);
}

bool FileSystem::CreateDir(const char *DirectoryName) {
    char PartialFileName[strlen(DirectoryName)+1+strlen(TaskManagement::GetCurrentDirectoryLocation())+2];
    struct Storage *Storage;
    if(strcmp(DirectoryName , HeadDirectory()) == 0) {
        return GetHeadDirectory();
    }
    if((Storage = GetInfoFromFileName(PartialFileName , DirectoryName)) == 0) {
        return 0x00;
    }
    return Storage->FileSystem->CreateDir(Storage , PartialFileName);
}

void FileSystem::SetOffset(struct FileInfo *FileInfo , int Offset , int OffsetOption) {
    FileInfo->Storage->FileSystem->SetOffset(FileInfo , Offset , OffsetOption);
}

struct FileInfo *FileSystem::OpenFile(const char *FileName , int OpenOption) {
    struct FileInfo *File;
    char PartialFileName[strlen(FileName)+1+strlen(TaskManagement::GetCurrentDirectoryLocation())+2];
    struct Storage *Storage;
    if(strcmp(FileName , HeadDirectory()) == 0) {
        return GetHeadDirectory();
    }
    if((Storage = GetInfoFromFileName(PartialFileName , FileName)) == 0) {
        return 0x00;
    }
    return Storage->FileSystem->OpenFile(Storage , PartialFileName , OpenOption);
}

int FileSystem::CloseFile(struct FileInfo *FileInfo) {
    return FileInfo->Storage->FileSystem->CloseFile(FileInfo);
}

int FileSystem::RemoveFile(struct FileInfo *FileInfo) {
    return FileInfo->Storage->FileSystem->RemoveFile(FileInfo);
}
    
int FileSystem::WriteFile(struct FileInfo *FileInfo , unsigned long Size , void *Buffer) {
    return FileInfo->Storage->FileSystem->WriteFile(FileInfo , Size , Buffer);
}

int FileSystem::ReadFile(struct FileInfo *FileInfo , unsigned long Size , void *Buffer) {
    return FileInfo->Storage->FileSystem->ReadFile(FileInfo , Size , Buffer);
}

static int AddStorageToList(struct FileInfo **FileList , struct Storage *Storage , int Index) {
    int i;
    struct Storage *Partition;

    if(Storage->LogicalStorages == 0x00) {
        if(Storage->FileSystem == 0x00) {
            return Index;
        }
        FileList[Index] = (struct FileInfo *)MemoryManagement::Allocate(sizeof(struct FileInfo));
        memset(FileList[Index] , 0 , sizeof(struct FileInfo));
        FileList[Index]->FileName = (char *)MemoryManagement::Allocate(128);
        sprintf(FileList[Index]->FileName , "%s%d" , Storage->Driver->DriverName , Storage->ID);
        FileList[Index]->Storage = Storage;
        FileList[Index]->FileType = FILESYSTEM_FILETYPE_SYSDIR;
        return Index+1;
    }
    for(i = 0; i < Storage->LogicalStorages->MaxCount; i++) {
        if((Partition = Storage->LogicalStorages->GetObject(i)) == 0) {
            continue;
        }
        FileList[Index] = (struct FileInfo *)MemoryManagement::Allocate(sizeof(struct FileInfo));
        memset(FileList[Index] , 0 , sizeof(struct FileInfo));
        FileList[Index]->FileName = (char *)MemoryManagement::Allocate(128);
        // printf("Driver name : %s\n" , Partition->Driver->DriverName);
        sprintf(FileList[Index]->FileName , "%s%d:%d" , Partition->Driver->DriverName , Partition->ID , Partition->PartitionID);
        FileList[Index]->Storage = Partition;
        FileList[Index]->FileType = FILESYSTEM_FILETYPE_SYSDIR;
        Index++;
    }
    return Index;
}

int FileSystem::ReadDirectory(struct FileInfo *FileInfo , struct FileInfo **FileList) {
    int i;
    int j;
    int k = 0;
    struct Storage *Storage;
    struct StorageDriver *Driver;
    class StorageDriverManager *DriverManager = StorageDriverManager::GetInstance();
    if(FileInfo != GetHeadDirectory()) {
        return FileInfo->Storage->FileSystem->ReadDirectory(FileInfo , FileList);
    }
    for(i = 0; i < DriverManager->MaxCount; i++) {
        if((Driver = DriverManager->GetObject(i)) == 0x00) {
            continue;
        }
        for(j = 0; j < Driver->StorageManager->MaxCount; j++) {
            if((Storage = Driver->StorageManager->GetObject(j)) == 0x00) {
                continue;
            }
            k = AddStorageToList(FileList , Storage , k);
        }
    }
    return k;
}

int FileSystem::GetFileCountInDirectory(struct FileInfo *FileInfo) {
    int i;
    int j;
    int k = 0;
    struct Storage *Storage;
    struct StorageDriver *Driver;
    class StorageDriverManager *DriverManager = StorageDriverManager::GetInstance();
    if(FileInfo != GetHeadDirectory()) {
        return FileInfo->Storage->FileSystem->GetFileCountInDirectory(FileInfo);
    }
    for(i = 0; i < DriverManager->MaxCount; i++) {
        if((Driver = DriverManager->GetObject(i)) == 0x00) {
            continue;
        }
        for(j = 0; j < Driver->StorageManager->MaxCount; j++) {
            if((Storage = Driver->StorageManager->GetObject(j)) == 0x00) {
                continue;
            }
            if(Storage->LogicalStorages == 0x00) {
                if(Storage->FileSystem == 0x00) {
                    continue;
                }
                k++;
            }
            else {
                k += Storage->LogicalStorages->Count;
            }
        }
    }
    return k;
}
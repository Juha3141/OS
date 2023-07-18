#include <FileSystem/MountSystem.hpp>

void MountSystem::Initialize(void) {
    UniversalMountManager::GetInstance()->Initialize();
}

struct MountSystem::MountInfo *MountSystem::UniversalMountManager::GetMountInfo(const char *CompleteFileName) {
    int i;
    struct MountInfo *FinalMountInfo = 0x00;
    struct MountInfo *MountInfo = 0x00;
    char TempName[strlen(CompleteFileName)+1];
    for(i = 0; CompleteFileName[i] != 0; i++) {
        if(CompleteFileName[i] == FileSystem::ParseCharacter()) {
            strncpy(TempName , CompleteFileName , i);
            MountInfo = MountedListManager->GetObjectByCompleteName(TempName);
            FinalMountInfo = (MountInfo == 0x00) ? FinalMountInfo : MountInfo;
        }
    }
    MountInfo = MountedListManager->GetObjectByCompleteName(CompleteFileName);
    FinalMountInfo = (MountInfo == 0x00) ? FinalMountInfo : MountInfo;
    if(FinalMountInfo->Type == MountInfo::Type::SystemInterface) {
        return 0x00;
    }
    return FinalMountInfo;
}

struct MountSystem::MountInfo *MountSystem::UniversalMountManager::GetMountInfo(unsigned long MountInfoID) {
    return MountedListManager->GetObject(MountInfoID);
}

struct MountSystem::MountInfo *MountSystem::UniversalMountManager::RegisterInterface(const char *InterfaceName , const char *FileName , MountSystem::InterfaceHandler *Handler) {
    struct MountInfo *MountInfo = (struct MountInfo *)MemoryManagement::Allocate(sizeof(struct MountInfo));
    MountInfo->Type = MountInfo::SystemInterface;
    MountInfo->Handler = Handler;
    
    strcpy(MountInfo->InterfaceName , InterfaceName);
    strcpy(MountInfo->AccessFileName , FileName);
    
    MountedListManager->Register(MountInfo);
    return MountInfo;
}

struct MountSystem::MountInfo *MountSystem::UniversalMountManager::MountStorage(const char *DirectoryName , struct Storage *Storage) {
    int i;
    struct MountInfo *MountInfo = (struct MountInfo *)MemoryManagement::Allocate(sizeof(struct MountInfo));
    if(Storage->FileSystem == 0x00) {
        return 0x00;
    }
    if(Storage->IsMounted == true) {
        return 0x00;
    }
    if(MountedListManager->GetObjectByCompleteName(DirectoryName) != 0x00) {
        return 0x00;
    }
    MountInfo->Type = MountInfo::StorageLink;
    MountInfo->TargetStorage = Storage;

    strcpy(MountInfo->AccessFileName , DirectoryName);
    
    MountedListManager->Register(MountInfo);

    Storage->IsMounted = true;
    Storage->MountInfoID = MountInfo->MountID;
    return MountInfo;
}

bool MountSystem::UniversalMountManager::Unmount(struct MountInfo *MountInfo) {
    return MountedListManager->Deregister(MountInfo);
}

/// @brief Get actual name (Storage name) of mounted file name
/// @param CompleteFileName Complete name of the file
/// @return Mounted file name
bool MountSystem::UniversalMountManager::GetMountedName(char *MountedName , const char *CompleteFileName) {
    struct MountInfo *MountInfo;
    if((MountInfo = GetMountInfo(CompleteFileName)) == 0x00) {
        return false;
    }
    if(MountInfo->Type == MountInfo::Type::SystemInterface) {
        return false;
    }
    strcpy(MountedName , CompleteFileName+strlen(MountInfo->AccessFileName)+1);

    return true;
}

struct Storage *MountSystem::UniversalMountManager::GetMountedStorage(const char *CompleteFileName) {
    struct MountInfo *MountInfo;
    if((MountInfo = GetMountInfo(CompleteFileName)) == 0x00) {
        return 0x00;
    }
    if(MountInfo->Type == MountInfo::Type::SystemInterface) {
        return 0x00;
    }
    return MountInfo->TargetStorage;
}
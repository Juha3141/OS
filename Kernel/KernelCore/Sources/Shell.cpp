#include <MutualExclusion.hpp>

#include <StorageDriver.hpp>
#include <FileSystemDriver.hpp>

#include <RAMDisk.hpp>
#include <IDE.hpp>
#include <PCI.hpp>
#include <BootRAMDisk.hpp>

#include <MBR.hpp>
#include <GPT.hpp>
#include <MountSystem.hpp>

#include <ISO9660.hpp>
#include <FAT16.hpp>
#include <Shell.hpp>

namespace Shell {
    namespace DefaultCommands {
        void createfile(int argc , char **argv);
        void createdir(int argc , char **argv);
        void removefile(int argc , char **argv);
        void writefile(int argc , char **argv);
        void readfile(int argc , char **argv);
        void ls(int argc , char **argv);

        void storagelist(void);
        void mount(int argc , char **argv);
        
        void mem(void);
        void memmap(void);
        
        void tasklist(void);
        void terminate(int argc , char **argv);
        void changetasktime(int argc , char **argv);
        void spinner(int argc , char **argv);

        void cpuinfo(int argc , char **argv);
    };
};

void Shell::DefaultCommands::createfile(int argc , char **argv) {
    struct FileInfo *File;
    if(argc != 2) {
        printf("Usage : %s [file name]\n" , argv[0]);
        return;
    }
    File = FileSystem::OpenFile(argv[1] , FILESYSTEM_OPEN_READ);
    if(File != 0x00) {
        printf("File already exists\n");
        MemoryManagement::Free(File);
        return;
    }
    if(FileSystem::CreateFile(argv[1]) == false) {
        printf("Failed creating file\n");
        return;
    }
    printf("File created\n");
}

void Shell::DefaultCommands::createdir(int argc , char **argv) {
    struct FileInfo *File;
    if(argc != 2) {
        printf("Usage : %s [file name]\n" , argv[0]);
        return;
    }
    File = FileSystem::OpenFile(argv[1] , FILESYSTEM_OPEN_READ);
    if(File != 0x00) {
        printf("File already exists\n");
        MemoryManagement::Free(File);
        return;
    }
    if(FileSystem::CreateDir(argv[1]) == false) {
        printf("Failed creating directory\n");
        return;
    }
}

void Shell::DefaultCommands::removefile(int argc , char **argv) {
    struct FileInfo *File;
    if(argc != 2) {
        printf("Usage : %s [file name]\n" , argv[0]);
        return;
    }
    File = FileSystem::OpenFile(argv[1] , FILESYSTEM_OPEN_READ);
    if(File == 0x00) {
        printf("File not found\n");
        MemoryManagement::Free(File);
        return;
    }
    if(FileSystem::RemoveFile(File) == false) {
        printf("Failed removing file\n");
        MemoryManagement::Free(File);
        return;
    }
    printf("Succeed removing file\n");
    MemoryManagement::Free(File);
}

void Shell::DefaultCommands::writefile(int argc , char **argv) {
    
}

void Shell::DefaultCommands::readfile(int argc , char **argv) {
    unsigned int i;
    unsigned int j;
    int KeyData;
    int X;
    int Y;
    int PreviousX;
    int PreviousY;
    int Line = 0;
    struct FileInfo *File;
    char *FileData;

    char KeyInput;
    if(argc != 2) {
        printf("Usage : %s [file name]\n" , argv[0]);
        return;
    }
    File = FileSystem::OpenFile(argv[1] , FILESYSTEM_OPEN_READ);
    if(File == 0) {
        printf("File \"%s\" not found\n" , argv[1]);
        return;
    }
    if((File->FileType == FILESYSTEM_FILETYPE_DIRECTORY)||(File->FileType == FILESYSTEM_FILETYPE_SYSDIR)) {
        printf("File \"%s\" is a directory\n" , argv[1]);
        MemoryManagement::Free(File);
        return;
    }
    if(File->FileSize == 0) {
        MemoryManagement::Free(File);
        return;
    }
    FileData = (char*)MemoryManagement::Allocate(File->FileSize);
    if(FileSystem::ReadFile(File , File->FileSize , FileData) == 0) {
        printf("Failed reading file \"%s\"\n" , argv[1]);
        MemoryManagement::Free(File);
        return;
    }
    FileData[File->FileSize] = 0x00;
    for(i = j = 0; i < File->FileSize; i++) {
        if(Line >= 5) {
            Line = 0;
            GetScreenInformation(&PreviousX , &PreviousY , 0 , 0);
            printf("\n");
            GetScreenInformation(&X , &Y , 0 , 0);
            printf("(q/esc to escape) : ");
            KeyInput = Keyboard::GetASCIIData();
            SetPosition(0 , PreviousY);
            printf("                    ");
            SetPosition(PreviousX , PreviousY-1);
            if((KeyInput == 'q')||(KeyInput == KEYBOARD_KEY_ESC)) {
                return;
            }
        }
        printf("%c" , FileData[i]);
        if(FileData[i] == '\n') {
            Line++;
        }
        if(FileData[i] != '\n') {
            j++;
        }
        if((j != 0) && (j%80 == 0)) {
            Line++;
        }
    }
    printf("\n");
    MemoryManagement::Free(FileData);
    MemoryManagement::Free(File);
    return;

}

void Shell::DefaultCommands::ls(int argc , char **argv) {
    int i;
    int X;
    int Y;
    int FileCount;
    int MaxFileNameLength = 0;
    struct FileInfo *File = 0x00;
    struct FileInfo **FileList;

    char KeyInput;
    char *Directory;
    if(argc == 2) {
        Directory = argv[1];
    }
    else {
        Directory = TaskManagement::GetCurrentDirectoryLocation();
    }
    
    
    File = FileSystem::OpenFile(Directory , FILESYSTEM_OPEN_READ);
    if(File == 0) {
        printf("No such directory named \"%s\"\n" , Directory);
        return;
    }
    if((File->FileType != FILESYSTEM_FILETYPE_DIRECTORY) && (File->FileType != FILESYSTEM_FILETYPE_SYSDIR)) {
        printf("\"%s\" is not a directory\n" , Directory);
        return;
    }
    FileCount = FileSystem::GetFileCountInDirectory(File);
    if(FileCount == 0) {
        printf("This directory is completely empty somehow\n");
        return;
    }
    printf("File count : %d\n" , FileCount);
    FileList = (struct FileInfo **)MemoryManagement::Allocate(FileCount*sizeof(FileInfo *));
    FileSystem::ReadDirectory(File , FileList);
    for(i = 0; i < FileCount; i++) {
        if(MaxFileNameLength < strlen(FileList[i]->FileName)) { // error??
            MaxFileNameLength = strlen(FileList[i]->FileName);
        }
    }
    for(i = 0; i < FileCount; i++) {
        printf("%s" , FileList[i]->FileName);
        GetScreenInformation(&(X) , &(Y) , 0 , 0);
        SetPosition(MaxFileNameLength+1 , Y);
        if((FileList[i]->FileType != FILESYSTEM_FILETYPE_DIRECTORY) && (FileList[i]->FileType != FILESYSTEM_FILETYPE_SYSDIR)) {
            if(FileList[i]->FileSize < 1024) {
                printf(" %dB" , FileList[i]->FileSize);
            }
            if((FileList[i]->FileSize > 1024) && (FileList[i]->FileSize < 1024*1024)) {
                printf(" %dKB" , FileList[i]->FileSize/1024);
            }
            if((FileList[i]->FileSize > 1024*1024) && (FileList[i]->FileSize < 1024*1024*1024)) {
                printf(" %dMB" , FileList[i]->FileSize/1024/1024);
            }
            if(FileList[i]->FileSize > 1024*1024*1024) {
                printf(" %dGB" , FileList[i]->FileSize/1024/1024/1024);
            }
        }
        else {
            printf(" DIR");
        }
        printf("\n");
        if((i != 0) && (i%24 == 0)) {
            printf("(q/esc to escape) : ");
            KeyInput = Keyboard::GetASCIIData();
            printf("\n");
            if((KeyInput == 'q')||(KeyInput == KEYBOARD_KEY_ESC)) {
                return;
            }
        }
        MemoryManagement::Free(FileList[i]);
    }
    MemoryManagement::Free(FileList);
    return;
}


void Shell::DefaultCommands::storagelist(void) {
    int i;
    int j;
    int k = 0;
    struct Storage *Storage;
    struct Storage *Partition;
    struct StorageDriver *Driver;
    struct MountSystem::MountInfo *MountInfo;
    class StorageDriverManager *DriverManager = StorageDriverManager::GetInstance();
    for(i = 0; i < DriverManager->MaxCount; i++) {
        if((Driver = DriverManager->GetObject(i)) == 0x00) {
            continue;
        }
        for(j = 0; j < Driver->StorageManager->MaxCount; j++) {
            if((Storage = Driver->StorageManager->GetObject(j)) == 0x00) {
                continue;
            }
            printf("%s%d" , Storage->Driver->DriverName , Storage->ID);
            if(Storage->FileSystem != 0x00) {
                printf("(%s)" , Storage->FileSystemString);
            }
            if(Storage->IsMounted == true) {
                MountInfo = MountSystem::UniversalMountManager::GetInstance()->GetMountInfo(Storage->MountInfoID);
                printf(" - Mounted to %s" , MountInfo->AccessFileName);
            }
            printf("\n");
            if(Storage->LogicalStorages != 0x00) {
                for(k = 0; k < Storage->LogicalStorages->Count; k++) {
                    Partition = Storage->LogicalStorages->GetObject(k);
                    printf("  + %s%d:%d" , Partition->Driver->DriverName , Partition->ID , Partition->PartitionID);
                    if(Partition->FileSystem != 0x00) {
                        printf("(%s)" , Partition->FileSystemString);
                    }
                    if(Partition->IsMounted == true) {
                        MountInfo = MountSystem::UniversalMountManager::GetInstance()->GetMountInfo(Partition->MountInfoID);
                        printf(" - Mounted to %s" , MountInfo->AccessFileName);
                    }
                    printf("\n");
                }
            }
        }
    }
    return;
}

void Shell::DefaultCommands::mount(int argc , char **argv) {
    struct FileInfo *FileInfo;
    struct Storage *Storage;

    unsigned long StorageID;
    unsigned long PartitionID;
    char ParseCharacterString[2] = {FileSystem::ParseCharacter() , 0};
    if(argc < 4) {
        printf("Usage : %s [directory name] [storage name] [storage id] [optional partition id]\n" , argv[0]);
        return;
    }
    int CurrentDirLength = ((TaskManagement::GetCurrentDirectoryLocation() == 0x00) ? 0 : strlen(TaskManagement::GetCurrentDirectoryLocation()));
    char FullDirectoryName[strlen(argv[1])+1+CurrentDirLength+2];
    if((FileInfo = FileSystem::OpenFile(argv[1] , FILESYSTEM_OPEN_READ)) == 0x00) {
        printf("No such directory named \"%s\"\n" , argv[1]);
        return;
    }
    FileSystem::CloseFile(FileInfo);
    StorageID = atoi(argv[3]);
    if(argc == 4) {
        PartitionID = STORAGESYSTEM_INVALIDID;
        Storage = StorageSystem::SearchStorage(argv[2] , StorageID);
    }
    else {
        PartitionID = atoi(argv[4]);
        Storage = StorageSystem::SearchStorage(argv[2] , StorageID , PartitionID);
    }
    if(Storage == 0x00) {
        printf("Couldn't find the storage\n");
        return;
    }
    if(strncmp(argv[1] , FileSystem::HeadDirectory() , 1) != 0) {
        strcpy(FullDirectoryName , TaskManagement::GetCurrentDirectoryLocation());
        strncat(FullDirectoryName , ParseCharacterString , 1);
        strcat(FullDirectoryName , argv[1]);
    }
    else {
        strcpy(FullDirectoryName , argv[1]);
    }
    if(MountSystem::UniversalMountManager::GetInstance()->MountStorage(FullDirectoryName , Storage) == 0x00) {
        printf("Mount failed.\n");
    }
}

void Shell::DefaultCommands::tasklist(void) {
    int i;
    int j;
    int k;
    int Index = 0;
    struct TaskManagement::Task *TaskPointer;
    struct TaskManagement::TaskQueue *TaskQueue;
    int KeyInput;
    for(i = 0; i < CoreInformation::GetInstance()->CoreCount; i++) {
        printf("========== Core #%d ========== \n" , i);
        for(j = 0; j < TASK_QUEUE_COUNT; j++) {
            TaskQueue = TaskManagement::GetTaskQueue(i , j);
            TaskPointer = TaskQueue->StartTask;
            for(k = 0; k < TaskQueue->TaskCount; k++) {
                if(TaskPointer->NextTask == TaskQueue->StartTask) {
                    break;
                }
                printf("%d. %s , ID : 0x%X , Given Time : %d\n" , Index , TaskPointer->Name , TaskPointer->ID , TaskPointer->DemandTime);
                TaskPointer = TaskPointer->NextTask;
                Index++;
                if(Index%24 == 0) {
                    printf("(q/esc to escape) : ");
                    KeyInput = Keyboard::GetASCIIData();
                    printf("\n");
                    if((KeyInput == 'q')||(KeyInput == KEYBOARD_KEY_ESC)) {
                        return;
                    }
                }
            }
        }
    }
    return;
}

void Shell::DefaultCommands::terminate(int argc , char **argv) {
    unsigned long TaskID;
    struct TaskManagement::Task *Task;
    if(argc != 2) {
        printf("Usage : %s [Task ID or name of the task]\n" , argv[0]);
        return;
    }
    if(!((argv[1][0] >= '0') && (argv[1][0] <= '9'))) {
        Task = TaskManagement::GetTask(argv[1]);
        if(Task == 0x00) {
            printf("There's no task named \"%s\"\n" , argv[1]);
            return;
        }
        TaskID = Task->ID;
    }
    else {
        if(memcmp(argv[1] , "0x" , 2) == 0) {
            TaskID = atol(argv[1]+2);
        }
        else {
            TaskID = atoi(argv[1]);
        }
        if((Task = TaskManagement::GetTask(TaskID)) == 0x00) {
            printf("Task not found\n");
            return;
        }
    }
    if(TaskManagement::TerminateTask(TaskID) == false) {
        printf("Failed terminating the task\n");
        return;
    }
    else {
        printf("Successfully terminated task \"%s\"\n" , Task->Name);
    }
}

void Shell::DefaultCommands::changetasktime(int argc , char **argv) {
    unsigned long TaskID;
    unsigned long OldTime;
    unsigned long Time;
    struct TaskManagement::Task *Task;
    if(argc != 3) {
        printf("Usage : %s [Task ID or name of the task] [time(cycle)]\n" , argv[0]);
        return;
    }
    if(!((argv[1][0] >= '0') && (argv[1][0] <= '9'))) {
        Task = TaskManagement::GetTask(argv[1]);
        if(Task == 0x00) {
            printf("There's no task named \"%s\"\n" , argv[1]);
            return;
        }
        TaskID = Task->ID;
    }
    else {
        if(memcmp(argv[1] , "0x" , 2) == 0) {
            TaskID = atol(argv[1]+2);
        }
        else {
            TaskID = atoi(argv[1]);
        }
        if((Task = TaskManagement::GetTask(TaskID)) == 0x00) {
            printf("Task not found\n");
            return;
        }
    }
    Time = atoi(argv[2]);
    OldTime = Task->DemandTime;
    TaskManagement::ChangeDemandTime(TaskID , Time);
    printf("Changed task time from : %d, to : %d\n" , OldTime , Time);
    return;
}

int Index = 0;

void testtask(void) {
    int i = 0;
    int MyIndex = 0;
    unsigned char *VideoMemory = (unsigned char *)0xB8000;
    unsigned char Spinner[4] = {'-' , '/' , '|' , '\\'};
    MyIndex = Index++;
    while(1) {
        VideoMemory[(80*25*2)-((MyIndex+1)*2)] = Spinner[i%4];
        i++;
    }
}

void Shell::DefaultCommands::spinner(int argc , char **argv) {
    int i;
    int TaskCount;
    if(argc != 2) {
        printf("Usage : %s [task count]\n" , argv[0]);
        return;
    }
    TaskCount = atoi(argv[1]);
    for(i = 0; i < TaskCount; i++) {
        TaskManagement::CreateTask((unsigned long)testtask , TASK_FLAGS_PRIVILAGE_KERNEL , TASK_STATUS_RUNNING , 2048 , "testtask" , TaskManagement::GetCurrentDirectoryLocation());
    }
    return;
}

void Shell::DefaultCommands::mem(void) {
    MemoryManagement::NodeManager *NodeManager = (MemoryManagement::NodeManager *)MEMORYMANAGEMENT_MEMORY_STARTADDRESS;
    if((NodeManager->TotalUsableMemory >= 1024*1024) && (NodeManager->TotalUsableMemory < 1024*1024*1024)) {
        printf("Total Memory    : %dMB\n" , NodeManager->TotalUsableMemory/1024/1024);
    }
    if(NodeManager->TotalUsableMemory >= 1024*1024*1024) {
        printf("Total Memory    : %dGB\n" , NodeManager->TotalUsableMemory/1024/1024/1024);
    }
    if(NodeManager->CurrentlyUsingMemory < 1024) {
        printf("Currently Using : %dB\n" , NodeManager->CurrentlyUsingMemory);
    }
    if((NodeManager->CurrentlyUsingMemory >= 1024) && (NodeManager->CurrentlyUsingMemory < 1024*1024)) {
        printf("Currently Using : %dKB\n" , NodeManager->CurrentlyUsingMemory/1024);
    }
    if((NodeManager->CurrentlyUsingMemory >= 1024*1024) && (NodeManager->CurrentlyUsingMemory < 1024*1024*1024)) {
        printf("Currently Using : %dMB\n" , NodeManager->CurrentlyUsingMemory/1024/1024);
    }
    if(NodeManager->CurrentlyUsingMemory >= 1024*1024*1024) {
        printf("Currently Using : %dGB\n" , NodeManager->CurrentlyUsingMemory/1024/1024/1024);
    }
    return;
}

void Shell::DefaultCommands::memmap(void) {
    int i = 0;
	MemoryManagement::QuerySystemAddressMap *E820 = (MemoryManagement::QuerySystemAddressMap *)MEMORYMANAGEMENT_E820_ADDRESS;
	while(1) {
		// If entry type is zero, we reached the end of the table.
		if(E820[i].Type == 0x00) {
			break;
		}
		printf("0x%X-0x%X   " , E820[i].Address , E820[i].Address+E820[i].Length);
        switch(E820[i].Type) {
            case 1:
                printf("Available");
                break;
            case 2:
                printf("Reserved");
                break;
            case 3:
                printf("ACPI Reclaimable");
                break;
            case 4:
                printf("ACPI NVS Memory");
                break;
            case 5:
                printf("Bad Memory");
                break;
        }
        printf("\n");
		i += 1;
	}
}

void Shell::DefaultCommands::cpuinfo(int argc , char **argv) {
    CoreInformation *CPUInfo = CoreInformation::GetInstance();
    printf("Number of Cores    : %d\n" , CPUInfo->CoreCount);
    printf("IO APIC Address    : 0x%X\n" , CPUInfo->IOAPICAddress);
    printf("Local APIC Address : 0x%X\n" , CPUInfo->LocalAPICAddress);
}

void Shell::ShellSystem::AddBasicCommands(void) {
    CommandList.Initialize(256);
    CommandList.AddCommand("createdir" , "Creates directory" , (unsigned long)DefaultCommands::createdir);
    CommandList.AddCommand("createfile" , "Creates file" , (unsigned long)DefaultCommands::createfile);
    CommandList.AddCommand("removefile" , "Removes file" , (unsigned long)DefaultCommands::removefile);
    CommandList.AddCommand("writefile" , "Writes file" , (unsigned long)DefaultCommands::writefile);
    CommandList.AddCommand("readfile" , "Reads file" , (unsigned long)DefaultCommands::readfile);
    CommandList.AddCommand("ls" , "Lists content in current/targetted directory" , (unsigned long)DefaultCommands::ls);
    CommandList.AddCommand("storagelist" , "Lists registered storage to system" , (unsigned long)DefaultCommands::storagelist);
    CommandList.AddCommand("mount" , "Mounts storage to a directory" , (unsigned long)DefaultCommands::mount);

    CommandList.AddCommand("mem" , "Shows status of memory usage" , (unsigned long)DefaultCommands::mem);
    CommandList.AddCommand("memmap" , "Shows the memory map" , (unsigned long)DefaultCommands::memmap);
    
    CommandList.AddCommand("tasklist" , "Lists currently running tasks" , (unsigned long)DefaultCommands::tasklist);
    CommandList.AddCommand("terminate" , "Terminates task by given ID" , (unsigned long )DefaultCommands::terminate);
    CommandList.AddCommand("changetasktime" , "Changes give task time" , (unsigned long )DefaultCommands::changetasktime);
    CommandList.AddCommand("spinner" , "Creates task that da cool spinin'" , (unsigned long)DefaultCommands::spinner);

    CommandList.AddCommand("cpuinfo" , "Shows information of CPU" , (unsigned long)DefaultCommands::cpuinfo);
}

void Shell::ShellSystem::Start(void) {
    int i;
    int j;
    int X;
    int Y;
    int IsCommandDuplicated = 0;
    int ASCIIData;
    Command = (char *)MemoryManagement::Allocate(8192);
    CurrentDirectory = (char *)MemoryManagement::Allocate(512);
    strcpy(CurrentDirectory , "@");
    History.Initialize();
    this->AddBasicCommands();

    TaskManagement::ChangeDemandTime(TaskManagement::GetCurrentlyRunningTaskID() , 15);
    while(1) {
        History.Index = History.CurrentHistoryCount-1;
        Offset = 0;
        MaxOffset = 0;
        IsCommandDuplicated = 0;
        memset(Command , 0 , 8192);
        printf("%s> " , CurrentDirectory);
        GetScreenInformation(&(DefaultPositionX) , &(DefaultPositionY) , 0 , 0);
        while(1) {
            ASCIIData = Keyboard::GetASCIIData();
            if(ASCIIData == KEYBOARD_KEY_LEFT) {
                this->ProcessKeyboardLeft();
                continue;
            }
            else if(ASCIIData == KEYBOARD_KEY_RIGHT) {
                this->ProcessKeyboardRight();
                continue;
            }
            else if(ASCIIData == KEYBOARD_KEY_UP) {
                IsCommandDuplicated = 1;
                SetPosition(DefaultPositionX , DefaultPositionY);
                for(i = 0; Command[i] != 0; i++) {
                    printf(" ");
                }
                History.View(Command);
                History.Up();
                SetPosition(DefaultPositionX , DefaultPositionY);
                printf("%s" , Command);
                Offset = strlen(Command);
                MaxOffset = strlen(Command);
                continue;
            }
            else if(ASCIIData == KEYBOARD_KEY_DOWN) {
                IsCommandDuplicated = 1;
                SetPosition(DefaultPositionX , DefaultPositionY);
                for(i = 0; Command[i] != 0; i++) {
                    printf(" ");
                }
                History.View(Command);
                History.Down();
                SetPosition(DefaultPositionX , DefaultPositionY);
                printf("%s" , Command);
                Offset = strlen(Command);
                MaxOffset = strlen(Command);
                continue;
            }
            else if(ASCIIData == KEYBOARD_KEY_INSERT) {
                InsertKeyPressed = ((InsertKeyPressed == 0) ? 1 : 0);
                continue;
            }
            else if(ASCIIData == '\b') {
                this->ProcessKeyboardBackspace();
                continue;
            }
            else if(ASCIIData == '\n') {
                if(MaxOffset == 0) {
                    printf("\n");
                    break;
                }
                Command[MaxOffset] = 0x00;
                if(History.CurrentHistoryCount != 0) {
                    if(strcmp(Command , History.Histories[History.CurrentHistoryCount-1]) != 0) {
                        History.Index = History.CurrentHistoryCount-1;
                        History.Add(Command);
                    }
                }
                else {
                    History.Index = History.CurrentHistoryCount-1;
                    History.Add(Command);
                }
                printf("\n");
                ProcessCommand();
                break;
            }
            else {
                if(InsertKeyPressed == 1) {
                    this->Insert_ProcessDefault(ASCIIData);
                }
                else {
                    this->ProcessDefault(ASCIIData);
                }
            }
        }
    }
}

void Shell::ShellSystem::ProcessKeyboardLeft(void) {
    if(Offset > 0) {
        MovePosition(-1 , 0);
        Offset--;
    }
}

void Shell::ShellSystem::ProcessKeyboardRight(void) {
    Offset++;
    if(Offset > MaxOffset) {
        Offset = MaxOffset;
    }
    else {
        MovePosition(1 , 0);
    }
}

void Shell::ShellSystem::ProcessKeyboardBackspace(void) {
    int i;
    int X;
    int Y;
    if(Offset > 0) {
        GetScreenInformation(&(X) , &(Y) , 0 , 0);
        if(Offset < MaxOffset) {
            for(i = Offset-1; i < MaxOffset; i++) {
                Command[i] = Command[i+1];
            }
            Command[MaxOffset] = 0x00;
            SetPosition(DefaultPositionX , DefaultPositionY);
            printf("%s " , Command);
            SetPosition(X-1 , Y);
            Offset -= 1;
            MaxOffset -= 1;
            return;
        }
        printf("\b");
        Offset--;
        MaxOffset--;
        Command[Offset] = 0x00;
        if(X == 0) {
            if(Y == 0) {
                return;
            }
            X = 80;
            Y -= 1;
            SetPosition(X , Y);
        }
    }
}

void Shell::CommandListSystem::Initialize(int MaxCount)  {
    int i;
    this->MaxCommandsCount = MaxCount;
    CommandEntryPoint = (unsigned long*)MemoryManagement::Allocate(MaxCount*sizeof(unsigned long));
    CommandNames = (char **)MemoryManagement::Allocate(MaxCount*sizeof(char *));
    CommandHelpMessages = (char **)MemoryManagement::Allocate(MaxCount*sizeof(char *));
    for(i = 0; i < MaxCommandsCount; i++) {
        CommandEntryPoint[i] = 0x00;
        CommandNames[i] = 0x00;
        CommandHelpMessages[i] = 0x00;
    }
}

void Shell::CommandListSystem::AddCommand(const char *Name , const char *HelpMessage , unsigned long EntryPoint) {
    CommandEntryPoint[CommandsCount] = EntryPoint;
    CommandNames[CommandsCount] = (char *)MemoryManagement::Allocate(strlen(Name)+1);
    CommandHelpMessages[CommandsCount] = (char *)MemoryManagement::Allocate(strlen(HelpMessage)+1);
    strcpy(CommandNames[CommandsCount] , Name);
    strcpy(CommandHelpMessages[CommandsCount] , HelpMessage);
    CommandsCount++;
}

unsigned long Shell::CommandListSystem::EntryPoint(char *Name) {
    int i;
    for(i = 0; i < this->MaxCommandsCount; i++) {
        if(this->CommandNames[i] == 0x00) {
            continue;
        }
        if(strcmp(Name , this->CommandNames[i]) == 0) {
            break;
        }
    }
    if(i == this->MaxCommandsCount) {
        return 0x00;
    }
    return this->CommandEntryPoint[i];
}

void Shell::ShellSystem::ProcessDefault(int ASCIIData) {
    int i;
    int X;
    int Y;
    int PrintX;
    int PrintY;
    if(Offset < MaxOffset) {
        GetScreenInformation(&(X) , &(Y) , 0 , 0);
        for(i = 0; i < MaxOffset-Offset; i++) {
            Command[MaxOffset-i] = Command[MaxOffset-i-1];
        }
        Command[Offset] = ASCIIData;
        SetPosition(DefaultPositionX , DefaultPositionY);
        printf("%s" , Command);
        SetPosition(X+1 , Y);
        Offset++;
        MaxOffset++;
        return;
    }
    printf("%c" , ASCIIData);
    Command[Offset] = ASCIIData;
    Offset++;
    MaxOffset++;
    return;
}

void Shell::ShellSystem::Insert_ProcessDefault(int ASCIIData) {
    printf("%c" , ASCIIData);
    Command[Offset] = ASCIIData;
    Offset++;
    MaxOffset++;
    return;
}

static void AdjustFileName(char *FileName) {
    if(FileName[strlen(FileName)-1] == '/') {
        FileName[strlen(FileName)-1] = 0;
    }
}

void Shell::ShellSystem::ChangeDirectory(int SlicedCommandCount , char **SlicedCommand) {
    int i;
    struct FileInfo *File;
    if(strcmp(SlicedCommand[1] , ".") == 0) {
        return;
    }

    if(strcmp(SlicedCommand[1] , "..") == 0) {
        for(i = strlen(CurrentDirectory)-1; i >= 0; i--) {
            if(CurrentDirectory[i] == FileSystem::ParseCharacter()) {
                CurrentDirectory[i] = 0;
                break;
            }
        }
        return;
    }
    if(SlicedCommandCount == 1) {
        printf("%s\n" , CurrentDirectory);
        return;
    }
    if(SlicedCommand[1][0] != '@') {
        i = strlen(CurrentDirectory);
        strcat(CurrentDirectory , "/");
        strcat(CurrentDirectory , SlicedCommand[1]);
    }
    else {
        i = 0;
        strcpy(CurrentDirectory , SlicedCommand[1]);
    }
    AdjustFileName(CurrentDirectory);
    if((File = FileSystem::OpenFile(CurrentDirectory , FILESYSTEM_OPEN_READ)) == 0) {
        printf("Directory not found\n");
        CurrentDirectory[i] = 0;
        return;
    }
    if((File->FileType != FILESYSTEM_FILETYPE_SYSDIR) && (File->FileType != FILESYSTEM_FILETYPE_DIRECTORY)) {
        printf("This is not a directory\n");
        CurrentDirectory[i] = 0;
    }
    MemoryManagement::Free(File);
}

void Shell::ShellSystem::ProcessCommand(void) {
    int i;
    int j;
    int k;
    int l;
    int AllocatedCount = 0;
    int SlicedCommandCount = 0;
    int LongCommandSpaceCount = 0;
    int KeyboardData;
    unsigned long EntryPoint;
    unsigned long ID;
    unsigned long TemproryKeyboardData;
    unsigned long ArgumentForCommandFunction[4]; // <- PLEASE CHANGE THIS IF YOU ARE CHANGING LINE 467!!!
    int CommandLength;
    struct FileInfo *File;
    FileSystem::RemoveUnnecessarySpaces(Command);
    SlicedCommandCount = 0;
    for(i = 0; Command[i] != 0; i++) {
        if(Command[i] == ' ') {
            SlicedCommandCount++;
        }
    }
    SlicedCommandCount++;

    char *SlicedCommand[SlicedCommandCount];
    
    for(i = j = 0; i < SlicedCommandCount; i++) {
        for(CommandLength = j; Command[j] != ' '; j++) {
            if(Command[j] == 0x00) {
                break;
            }
        }
        CommandLength = (++j)-CommandLength;
        if(Command[j-CommandLength] == '"') {
            for(k = j-CommandLength+1; Command[k] != '"'; k++) {
                if(Command[k] == 0x00) {
                    break;
                }
            }
            SlicedCommand[i] = (char *)MemoryManagement::Allocate(k-(j-CommandLength+1)+1024);
            AllocatedCount++;
            memcpy(SlicedCommand[i] , Command+j-CommandLength+1 , k-(j-CommandLength+1));
            SlicedCommand[i][k-(j-CommandLength+1)] = 0x00;
            j = k+2;
            for(l = 0; SlicedCommand[i][l] != 0; l++) {
                LongCommandSpaceCount += ((SlicedCommand[i][l] == ' ') ? 1 : 0);
            }
        }
        else {
            SlicedCommand[i] = (char *)MemoryManagement::Allocate(CommandLength+1+1024);
            AllocatedCount++;
            memcpy(SlicedCommand[i] , Command+j-CommandLength , CommandLength);
            SlicedCommand[i][CommandLength-1] = 0x00;
        }
    }
    SlicedCommandCount -= LongCommandSpaceCount;
    // error
    EntryPoint = CommandList.EntryPoint(SlicedCommand[0]);
    ArgumentForCommandFunction[0] = (unsigned long)SlicedCommandCount;
    ArgumentForCommandFunction[1] = (unsigned long)SlicedCommand;
    ArgumentForCommandFunction[2] = (unsigned long)&(this->CommandList);
    ArgumentForCommandFunction[3] = (unsigned long)CurrentDirectory;
    if(strcmp(SlicedCommand[0] , "cd") == 0) {
        ChangeDirectory(SlicedCommandCount , SlicedCommand);
    }
    else if(strcmp(SlicedCommand[0] , "clear") == 0) {
        ClearScreen(0x00 , 0x07);
    }
    else if(strcmp(SlicedCommand[0] , "help") == 0) {
        int i;
        int j;
        int MaxX = 0;
        int X = 0;
        for(i = 0; i < this->CommandList.GetCommandsCount(); i++) {
            if(MaxX < strlen(this->CommandList.GetCommandNameList()[i])) {
                MaxX = strlen(this->CommandList.GetCommandNameList()[i]);
            }
        }
        for(i = 0; i < this->CommandList.GetCommandsCount(); i++) {
            printf("%s" , this->CommandList.GetCommandNameList()[i]);
            GetScreenInformation(&X , 0 , 0 , 0);
            for(j = 0; j < MaxX-X; j++) {
                printf(" ");
            }
            printf(" | %s\n" , this->CommandList.GetCommandHelpMessageList()[i]);
        }
    }
    else if(EntryPoint == 0) {
        printf("Command not found\n");
    }
    else {
        ID = TaskManagement::CreateTask((unsigned long)EntryPoint , TASK_FLAGS_PRIVILAGE_KERNEL , TASK_STATUS_RUNNING , 2*1024*1024 , "com" , CurrentDirectory , 4 , ArgumentForCommandFunction);
        TaskManagement::ChangeDemandTime(ID , 10);
        while(1) {
            if(TaskManagement::GetTask(ID) == 0x00) {
                break;
            }
        }
    }
    for(i = 0; i < SlicedCommandCount; i++) {
        MemoryManagement::Free(SlicedCommand[i]);
    }
    return;
}
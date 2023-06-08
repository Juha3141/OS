#include <MutualExclusion.hpp>

#include <Drivers/StorageDriver.hpp>
#include <Drivers/FileSystemDriver.hpp>

#include <Drivers/RAMDisk.hpp>
#include <Drivers/IDE.hpp>
#include <Drivers/PCI.hpp>
#include <Drivers/BootRAMDisk.hpp>

#include <FileSystem/MBR.hpp>
#include <FileSystem/GPT.hpp>

#include <FileSystem/ISO9660.hpp>
#include <FileSystem/FAT16.hpp>
#include <Shell.hpp>

namespace Shell {
    namespace DefaultCommands {
        void createfile(int argc , char **argv);
        void createdir(int argc , char **argv);
        void writefile(int argc , char **argv);
        void readfile(int argc , char **argv);
        void listfile(int argc , char **argv);
        
        void mem(void);
        void testmemalloc(void);
        
        void tasklist(void);
        void terminate(int argc , char **argv);
        void changetasktime(int argc , char **argv);
        void spinner(int argc , char **argv);
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
            SetPosition(PreviousX , PreviousY-1);
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

void Shell::DefaultCommands::listfile(int argc , char **argv) {
    int i;
    int X;
    int Y;
    int FileCount;
    int MaxFileNameLength = 0;
    struct FileInfo *File = 0x00;
    struct FileInfo **FileList;

    char KeyInput;
    if(argc != 2) {
        printf("Usage : %s [directory name]\n" , argv[0]);
        return;
    }

    File = FileSystem::OpenFile(argv[1] , FILESYSTEM_OPEN_READ);
    if(File == 0) {
        printf("No such directory named \"%s\"\n" , argv[1]);
        return;
    }
    if((File->FileType != FILESYSTEM_FILETYPE_DIRECTORY) && (File->FileType != FILESYSTEM_FILETYPE_SYSDIR)) {
        printf("\"%s\" is not a directory\n" , argv[1]);
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


void Shell::DefaultCommands::tasklist(void) {
    int i;
    int j;
    int k;
    int Index = 0;
    struct TaskManagement::Task *TaskPointer;
    struct TaskManagement::TaskQueue *TaskQueue;
    int KeyInput;
    for(i = 0; i < CoreInformation::GetInstance()->CoreCount; i++) {
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
        TaskManagement::CreateTask((unsigned long)testtask , TASK_FLAGS_PRIVILAGE_KERNEL , TASK_STATUS_RUNNING , 2048 , "testtask");
    }
    return;
}

void Shell::DefaultCommands::testmemalloc(void) {
    unsigned long Block8KB;
    unsigned long Block8MB;
    unsigned long Block16MB;
    unsigned long Block32MB;
    unsigned long Block64MB;
    unsigned long Block128MB;
    unsigned long Block256MB;
    unsigned long Block512MB;
    unsigned long Block1GB;
    printf("8KB Allocating : ");
    Block8KB = (unsigned long)MemoryManagement::Allocate(8192);
    printf("Located at 0x%X\n" , Block8KB);
    printf("8MB Allocating : ");
    Block8MB = (unsigned long)MemoryManagement::Allocate(8192*1024);
    printf("Located at 0x%X\n" , Block8MB);
    printf("16MB Allocating : ");
    Block16MB = (unsigned long)MemoryManagement::Allocate(16*1024*1024);
    printf("Located at 0x%X\n" , Block16MB);
    printf("32MB Allocating : ");
    Block32MB = (unsigned long)MemoryManagement::Allocate(32*1024*1024);
    printf("Located at 0x%X\n" , Block32MB);
    printf("64MB Allocating : ");
    Block64MB = (unsigned long)MemoryManagement::Allocate(64*1024*1024);
    printf("Located at 0x%X\n" , Block64MB);
    printf("128MB Allocating : ");
    Block128MB = (unsigned long)MemoryManagement::Allocate(128*1024*1024);
    printf("Located at 0x%X\n" , Block128MB);
    printf("256MB Allocating : ");
    Block256MB = (unsigned long)MemoryManagement::Allocate(256*1024*1024);
    printf("Located at 0x%X\n" , Block256MB);
    printf("512MB Allocating : ");
    Block512MB = (unsigned long)MemoryManagement::Allocate(512*1024*1024);
    printf("Located at 0x%X\n" , Block512MB);
    printf("1GB Allocating : ");
    Block1GB = (unsigned long)MemoryManagement::Allocate(1024*1024*1024);
    printf("Located at 0x%X\n" , Block1GB);
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

void Shell::ShellSystem::AddBasicCommands(void) {
    CommandList.Initialize(256);
    CommandList.AddCommand("createdir" , "Create directory" , (unsigned long)DefaultCommands::createdir);
    CommandList.AddCommand("createfile" , "Create file" , (unsigned long)DefaultCommands::createfile);
    CommandList.AddCommand("writefile" , "Write file" , (unsigned long)DefaultCommands::writefile);
    CommandList.AddCommand("readfile" , "Read file" , (unsigned long)DefaultCommands::readfile);
    CommandList.AddCommand("listfile" , "List content in current/targetted directory" , (unsigned long)DefaultCommands::listfile);
    
    CommandList.AddCommand("mem" , "Check status of memory usage" , (unsigned long)DefaultCommands::mem);
    CommandList.AddCommand("testmemalloc" , "Test Memory Allocation System" , (unsigned long)DefaultCommands::testmemalloc);
    
    CommandList.AddCommand("tasklist" , "List currently running tasks" , (unsigned long)DefaultCommands::tasklist);
    CommandList.AddCommand("terminate" , "Terminate task by given ID" , (unsigned long )DefaultCommands::terminate);
    CommandList.AddCommand("changetasktime" , "Change give task time" , (unsigned long )DefaultCommands::changetasktime);
    CommandList.AddCommand("spinner" , "Create task that da cool spinin'" , (unsigned long)DefaultCommands::spinner);
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
    this->MaxCommandsCount = MaxCount;
    CommandEntryPoint = (unsigned long*)MemoryManagement::Allocate(MaxCommandsCount*sizeof(unsigned long));
    CommandNames = (char **)MemoryManagement::Allocate(MaxCount*sizeof(char *));
    CommandHelpMessages = (char **)MemoryManagement::Allocate(MaxCount*sizeof(char *));
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
        if(memcmp(Name , this->CommandNames[i] , strlen(this->CommandNames[i])) == 0) {
            break;
        }
    }
    if(i == this->MaxCommandsCount) {
        return 0;
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
    EntryPoint = CommandList.EntryPoint(SlicedCommand[0]);
    ArgumentForCommandFunction[0] = (unsigned long)SlicedCommandCount;
    ArgumentForCommandFunction[1] = (unsigned long)SlicedCommand;
    ArgumentForCommandFunction[2] = (unsigned long)&(this->CommandList);
    ArgumentForCommandFunction[3] = (unsigned long)CurrentDirectory;
    if(strcmp(SlicedCommand[0] , "cd") == 0) {
        printf("Changing directory to : %s\n" , SlicedCommand[1]);
        printf("Current directory     : %s\n" , CurrentDirectory);
        if(SlicedCommand[1][0] != '@') {
            strcat(CurrentDirectory , "/");
            strcat(CurrentDirectory , SlicedCommand[1]);
        }
        else {
            strcpy(CurrentDirectory , SlicedCommand[1]);
        }
        AdjustFileName(CurrentDirectory);
        printf("Changed directory     : %s\n" , CurrentDirectory);
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
        ID = TaskManagement::CreateTask((unsigned long)EntryPoint , TASK_FLAGS_PRIVILAGE_KERNEL , TASK_STATUS_RUNNING , 8*1024 , "testing" , 4 , ArgumentForCommandFunction);
        TaskManagement::ChangeDemandTime(ID , 10);
        while(1) {
            if(TaskManagement::GetTask(ID) == 0x00) {
                break;
            }
        }
    }
    for(i = 0; i < SlicedCommandCount; i++) {
        // MemoryManagement::Free(SlicedCommand[i]);
    }
    return;
}
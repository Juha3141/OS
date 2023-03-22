#include <TaskManagement.hpp>
#include <ResourceAccessManagement.hpp>

static Kernel::TaskManagement::SchedulingManager *TaskSchedulingManager;

static void SetTaskRegisters(struct Kernel::TaskManagement::Task *Task , unsigned long StartAddress);

void Kernel::TaskManagement::Initialize(void) {
    TaskSchedulingManager = (SchedulingManager *)Kernel::SystemStructure::Allocate(sizeof(SchedulingManager));
    TaskSchedulingManager->Initialize();
}

static void SetTaskRegisters(struct Kernel::TaskManagement::Task *Task , unsigned long StartAddress , unsigned long StackSize) {
    memset(&(Task->Registers) , 0 , sizeof(struct Kernel::TaskRegisters));
    switch(Task->Flags & 0x03) {
        case TASK_FLAGS_PRIVILAGE_KERNEL:
            Task->Registers.CS = KERNEL_CS;
            Task->Registers.DS = KERNEL_DS;
            Task->Registers.ES = KERNEL_DS;
            Task->Registers.FS = KERNEL_DS;
            Task->Registers.GS = KERNEL_DS;
            Task->Registers.SS = KERNEL_DS;
            break;
        case TASK_FLAGS_PRIVILAGE_USER_LEVEL:
            Task->Registers.CS = USER_CS;
            Task->Registers.DS = USER_DS;
            Task->Registers.ES = USER_DS;
            Task->Registers.FS = USER_DS;
            Task->Registers.GS = USER_DS;
            Task->Registers.SS = USER_DS;
            break;
    }
    Task->Registers.RSP = (unsigned long)Kernel::MemoryManagement::Allocate(StackSize)+StackSize-8;
    Task->Registers.RBP = Task->Registers.RSP;
    //*((unsigned long *)Task->Registers.RBP) = /*Return Address*/;
    Task->Registers.RFlags = 0x202;
    Task->Registers.RIP = StartAddress;
}

unsigned long Kernel::TaskManagement::CreateTask(unsigned long StartAddress , unsigned long Flags , unsigned long Status , unsigned long StackSize , const char *TaskName) {
    struct Task *Task;

    EnterCriticalSection();
    
    Task = (struct Task *)Kernel::MemoryManagement::Allocate(sizeof(struct Task));
    Task->ID = TaskSchedulingManager->CurrentMaxAllocatedID++;
    Task->Flags = Flags;
    Task->StackSize = StackSize;
    Task->DemandTime = TASK_DEFAULT_DEMAND_TIME;

    SetTaskRegisters(Task , StartAddress , StackSize);
    strcpy(Task->Name , TaskName);
    TaskSchedulingManager->Queues[(Status) & 0b11]->AddTask(Task);
    ExitCriticalSection();

    return Task->ID;
}

unsigned long Kernel::TaskManagement::TerminateTask(unsigned long TaskID) {
    int i;
    struct Task *Task = GetTask(TaskID);
    if(Task == 0x00) {
        return false;
    }
    TaskSchedulingManager->Queues[Task->Status]->RemoveTask(TaskID);
    return true;
}

bool Kernel::TaskManagement::ChangeTaskStatus(unsigned long TaskID , unsigned long NewStatus) {
    int i;
    struct Task *Task = GetTask(TaskID);
    if(Task == 0x00) {
        return false;
    }
    if(Task->Status == NewStatus) {
        return true;
    }
    TaskSchedulingManager->Queues[Task->Status]->RemoveTask(TaskID);
    TaskSchedulingManager->Queues[NewStatus]->AddTask(Task);
    return true;
}

void Kernel::TaskManagement::ChangeDemandTime(unsigned long TaskID , unsigned long DemandTime) {
    int i;
    struct Task *Task = GetTask(TaskID);
    if(Task == 0x00) {
        return;
    }
    Task->DemandTime = DemandTime;
}

void Kernel::TaskManagement::SchedulingManager::Initialize(void) {
    int i;
    struct Task *MainTask;
    TotalTaskCount = 0;
    CurrentMaxAllocatedID = 0x00;
    for(i = 0; i < 3; i++) {
        Queues[i] = (TaskManagement::TaskQueue *)Kernel::MemoryManagement::Allocate(sizeof(TaskQueue));
        Queues[i]->Initialize();
    }
    
    MainTask = (struct Task *)Kernel::MemoryManagement::Allocate(sizeof(struct Task));
    MainTask->ID = this->CurrentMaxAllocatedID++;
    MainTask->Flags = TASK_FLAGS_PRIVILAGE_KERNEL;

    SetTaskRegisters(MainTask , 0x00 , 8*1024*1024);
    strcpy(MainTask->Name , "MAINTASK");
    Queues[TASK_STATUS_RUNNING]->AddTask(MainTask);
    CurrentlyRunningTask = MainTask;
}

struct Kernel::TaskManagement::Task *Kernel::TaskManagement::SchedulingManager::SwitchTask(void) {
    struct Task *Task;
    if(Queues[TASK_STATUS_RUNNING]->TaskCount != 0) {
        Queues[TASK_STATUS_RUNNING]->SwitchToNextTask();
        Task = Queues[TASK_STATUS_RUNNING]->GetCurrentTask();
        // Kernel::printf("0x%X->" , CurrentlyRunningTask->ID);
        this->CurrentlyRunningTask = Task;
        // Kernel::printf("0x%X\n" , CurrentlyRunningTask->ID);
        TaskSchedulingManager->ExpirationDate = this->CurrentlyRunningTask->DemandTime;
    }
    else {
        return this->SwitchTask();
    }
    return Task;
}

void Kernel::TaskManagement::SwitchTask(void) {
    struct Kernel::TaskManagement::Task *PreviousTask;
    struct Kernel::TaskManagement::Task *CurrentTask;
    PreviousTask = TaskSchedulingManager->CurrentlyRunningTask;
    if(TaskSchedulingManager->ExpirationDate > 0) {
        TaskSchedulingManager->ExpirationDate -= 1;
        return;
    }
    CurrentTask = TaskSchedulingManager->SwitchTask();
    SwitchContext(&(PreviousTask->Registers) , &(CurrentTask->Registers));
}

void Kernel::TaskManagement::SwitchTaskInTimerInterrupt(void) {
    struct Kernel::TaskManagement::Task *PreviousTask;
    struct Kernel::TaskManagement::Task *CurrentTask;
    struct Kernel::TaskRegisters *IST = (struct Kernel::TaskRegisters *)(IST_STARTADDRESS+IST_SIZE-sizeof(struct Kernel::TaskRegisters));
    PreviousTask = TaskSchedulingManager->CurrentlyRunningTask;
    if(TaskSchedulingManager->ExpirationDate > 0) {
        TaskSchedulingManager->ExpirationDate -= 1;
        return;
    }
    CurrentTask = TaskSchedulingManager->SwitchTask();
    //memcpy(&(PreviousTask->Registers) , IST , sizeof(struct Kernel::TaskRegisters));
    PreviousTask->Registers.RAX = IST->RAX;
    PreviousTask->Registers.RBX = IST->RBX;
    PreviousTask->Registers.RCX = IST->RCX;
    PreviousTask->Registers.RDX = IST->RDX;

    PreviousTask->Registers.RDI = IST->RDI;
    PreviousTask->Registers.RSI = IST->RSI;

    PreviousTask->Registers.R8 = IST->R8;
    PreviousTask->Registers.R9 = IST->R9;
    PreviousTask->Registers.R10 = IST->R10;
    PreviousTask->Registers.R11 = IST->R11;
    PreviousTask->Registers.R12 = IST->R12;
    PreviousTask->Registers.R13 = IST->R13;
    PreviousTask->Registers.R14 = IST->R14;
    PreviousTask->Registers.R15 = IST->R15;

    PreviousTask->Registers.RIP = IST->RIP;
    PreviousTask->Registers.RBP = IST->RBP;
    PreviousTask->Registers.RSP = IST->RSP;

    PreviousTask->Registers.CS = IST->CS;
    PreviousTask->Registers.DS = IST->DS;
    PreviousTask->Registers.ES = IST->ES;
    PreviousTask->Registers.FS = IST->FS;
    PreviousTask->Registers.GS = IST->GS;
    PreviousTask->Registers.SS = IST->SS;

    PreviousTask->Registers.RFlags = IST->RFlags;
    
    if(PreviousTask->ID == CurrentTask->ID) {
        return;
    }

    IST->RAX = CurrentTask->Registers.RAX;
    IST->RBX = CurrentTask->Registers.RBX;
    IST->RCX = CurrentTask->Registers.RCX;
    IST->RDX = CurrentTask->Registers.RDX;

    IST->RDI = CurrentTask->Registers.RDI;
    IST->RSI = CurrentTask->Registers.RSI;

    IST->R8 = CurrentTask->Registers.R8;
    IST->R9 = CurrentTask->Registers.R9;
    IST->R10 = CurrentTask->Registers.R10;
    IST->R11 = CurrentTask->Registers.R11;
    IST->R12 = CurrentTask->Registers.R12;
    IST->R13 = CurrentTask->Registers.R13;
    IST->R14 = CurrentTask->Registers.R14;
    IST->R15 = CurrentTask->Registers.R15;

    IST->CS = CurrentTask->Registers.CS;
    IST->ES = CurrentTask->Registers.ES;
    IST->FS = CurrentTask->Registers.FS;
    IST->GS = CurrentTask->Registers.GS;
    IST->SS = CurrentTask->Registers.SS;

    IST->RFlags = CurrentTask->Registers.RFlags;

    IST->RIP = CurrentTask->Registers.RIP;
    IST->RBP = CurrentTask->Registers.RBP;
    IST->RSP = CurrentTask->Registers.RSP;
    //memcpy(IST , &(CurrentTask->Registers) , sizeof(struct Kernel::TaskRegisters));
    
}

struct Kernel::TaskManagement::Task *Kernel::TaskManagement::GetCurrentlyRunningTask(void) {
    return TaskSchedulingManager->CurrentlyRunningTask;
}

unsigned long Kernel::TaskManagement::GetCurrentlyRunningTaskID(void) {
    return TaskSchedulingManager->CurrentlyRunningTask->ID;
}

struct Kernel::TaskManagement::Task *Kernel::TaskManagement::GetTask(unsigned long ID) {
    int i;
    struct Task *Task;
        for(i = 0; i < 3; i++) {
        if((Task = TaskSchedulingManager->Queues[i]->GetTask(ID)) != 0) {
            return Task;
        }
    }
    return 0x00;
}

////////// PriorityQueue ////////////

void Kernel::TaskManagement::TaskQueue::Initialize(void) {
    TaskCount = 0;
}

void Kernel::TaskManagement::TaskQueue::AddTask(struct Task *Task) {
    if(TaskCount == 0) {
        StartTask = Task;
        StartTask->NextTask = StartTask;
        TaskCount++;
        CurrentTask = StartTask;
        NextTaskLinkerToAllocate = StartTask;
        return;
    }
    NextTaskLinkerToAllocate->NextTask = Task;
    NextTaskLinkerToAllocate->NextTask->NextTask = StartTask;
    NextTaskLinkerToAllocate = NextTaskLinkerToAllocate->NextTask;
    TaskCount++;
    return;
}

void Kernel::TaskManagement::TaskQueue::RemoveTask(unsigned long TaskID) {
    struct Task *TaskPointer = StartTask;
    struct Task *PreviousPointer = 0x00;
    if(TaskCount == 0) {
        return;
    }
    while(1) {
        if(TaskPointer->ID == TaskID) {
            break;
        }
        PreviousPointer = TaskPointer;
        TaskPointer = TaskPointer->NextTask;
    }
    if(PreviousPointer != 0x00) {
        PreviousPointer->NextTask = TaskPointer->NextTask;
    }
    TaskCount -= 1;
}

struct Kernel::TaskManagement::Task *Kernel::TaskManagement::TaskQueue::GetCurrentTask(void) {
    return CurrentTask;
}

void Kernel::TaskManagement::TaskQueue::SwitchToNextTask(void) {
    /*Kernel::printf("CurrentTask : 0x%X\n" , CurrentTask);
    Kernel::printf("CurrentTask->NextTask : 0x%X\n" , CurrentTask->NextTask);
    Kernel::printf("TaskCount : %d\n" , TaskCount);*/
    CurrentTask = CurrentTask->NextTask;
}

struct Kernel::TaskManagement::Task *Kernel::TaskManagement::TaskQueue::GetTask(unsigned long ID) {
    struct Task *TaskPointer = StartTask;
    if(TaskCount == 0) {
        return 0x00;
    }
    while(1) {
        if(TaskPointer->ID == ID) {
            return TaskPointer;
        }
        TaskPointer = TaskPointer->NextTask;
    }
    return 0x00;
}
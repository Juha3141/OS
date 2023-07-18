#include <TaskManagement.hpp>
#include <ResourceAccessManagement.hpp>
#include <MutualExclusion.hpp>

static TaskManagement::SchedulingManager **TaskSchedulingManager;
static MutualExclusion::SpinLock *CommonSpinLock;

static void SetTaskRegisters(struct TaskManagement::Task *Task , unsigned long StartAddress);

void TaskManagement::Initialize(void) {
    int i;
    class CoreSchedulingManager *CoreSchedulingManager = CoreSchedulingManager::GetInstance();
    CommonSpinLock = (MutualExclusion::SpinLock *)MemoryManagement::Allocate(sizeof(MutualExclusion::SpinLock));
    // CoreSchedulingManager *CoreManager = (class CoreSchedulingManager *)CoreSchedulingManager::GetInstance();
    TaskSchedulingManager = (SchedulingManager **)SystemStructure::Allocate(sizeof(SchedulingManager *)*CoreInformation::GetInstance()->CoreCount);
    for(int i = 0; i < CoreInformation::GetInstance()->CoreCount; i++) {
        TaskSchedulingManager[i] = (SchedulingManager *)SystemStructure::Allocate(sizeof(SchedulingManager));
        TaskSchedulingManager[i]->Initialize();
    }
    CommonSpinLock->Initialize();
    printf("CommonSpinLock : 0x%X\n" , CommonSpinLock);
}

static void SetTaskRegisters(struct TaskManagement::Task *Task , unsigned long StartAddress , unsigned long StackSize) {
    memset(&(Task->Registers) , 0 , sizeof(struct TaskRegisters));
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
    Task->StackAddress = (unsigned long)MemoryManagement::Allocate(StackSize);
    Task->Registers.RSP = Task->StackAddress+StackSize-8;
    Task->Registers.RBP = Task->Registers.RSP;
    *((unsigned long *)Task->Registers.RBP) = (unsigned long)TaskManagement::Exit;
    Task->Registers.RFlags = 0x202;
    Task->Registers.RIP = StartAddress;
}

unsigned long TaskManagement::CreateTask(unsigned long StartAddress , unsigned long Flags , unsigned long Status , unsigned long StackSize , const char *TaskName , const char *SubdirectoryLocation , int ArgumentCount , unsigned long *Arguments) {
    struct Task *Task;
    class CoreSchedulingManager *CoreSchedulingManager = CoreSchedulingManager::GetInstance();
    unsigned int AssignedCoreID = /*CoreSchedulingManager->AddTask();*/0; // Temporarily use BSP.. there's some weird error going on
    __asm__("cli");
    Task = (struct Task *)MemoryManagement::Allocate(sizeof(struct Task));
    Task->ID = ((TaskSchedulingManager[AssignedCoreID]->CurrentMaxAllocatedID++)|(0x100000000*AssignedCoreID));
    Task->Status = Status;
    Task->Flags = Flags;
    Task->StackSize = StackSize;
    Task->DemandTime = TASK_DEFAULT_DEMAND_TIME;
    Task->SubdirectoryLocation = (char *)MemoryManagement::Allocate(strlen(SubdirectoryLocation)+1);
    strcpy(Task->SubdirectoryLocation , SubdirectoryLocation);
    SetTaskRegisters(Task , StartAddress , StackSize);
    if((ArgumentCount != 0) && (Arguments != 0x00)) {
        AddArgumentToRegister(Task , ArgumentCount , Arguments);
    }
    strcpy(Task->Name , TaskName);
    TaskSchedulingManager[AssignedCoreID]->Queues[(Status) & 0b11]->AddTask(Task);
    __asm__ ("sti");
    return Task->ID;
}

void TaskManagement::AddArgumentToRegister(struct Task *Task , unsigned long ArgumentCount , unsigned long *Arguments) {
    int i;
    unsigned char *RSP = (unsigned char *)Task->Registers.RSP;
    unsigned char *RBP = (unsigned char *)Task->Registers.RBP;
    if(ArgumentCount >= 6) {
        RSP -= (ArgumentCount-5)*8;
    }
    if(ArgumentCount == 0) {
        return;
    }
    for(i = 0; i < ArgumentCount; i++) {
        switch(i) {
            case 0:
                Task->Registers.RDI = Arguments[i];
                break;
            case 1:
                Task->Registers.RSI = Arguments[i];
                break;
            case 2:
                Task->Registers.RDX = Arguments[i];
                break;
            case 3:
                Task->Registers.RCX = Arguments[i];
                break;
            case 4:
                Task->Registers.R8 = Arguments[i];
                break;
            case 5:
                Task->Registers.R9 = Arguments[i];
                break;
            default:
                *(RBP) = Arguments[i];
                RBP -= 8;
                RSP -= 8;
                break;
        }
    }
}

bool TaskManagement::TerminateTask(unsigned long TaskID) {
    int i;
    struct Task *Task = GetTask(TaskID);
    if(Task == 0x00) {
        return false;
    }
    if((TaskID >> 32) > CoreInformation::GetInstance()->CoreCount) {
        return false;
    }
    TaskSchedulingManager[(TaskID >> 32)]->Queues[Task->Status]->RemoveTask(TaskID);

    MemoryManagement::Free(Task->SubdirectoryLocation);
    MemoryManagement::Free((void *)Task->StackAddress);
    return true;
}

void TaskManagement::Exit(void) {
    if(TerminateTask(GetCurrentlyRunningTaskID()) == false) {
        printf("Failed terminating task\n");
        while(1) {
            ;
        }
    }
    while(1) {
        ;
    }
}

bool TaskManagement::ChangeTaskStatus(unsigned long TaskID , unsigned long NewStatus) {
    int i;
    struct Task *Task = GetTask(TaskID);
    if(Task == 0x00) {
        return false;
    }
    if(Task->Status == NewStatus) {
        return true;
    }
    TaskSchedulingManager[0]->Queues[Task->Status]->RemoveTask(TaskID);
    TaskSchedulingManager[0]->Queues[NewStatus]->AddTask(Task);
    Task->Status = NewStatus;
    return true;
}

void TaskManagement::ChangeDemandTime(unsigned long TaskID , unsigned long DemandTime) {
    int i;
    struct Task *Task = GetTask(TaskID);
    if(Task == 0x00) {
        return;
    }
    Task->DemandTime = DemandTime;
}

void TaskManagement::SchedulingManager::Initialize(void) {
    int i;
    struct Task *MainTask;
    TotalTaskCount = 0;
    CurrentMaxAllocatedID = 0x00;
    for(i = 0; i < 3; i++) {
        Queues[i] = (TaskManagement::TaskQueue *)MemoryManagement::Allocate(sizeof(TaskQueue));
        Queues[i]->Initialize();
    }
    
    MainTask = (struct Task *)MemoryManagement::Allocate(sizeof(struct Task));
    MainTask->ID = this->CurrentMaxAllocatedID++;
    MainTask->Flags = TASK_FLAGS_PRIVILAGE_KERNEL;
    MainTask->SubdirectoryLocation = (char *)MemoryManagement::Allocate(24);

    SetTaskRegisters(MainTask , 0x00 , 8*1024*1024);
    strcpy(MainTask->Name , "kernel");
    Queues[TASK_STATUS_RUNNING]->AddTask(MainTask);
    CurrentlyRunningTask = MainTask;
}

struct TaskManagement::Task *TaskManagement::SchedulingManager::SwitchTask(void) {
    struct Task *Task;
    if(Queues[TASK_STATUS_RUNNING]->TaskCount != 0) {
        Queues[TASK_STATUS_RUNNING]->SwitchToNextTask();
        Task = Queues[TASK_STATUS_RUNNING]->GetCurrentTask();
        this->CurrentlyRunningTask = Task;
        this->ExpirationDate = this->CurrentlyRunningTask->DemandTime;
    }
    else {
        return this->SwitchTask();
    }
    return Task;
}

void TaskManagement::SwitchTask(void) {
    struct TaskManagement::Task *PreviousTask;
    struct TaskManagement::Task *CurrentTask;
    CommonSpinLock->Lock();
    PreviousTask = TaskSchedulingManager[LocalAPIC::GetCurrentAPICID()]->CurrentlyRunningTask;
    if(TaskSchedulingManager[LocalAPIC::GetCurrentAPICID()]->ExpirationDate > 0) {
        TaskSchedulingManager[LocalAPIC::GetCurrentAPICID()]->ExpirationDate -= 1;
        CommonSpinLock->Unlock();
        return;
    }
    CurrentTask = TaskSchedulingManager[LocalAPIC::GetCurrentAPICID()]->SwitchTask();
    CommonSpinLock->Unlock();
    SwitchContext(&(PreviousTask->Registers) , &(CurrentTask->Registers));
}

void TaskManagement::SwitchTaskInTimerInterrupt(void) {
    struct TaskManagement::Task *PreviousTask;
    struct TaskManagement::Task *CurrentTask;
    struct TaskRegisters *IST;
    IST = (struct TaskRegisters *)(DescriptorTables::GetInterruptStackTable
          (LocalAPIC::GetCurrentAPICID())-sizeof(struct TaskRegisters));
    CommonSpinLock->Lock();
    if(LocalAPIC::GetCurrentAPICID() != 0) {
        return;
    }
    PreviousTask = TaskSchedulingManager[LocalAPIC::GetCurrentAPICID()]->CurrentlyRunningTask;
    if(TaskSchedulingManager[LocalAPIC::GetCurrentAPICID()]->ExpirationDate > 0) {
        TaskSchedulingManager[LocalAPIC::GetCurrentAPICID()]->ExpirationDate -= 1;
        CommonSpinLock->Unlock();
        return;
    }
    int GetTaskCount(void);
    CurrentTask = TaskSchedulingManager[LocalAPIC::GetCurrentAPICID()]->SwitchTask();
    CommonSpinLock->Unlock();
    if(PreviousTask->ID == CurrentTask->ID) {
        return;
    }
    memcpy(&(PreviousTask->Registers) , IST , sizeof(struct TaskRegisters));
    memcpy(IST , &(CurrentTask->Registers) , sizeof(struct TaskRegisters));
}

struct TaskManagement::Task *TaskManagement::GetCurrentlyRunningTask(void) {
    return TaskSchedulingManager[LocalAPIC::GetCurrentAPICID()]->CurrentlyRunningTask;
}

unsigned long TaskManagement::GetCurrentlyRunningTaskID(void) {
    return TaskSchedulingManager[LocalAPIC::GetCurrentAPICID()]->CurrentlyRunningTask->ID;
}

struct TaskManagement::Task *TaskManagement::GetTask(unsigned long ID) {
    int i;
    int j;
    struct Task *Task;
    for(j = 0; j < 3; j++) {
        if((Task = TaskSchedulingManager[(ID >> 32)]->Queues[j]->GetTask(ID)) != 0) { // error
            return Task;
        }
    }
    return 0x00;
}

struct TaskManagement::Task *TaskManagement::GetTask(const char *Name) {
    int i;
    int j;
    struct Task *Task;
    for(i = 0; i < CoreInformation::GetInstance()->CoreCount; i++) {
        for(j = 0; j < 3; j++) {
            if((Task = TaskSchedulingManager[i]->Queues[j]->GetTask(Name)) != 0) { // error
                return Task;
            }
        }
    }
    return 0x00;
}

struct TaskManagement::TaskQueue *TaskManagement::GetTaskQueue(int CoreID , int QueueID) {
    return TaskSchedulingManager[CoreID]->Queues[QueueID];
}

int TaskManagement::GetTaskCount(void) {
    int i;
    int j;
    int TaskCount = 0;
    for(i = 0; i < CoreInformation::GetInstance()->CoreCount; i++) {
        for(j = 0; j < TASK_QUEUE_COUNT; j++) {
            TaskCount += TaskSchedulingManager[i]->Queues[j]->TaskCount;
        }
    }
    return TaskCount;
}

char *TaskManagement::GetCurrentDirectoryLocation(void) {
    return GetCurrentlyRunningTask()->SubdirectoryLocation;
}

////////// PriorityQueue ////////////

void TaskManagement::TaskQueue::Initialize(void) {
    TaskCount = 0;
}

void TaskManagement::TaskQueue::AddTask(struct Task *Task) {
    struct Task *TaskPointer;
    if((TaskCount != 0) && (MaxTaskCount != 0) && (TaskCount >= MaxTaskCount)) {
        return;
    }
    if(TaskCount == 0) {
        StartTask = Task;
        StartTask->NextTask = StartTask;
        StartTask->PreviousTask = 0x00;
        TaskCount++;
        CurrentTask = StartTask;
        return;
    }
    TaskPointer = StartTask;
    while(TaskPointer->NextTask != StartTask) {
        TaskPointer = TaskPointer->NextTask;
    }
    TaskPointer->NextTask = Task;
    TaskPointer->NextTask->NextTask = StartTask;
    TaskPointer->NextTask->PreviousTask = TaskPointer;
    TaskCount++;
    return;
}

void TaskManagement::TaskQueue::RemoveTask(unsigned long TaskID) {
    struct Task *TaskPointer = StartTask;
    if(TaskCount == 0) {
        return;
    }
    while(1) {
        if(TaskPointer->ID == TaskID) {
            break;
        }
        TaskPointer = TaskPointer->NextTask;
        if(TaskPointer == StartTask) {
            return;
        }
    }
    if(TaskPointer->PreviousTask == 0) {
        TaskPointer->NextTask->PreviousTask = 0x00;
        StartTask = TaskPointer->NextTask;
    }
    else {
        TaskPointer->NextTask->PreviousTask = TaskPointer->PreviousTask;
        TaskPointer->PreviousTask->NextTask = TaskPointer->NextTask;
    }
    TaskCount -= 1;
    return;
}

struct TaskManagement::Task *TaskManagement::TaskQueue::GetCurrentTask(void) {
    return CurrentTask;
}

void TaskManagement::TaskQueue::SwitchToNextTask(void) {
    CurrentTask = CurrentTask->NextTask;
}

struct TaskManagement::Task *TaskManagement::TaskQueue::GetTask(unsigned long ID) {
    struct Task *TaskPointer = StartTask;
    if(TaskCount == 0) {
        return 0x00;
    }
    while(1) { // error
        if(TaskPointer->ID == ID) {
            return TaskPointer;
        }
        TaskPointer = TaskPointer->NextTask;
        if(TaskPointer == StartTask) {
            break;
        }
    }
    return 0x00;
}

struct TaskManagement::Task *TaskManagement::TaskQueue::GetTask(const char *Name) {
    struct Task *TaskPointer = StartTask;
    if(TaskCount == 0) {
        return 0x00;
    }
    while(1) { // error
        if(strcmp(TaskPointer->Name , Name) == 0) {
            return TaskPointer;
        }
        TaskPointer = TaskPointer->NextTask;
        if(TaskPointer == StartTask) {
            break;
        }
    }
    return 0x00;
}

////////// CoreSchedulingManager ////////////
// CoreSchedulingManager : Logically manages task within each cores.
// Only contains number of task that one core has, not physical task information.
// If one task is newly made, this manager decides what core they should use.

/// @brief Initializes core scheduling manager
/// @param None
void TaskManagement::CoreSchedulingManager::Initialize(void) {
    this->CoreCount = CoreInformation::GetInstance()->CoreCount;
    TaskCountPerCore = (unsigned int *)MemoryManagement::Allocate(this->CoreCount*sizeof(unsigned int));
    memset(TaskCountPerCore , 0 , this->CoreCount*sizeof(unsigned int));
    CurrentCoreOffset = 0;
}

/// @brief Add task to system, not physically, just increase number of task
///        When deciding what core should handle what task, this function just
///        uses RR scheduling algorithm
/// @param None
/// @return Core ID that's going to handle the task
unsigned int TaskManagement::CoreSchedulingManager::AddTask(void) {
    unsigned int CoreID = (CurrentCoreOffset++)%this->CoreCount; // do some RR scheduling
    TaskCountPerCore[CoreID]++;
    return CoreID;
}

/// @brief Add task to specific core, not by RR scheduling system
/// @param CoreID (APIC ID) of the core
/// @return Core ID that's going to handle the task(= CoreID)
unsigned int TaskManagement::CoreSchedulingManager::AddTask(unsigned int CoreID) {
    TaskCountPerCore[CoreID]++;
    return CoreID;
}

/// @brief Remove task from specific core(Logically)
/// @param CoreID 
/// @return CoreID
unsigned int TaskManagement::CoreSchedulingManager::RemoveTask(unsigned int CoreID) {
    if(TaskCountPerCore[CoreID] <= 0) {
        TaskCountPerCore[CoreID] = 0; // Adjust error (If below zero)
        return 0xFFFFFFFF;
    }
    TaskCountPerCore[CoreID]--; // Decrease task count
    return CoreID;
}
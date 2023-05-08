#include <TaskManagement.hpp>
#include <ResourceAccessManagement.hpp>
#include <MutualExclusion.hpp>

static Kernel::TaskManagement::SchedulingManager **TaskSchedulingManager;
static Kernel::MutualExclusion::SpinLock *CommonSpinLock;

static void SetTaskRegisters(struct Kernel::TaskManagement::Task *Task , unsigned long StartAddress);

void Kernel::TaskManagement::Initialize(void) {
    int i;
    class CoreSchedulingManager *CoreSchedulingManager = CoreSchedulingManager::GetInstance();
    CommonSpinLock = (Kernel::MutualExclusion::SpinLock *)Kernel::MemoryManagement::Allocate(sizeof(Kernel::MutualExclusion::SpinLock));
    // CoreSchedulingManager *CoreManager = (class CoreSchedulingManager *)CoreSchedulingManager::GetInstance();
    TaskSchedulingManager = (SchedulingManager **)Kernel::SystemStructure::Allocate(sizeof(SchedulingManager *)*CoreInformation::GetInstance()->CoreCount);
    for(int i = 0; i < CoreInformation::GetInstance()->CoreCount; i++) {
        TaskSchedulingManager[i] = (SchedulingManager *)Kernel::SystemStructure::Allocate(sizeof(SchedulingManager));
        TaskSchedulingManager[i]->Initialize();
    }
    CommonSpinLock->Initialize();
    Kernel::printf("CommonSpinLock : 0x%X\n" , CommonSpinLock);
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
    class CoreSchedulingManager *CoreSchedulingManager = CoreSchedulingManager::GetInstance();
    unsigned int AssignedCoreID = /*CoreSchedulingManager->AddTask();*/0; // Temporarily use BSP.. there's some weird error going on
    CommonSpinLock->Lock();
    Task = (struct Task *)Kernel::MemoryManagement::Allocate(sizeof(struct Task));
    Task->ID = ((TaskSchedulingManager[AssignedCoreID]->CurrentMaxAllocatedID++)|(0x100000000*AssignedCoreID));
    Task->Status = Status;
    Task->Flags = Flags;
    Task->StackSize = StackSize;
    Task->DemandTime = TASK_DEFAULT_DEMAND_TIME;
    SetTaskRegisters(Task , StartAddress , StackSize);
    strcpy(Task->Name , TaskName);
    TaskSchedulingManager[AssignedCoreID]->Queues[(Status) & 0b11]->AddTask(Task);
    Kernel::printf("Assigned Core ID : %d , TaskID : 0x%X\n" , AssignedCoreID , Task->ID);
    CommonSpinLock->Unlock();
    return Task->ID;
}

unsigned long Kernel::TaskManagement::TerminateTask(unsigned long TaskID) {
    int i;
    struct Task *Task = GetTask(TaskID);
    if(Task == 0x00) {
        return false;
    }
    TaskSchedulingManager[0]->Queues[Task->Status]->RemoveTask(TaskID);
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
    TaskSchedulingManager[0]->Queues[Task->Status]->RemoveTask(TaskID);
    TaskSchedulingManager[0]->Queues[NewStatus]->AddTask(Task);
    Task->Status = NewStatus;
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
        this->CurrentlyRunningTask = Task;
        this->ExpirationDate = this->CurrentlyRunningTask->DemandTime;
    }
    else {
        return this->SwitchTask();
    }
    return Task;
}

void Kernel::TaskManagement::SwitchTask(void) {
    struct Kernel::TaskManagement::Task *PreviousTask;
    struct Kernel::TaskManagement::Task *CurrentTask;
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

void Kernel::TaskManagement::SwitchTaskInTimerInterrupt(void) {
    struct Kernel::TaskManagement::Task *PreviousTask;
    struct Kernel::TaskManagement::Task *CurrentTask;
    struct Kernel::TaskRegisters *IST;
    IST = (struct Kernel::TaskRegisters *)(DescriptorTables::GetInterruptStackTable
          (LocalAPIC::GetCurrentAPICID())-sizeof(struct Kernel::TaskRegisters));
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
    CurrentTask = TaskSchedulingManager[LocalAPIC::GetCurrentAPICID()]->SwitchTask();
    CommonSpinLock->Unlock();
    if(PreviousTask->ID == CurrentTask->ID) {
        return;
    }
    memcpy(&(PreviousTask->Registers) , IST , sizeof(struct Kernel::TaskRegisters));
    memcpy(IST , &(CurrentTask->Registers) , sizeof(struct Kernel::TaskRegisters));
}

struct Kernel::TaskManagement::Task *Kernel::TaskManagement::GetCurrentlyRunningTask(void) {
    return TaskSchedulingManager[LocalAPIC::GetCurrentAPICID()]->CurrentlyRunningTask;
}

unsigned long Kernel::TaskManagement::GetCurrentlyRunningTaskID(void) {
    return TaskSchedulingManager[LocalAPIC::GetCurrentAPICID()]->CurrentlyRunningTask->ID;
}

struct Kernel::TaskManagement::Task *Kernel::TaskManagement::GetTask(unsigned long ID) {
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
    CurrentTask = CurrentTask->NextTask;
}

struct Kernel::TaskManagement::Task *Kernel::TaskManagement::TaskQueue::GetTask(unsigned long ID) {
    struct Task *TaskPointer = StartTask;
    if(TaskCount == 0) {
        return 0x00;
    }
    while(1) { // error
        if(TaskPointer->ID == ID) {
            return TaskPointer;
        }
        TaskPointer = TaskPointer->NextTask;
        if(TaskPointer->NextTask == StartTask) {
            return 0x00;
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
void Kernel::TaskManagement::CoreSchedulingManager::Initialize(void) {
    this->CoreCount = CoreInformation::GetInstance()->CoreCount;
    TaskCountPerCore = (unsigned int *)Kernel::MemoryManagement::Allocate(this->CoreCount*sizeof(unsigned int));
    memset(TaskCountPerCore , 0 , this->CoreCount*sizeof(unsigned int));
    CurrentCoreOffset = 0;
}

/// @brief Add task to system, not physically, just increase number of task
///        When deciding what core should handle what task, this function just
///        uses RR scheduling algorithm
/// @param None
/// @return Core ID that's going to handle the task
unsigned int Kernel::TaskManagement::CoreSchedulingManager::AddTask(void) {
    unsigned int CoreID = (CurrentCoreOffset++)%this->CoreCount; // do some RR scheduling
    TaskCountPerCore[CoreID]++;
    return CoreID;
}

/// @brief Add task to specific core, not by RR scheduling system
/// @param CoreID (APIC ID) of the core
/// @return Core ID that's going to handle the task(= CoreID)
unsigned int Kernel::TaskManagement::CoreSchedulingManager::AddTask(unsigned int CoreID) {
    TaskCountPerCore[CoreID]++;
    return CoreID;
}

/// @brief Remove task from specific core(Logically)
/// @param CoreID 
/// @return CoreID
unsigned int Kernel::TaskManagement::CoreSchedulingManager::RemoveTask(unsigned int CoreID) {
    if(TaskCountPerCore[CoreID] <= 0) {
        TaskCountPerCore[CoreID] = 0; // Adjust error (If below zero)
        return 0xFFFFFFFF;
    }
    TaskCountPerCore[CoreID]--; // Decrease task count
    return CoreID;
}
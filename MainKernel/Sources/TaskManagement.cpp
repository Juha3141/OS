#include <TaskManagement.hpp>
#include <ResourceAccessManagement.hpp>

static Kernel::TaskManagement::SchedulingManager *TaskSchedulingManager;

static void SetTaskRegisters(struct Kernel::TaskManagement::Task *Task , unsigned long StartAddress);

void Kernel::TaskManagement::Initialize(void) {
    TaskSchedulingManager = (SchedulingManager *)Kernel::SystemStructure::Allocate(sizeof(SchedulingManager));
    Kernel::printf("Address : 0x%X\n" , TaskSchedulingManager);
    TaskSchedulingManager->Initialize();
} 

static void SetTaskRegisters(struct Kernel::TaskManagement::Task *Task , unsigned long StartAddress , unsigned long StackSize) {
    memset(&(Task->Registers) , 0 , sizeof(struct Kernel::TaskRegisters));
    switch(Task->Flags & 0x02) {
        case TASK_FLAGS_PRIVILAGE_KERNEL:
            Task->Registers.CS = KERNEL_CS;
            Task->Registers.DS = KERNEL_DS;
            Task->Registers.ES = KERNEL_DS;
            Task->Registers.FS = KERNEL_DS;
            Task->Registers.GS = KERNEL_DS;
            Task->Registers.SS = KERNEL_DS;
            break;
        case TASK_FLAGS_PRIVILAGE_DEVICE_DRIVER1:
            Task->Registers.CS = RING1_CS;
            Task->Registers.DS = RING1_DS;
            Task->Registers.ES = RING1_DS;
            Task->Registers.FS = RING1_DS;
            Task->Registers.GS = RING1_DS;
            Task->Registers.SS = RING1_DS;
            break;
        case TASK_FLAGS_PRIVILAGE_DEVICE_DRIVER2:
            Task->Registers.CS = RING2_CS;
            Task->Registers.DS = RING2_DS;
            Task->Registers.ES = RING2_DS;
            Task->Registers.FS = RING2_DS;
            Task->Registers.GS = RING2_DS;
            Task->Registers.SS = RING2_DS;
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
    // Temprory
    __asm__ ("mov %0 , cr3":"=r"(Task->Registers.CR3));
}

unsigned long Kernel::TaskManagement::CreateTask(unsigned long StartAddress , unsigned long Flags , unsigned long Priority , unsigned long StackSize , const char *TaskName) {
    struct Task *Task;

    EnterCriticalSection();
    
    Task = (struct Task *)Kernel::MemoryManagement::Allocate(sizeof(struct Task));
    Task->ID = TaskSchedulingManager->CurrentMaxAllocatedID++;
    Task->Flags = Flags;
    Task->Priority = Priority;

    SetTaskRegisters(Task , StartAddress , StackSize);
    strcpy(Task->Name , TaskName);

    TaskSchedulingManager->AddTaskToPriorityQueue(Task);

    ExitCriticalSection();

    return Task->ID;
}

void Kernel::TaskManagement::SchedulingManager::Initialize(void) {
    int i;
    int DemandingTime;
    int RunningTimePerPriority = TASK_MAX_DEMANDED_TIME;
    struct Task *MainTask;
    TotalTaskCount = 0;
    PriorityQueues = (TaskManagement::PriorityQueue *)Kernel::MemoryManagement::Allocate(TASK_PRIORITY_COUNT*sizeof(PriorityQueue));
    for(i = 0; i < TASK_PRIORITY_COUNT-1; i++) {
        DemandingTime = 100/(TASK_PRIORITY_COUNT-i);
        PriorityQueues[i].Initialize(DemandingTime);
        Kernel::printf("Time Demanded to Priority %d : %d cycle\n" , i , DemandingTime);
    }
    MainTask = (struct Task *)Kernel::MemoryManagement::Allocate(sizeof(struct Task));
    MainTask->ID = this->CurrentMaxAllocatedID++;
    MainTask->Flags = TASK_FLAGS_PRIVILAGE_KERNEL|TASK_FLAGS_WORKING;
    MainTask->Priority = 0;

    SetTaskRegisters(MainTask , 0x00 , 8*1024*1024);
    strcpy(MainTask->Name , "MAINTASK");

    PriorityQueues[0].AddTask(MainTask);
    CurrentlyRunningTask = MainTask;
    Kernel::printf("Main Task ID : %d\n" , MainTask->ID);
}

void Kernel::TaskManagement::SchedulingManager::AddTaskToPriorityQueue(struct Task *Task) {
    PriorityQueues[Task->Priority].AddTask(Task);
}

struct Kernel::TaskManagement::Task *Kernel::TaskManagement::SchedulingManager::SwitchTask(void) {
    struct Task *Task;
    if(CurrentPriority >= TASK_PRIORITY_COUNT) {
        CurrentPriority = 0;
    }
    if(PriorityQueues[CurrentPriority].TaskCount != 0) {
        PriorityQueues[CurrentPriority].SwitchToNextTask();
        Task = PriorityQueues[CurrentPriority].GetCurrentTask();
        this->CurrentlyRunningTask = Task;

        TaskSchedulingManager->SetExpirationDate(PriorityQueues[CurrentPriority].RunningTime);
        CurrentPriority++;
    }
    else {
        CurrentPriority++;
        return this->SwitchTask();
    }
    return Task;
}

void Kernel::TaskManagement::SwitchTask(void) {
    struct Kernel::TaskManagement::Task *PreviousTask;
    struct Kernel::TaskManagement::Task *CurrentTask;
    PreviousTask = TaskSchedulingManager->CurrentlyRunningTask;
    if(TaskSchedulingManager->IsTaskDone() == false) {
        TaskSchedulingManager->SlowlyExpirate();
        return;
    }
    CurrentTask = TaskSchedulingManager->SwitchTask();
    SwitchContext(&(PreviousTask->Registers) , &(CurrentTask->Registers));
}

void Kernel::TaskManagement::SwitchTaskInTimerInterrupt(void) {
    struct Kernel::TaskManagement::Task *PreviousTask;
    struct Kernel::TaskManagement::Task *CurrentTask;
    struct Kernel::STACK_STRUCTURE *IST = (struct Kernel::STACK_STRUCTURE *)(IST_STARTADDRESS+IST_SIZE-sizeof(struct Kernel::STACK_STRUCTURE));
    PreviousTask = TaskSchedulingManager->CurrentlyRunningTask;
    if(TaskSchedulingManager->IsTaskDone() == false) {
        TaskSchedulingManager->SlowlyExpirate();
        return;
    }
    CurrentTask = TaskSchedulingManager->SwitchTask();
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
}

struct Kernel::TaskManagement::Task *Kernel::TaskManagement::GetCurrentlyRunningTask(void) {
    return TaskSchedulingManager->CurrentlyRunningTask;
}

unsigned long Kernel::TaskManagement::GetCurrentlyRunningTaskID(void) {
    return TaskSchedulingManager->CurrentlyRunningTask->ID;
}

////////// PriorityQueue ////////////

void Kernel::TaskManagement::PriorityQueue::Initialize(int Time) {
    RunningTime = Time;
}

void Kernel::TaskManagement::PriorityQueue::AddTask(struct Task *Task) {
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

void Kernel::TaskManagement::PriorityQueue::RemoveTask(unsigned long TaskID) {
    struct Task *TaskPointer = StartTask;
    if(TaskCount == 0) {
        return;
    }
    while(1) {
        if(TaskPointer->ID == TaskID) {
            break;
        }
        TaskPointer = TaskPointer->NextTask;
    }
    TaskCount -= 1;
}

struct Kernel::TaskManagement::Task *Kernel::TaskManagement::PriorityQueue::GetCurrentTask(void) {
    return CurrentTask;
}

void Kernel::TaskManagement::PriorityQueue::SwitchToNextTask(void) {
    CurrentTask = CurrentTask->NextTask;
}
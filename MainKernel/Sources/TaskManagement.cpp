#include <TaskManagement.hpp>

static Kernel::TaskManagement::SchedulingManager *TaskSchedulingManager;

static void SetTaskRegisters(struct Kernel::TaskManagement::Task *Task , unsigned long StartAddress);

void Kernel::TaskManagement::Initialize(void) {
    TaskSchedulingManager = (SchedulingManager *)Kernel::SystemStructure::Allocate(sizeof(SchedulingManager));
    Kernel::printf("Address : 0x%X\n" , TaskSchedulingManager);
    TaskSchedulingManager->Initialize();
} 

static void SetTaskRegisters(struct Kernel::TaskManagement::Task *Task , unsigned long StartAddress) {
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
    Task->Registers.RSP = (unsigned long)Kernel::MemoryManagement::Allocate(TASK_STACK_SIZE)+TASK_STACK_SIZE-8;
    Task->Registers.RBP = Task->Registers.RSP;
    Task->Registers.RFlags = 0x202;
    Task->Registers.RIP = StartAddress;
    // Temprory
    __asm__ ("mov %0 , cr3":"=r"(Task->Registers.CR3));
}

unsigned long Kernel::TaskManagement::CreateTask(unsigned long StartAddress , unsigned long Flags , unsigned long Priority , const char *TaskName) {
    struct Task *Task;
    __asm__ ("cli");
    Task = (struct Task *)Kernel::MemoryManagement::Allocate(sizeof(struct Task));
    Task->ID = TaskSchedulingManager->CurrentMaxAllocatedID++;
    Task->Flags = Flags;
    Task->Priority = Priority;

    SetTaskRegisters(Task , StartAddress);
    strcpy(Task->Name , TaskName);

    // Kernel::printf("Adding task to queue : %d\n" , Task->Priority);
    TaskSchedulingManager->AddTaskToPriorityQueue(Task);
    // Kernel::printf("Task added to the queue\n");
    __asm__ ("sti");
    return Task->ID;
}

void Kernel::TaskManagement::SchedulingManager::Initialize(void) {
    int i;
    struct Task *MainTask;
    TotalTaskCount = 0;
    PriorityQueueManagerArray = (TaskManagement::PriorityQueueManager *)Kernel::MemoryManagement::Allocate(TASK_PRIORITY_COUNT*sizeof(PriorityQueueManager));
    for(i = 0; i < TASK_PRIORITY_COUNT; i++) {
        PriorityQueueManagerArray[i].Initialize(255 , (TASK_PRIORITY_COUNT-i)*2);
        PriorityQueueManagerArray[i].DebugPriority = i;
        //Kernel::printf("Max task demanded to Priority %d : %d task(Q : %d)\n" , i , 255 , TASK_QUANTUMN);
    }
    MainTask = (struct Task *)Kernel::MemoryManagement::Allocate(sizeof(struct Task));
    MainTask->ID = this->CurrentMaxAllocatedID++;
    MainTask->Flags = TASK_FLAGS_PRIVILAGE_KERNEL|TASK_FLAGS_WORKING;
    MainTask->Priority = 0;

    SetTaskRegisters(MainTask , 0x00);
    strcpy(MainTask->Name , "MAINTASK");

    PriorityQueueManagerArray[0].AddTask(MainTask);
    CurrentlyRunningTask = MainTask;
    Kernel::printf("Main Task ID : %d\n" , MainTask->ID);
}

bool Kernel::TaskManagement::SchedulingManager::AddTaskToPriorityQueue(struct Task *Task) {
    return PriorityQueueManagerArray[Task->Priority].AddTask(Task);
}

struct Kernel::TaskManagement::Task *Kernel::TaskManagement::SchedulingManager::SwitchTask(void) {
    struct Task *Task;
    if(CurrentPriority >= TASK_PRIORITY_COUNT) {
        CurrentPriority = 0;
    }
    if(PriorityQueueManagerArray[CurrentPriority].TotalTaskCount != 0) { // Check if the queue is all empty
        // Kernel::printf("Switching to next task : %d Queue" , CurrentPriority);
        PriorityQueueManagerArray[CurrentPriority].SwitchToNextTask();
        Task = PriorityQueueManagerArray[CurrentPriority].GetCurrentTask();
        this->CurrentlyRunningTask = Task;
        
        TaskSchedulingManager->SetExpirationDate(PriorityQueueManagerArray[CurrentPriority].CommonDemandedTime);
        CurrentPriority++;
    }
    else {
        // Kernel::printf("No task in %d queue, skipping\n" , CurrentPriority);
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
    // Kernel::printf("Switching Task : \n");
    PreviousTask = TaskSchedulingManager->CurrentlyRunningTask;
    // Kernel::printf("PreviousTask   : 0x%X\n" , PreviousTask->ID);
    if(TaskSchedulingManager->IsTaskDone() == false) { // If the demanded time is not ended, expire.
        TaskSchedulingManager->SlowlyExpirate();
        // Kernel::printf("Yet.\n");
        return;
    }
    CurrentTask = TaskSchedulingManager->SwitchTask(); // Time expired
    // Kernel::printf("CurrentTask    : 0x%X\n" , CurrentTask->ID);

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

/////// PriorityQueueManager ////////

void Kernel::TaskManagement::PriorityQueueManager::Initialize(int MaxTaskCount , int Time) {
    PriorityQueueCount = 1;
    CommonMaxTaskCount = MaxTaskCount;
    CommonDemandedTime = Time;
    StartPriorityQueue = (PriorityQueue *)Kernel::MemoryManagement::Allocate(sizeof(PriorityQueue));
    StartPriorityQueue->Initialize(CommonMaxTaskCount , CommonDemandedTime);
    CurrentPriorityQueue = StartPriorityQueue;
    StartPriorityQueue->NextQueue = (unsigned long)StartPriorityQueue;

    TotalTaskCount = 0;
}

bool Kernel::TaskManagement::PriorityQueueManager::AddTask(struct Task *Task) {
    PriorityQueue *NewPriorityQueue;
    // Kernel::printf("Q%d - 0x%X TaskCount : %d\n" , DebugPriority , CurrentPriorityQueue , CurrentPriorityQueue->TaskCount);  
    if(CurrentPriorityQueue->AddTask(Task) == false) {
        // Kernel::printf("====== Creating New Queue(DebugPriority : %d) ======\n" , DebugPriority);
        NewPriorityQueue = (PriorityQueue *)Kernel::MemoryManagement::Allocate(sizeof(PriorityQueue));
        // Kernel::printf("Initializing : ");
        NewPriorityQueue->Initialize(CommonMaxTaskCount , CommonDemandedTime);
        NewPriorityQueue->AddTask(Task);
        // Kernel::printf("Done\n");
        // Kernel::printf("Linking      : \n");
        // Kernel::printf("$$$$$$$$$$$ Before $$$$$$$$$$$\n");
        // Kernel::printf("CurrentPriorityQueue->NextQueue : 0x%X\n" , CurrentPriorityQueue->NextQueue);
        // Kernel::printf("NewPriorityQueue                : 0x%X\n" , NewPriorityQueue);
        // Kernel::printf("StartPriorityQueue              : 0x%X\n" , StartPriorityQueue);
        // ERROR , why????
        CurrentPriorityQueue->NextQueue = (unsigned long)NewPriorityQueue;
        NewPriorityQueue->NextQueue = (unsigned long)StartPriorityQueue;
        
        CurrentPriorityQueue = NewPriorityQueue;
        // Kernel::printf("Adding %d to new queue\n" , Task->ID);
        // 
        // Kernel::printf("$$$$$$$$$$$ After  $$$$$$$$$$$\n");
        // Kernel::printf("CurrentPriorityQueue->NextQueue : 0x%X\n" , CurrentPriorityQueue->NextQueue);
        // Kernel::printf("NewPriorityQueue                : 0x%X\n" , NewPriorityQueue);
        // Kernel::printf("StartPriorityQueue              : 0x%X\n" , StartPriorityQueue);
        
    }
    TotalTaskCount++;
    return true;
}

bool Kernel::TaskManagement::PriorityQueueManager::RemoveTask(unsigned long ID) {
    PriorityQueue *QueuePointer;
    QueuePointer = StartPriorityQueue;
    while(1) {
        if(StartPriorityQueue->RemoveTask(ID) == true) {
            return true;
        }
        if(StartPriorityQueue->NextQueue == 0x00) {
            break;
        }
        StartPriorityQueue = (PriorityQueue *)StartPriorityQueue->NextQueue;
    }
    return false;
}

struct Kernel::TaskManagement::Task *Kernel::TaskManagement::PriorityQueueManager::GetCurrentTask(void) {
    return CurrentPriorityQueue->GetCurrentTask();
}

///////////////////////////////////////////
// THIS FUNCTION IS CAUSING THE ERROR!!! //
/////////////////////////////////////////// 
void Kernel::TaskManagement::PriorityQueueManager::SwitchToNextTask(void) {
    Task *PreviousTask;
    // Kernel::printf("Current Task ID : 0x%X\n" , CurrentPriorityQueue->CurrentTask->ID);
    // Kernel::printf("Q%d(CPQ : 0x%X) : Switching : %d --> " , DebugPriority , CurrentPriorityQueue , CurrentPriorityQueue->CurrentTask->ID);
    PreviousTask = CurrentPriorityQueue->CurrentTask;
    CurrentPriorityQueue->SwitchToNextTask(); // error, not returning to its original position
    // Kernel::printf("%d\n" , CurrentPriorityQueue->CurrentTask->ID);
    // Kernel::printf("New Task ID     : 0x%X\n" , CurrentPriorityQueue->CurrentTask->ID);
    if(CurrentPriorityQueue->CurrentTask == CurrentPriorityQueue->StartTask) {
        // Kernel::printf("We hit the end, heading to the start\n");
        // Kernel::printf("<DebugPriority : %d>\n" , DebugPriority);
        // Kernel::printf("CurrentPriorityQueue   : 0x%X\n" , CurrentPriorityQueue);
        // Another issue, the system doesn't change the queue
        CurrentPriorityQueue = (PriorityQueue*)CurrentPriorityQueue->NextQueue;
        // Kernel::printf("NextPriorityQueue      : 0x%X\n" , CurrentPriorityQueue);
        // Kernel::printf("Next NextPriorityQueue : 0x%X\n" , CurrentPriorityQueue->NextQueue);
    }
}

////////// PriorityQueue ////////////

void Kernel::TaskManagement::PriorityQueue::Initialize(int MaxTaskCount , int Time) {
    RunningTime = Time;
    this->TaskCount = 0;
    this->MaxTaskCount = MaxTaskCount;
}

bool Kernel::TaskManagement::PriorityQueue::AddTask(struct Task *Task) {
    struct Task *TaskPointer;
    if((TaskCount != 0) && (MaxTaskCount != 0) && (TaskCount >= MaxTaskCount)) {
        return false;
    }
    if(TaskCount == 0) {
        StartTask = Task;
        StartTask->NextTask = StartTask;
        StartTask->PreviousTask = 0x00;
        TaskCount++;
        CurrentTask = StartTask;
        return true;
    }
    TaskPointer = StartTask;
    while(TaskPointer->NextTask != StartTask) {
        TaskPointer = TaskPointer->NextTask;
    }
    TaskPointer->NextTask = Task;
    TaskPointer->NextTask->NextTask = StartTask;
    TaskPointer->NextTask->PreviousTask = TaskPointer;
    TaskCount++;
    return true;
}

bool Kernel::TaskManagement::PriorityQueue::RemoveTask(unsigned long TaskID) {
    struct Task *TaskPointer = StartTask;
    if(TaskCount == 0) {
        return false;
    }
    while(1) {
        if(TaskPointer->ID == TaskID) {
            break;
        }
        TaskPointer = TaskPointer->NextTask;
        if(TaskPointer == StartTask) {
            return false;
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
    return true;
}

struct Kernel::TaskManagement::Task *Kernel::TaskManagement::PriorityQueue::GetCurrentTask(void) {
    return CurrentTask;
}

void Kernel::TaskManagement::PriorityQueue::SwitchToNextTask(void) {
    CurrentTask = CurrentTask->NextTask;
}

bool Kernel::TaskManagement::PriorityQueue::IsPriorityQueueEmpty(void) {
    if(TaskCount == 0) {
        return true;
    }
    return false;
}

bool Kernel::TaskManagement::PriorityQueue::IsPriorityQueueFull(void) {
    if((IsPriorityQueueEmpty() == false) && (TaskCount >= MaxTaskCount)) {
        return true;
    }
    return false;
}
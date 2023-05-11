#ifndef _SCHEDULER_HPP_
#define _SCHEDULER_HPP_

#include <Kernel.hpp>

#define TASK_COUNT_PER_QUEUE          256

#define TASK_FLAGS_PRIVILAGE_KERNEL         0x00
#define TASK_FLAGS_PRIVILAGE_USER_LEVEL     0x03
#define TASK_FLAGS_SEPERATED_MEMORY_AREA    0x04

#define TASK_STATUS_RUNNING           0x00
#define TASK_STATUS_EVENT_WAITING     0x01
#define TASK_STATUS_IDLE              0x02

#define TASK_DEFAULT_DEMAND_TIME      2
#define TASK_PRIORITY_COUNT           10

struct TaskRegisters {
    unsigned long GS;
    unsigned long FS;
    unsigned long ES;
    unsigned long DS;

    unsigned long RAX;
    unsigned long RBX;
    unsigned long RCX;
    unsigned long RDX;

    unsigned long RDI;
    unsigned long RSI;

    unsigned long R8;
    unsigned long R9;
    unsigned long R10;
    unsigned long R11;
    unsigned long R12;
    unsigned long R13;
    unsigned long R14;
    unsigned long R15;
    // Now I get it! If interrupt occurrs, system puts those values to stack
    // and after interrupt, by iretq instruction, all pushed instructions are put back to 
    // Original registers! Now I get it!!
    unsigned long RBP;
    unsigned long RIP;
    unsigned long CS;
    unsigned long RFlags;
    unsigned long RSP;
    unsigned long SS;
};
namespace TaskManagement {
    struct Event {
        char *EventName;
        int NameLength;

        unsigned long Sender;
        unsigned long Receiver;

        unsigned long EventID;
        unsigned long EventResource;
    };
    struct Task {
        unsigned long ID;
        unsigned long Flags;
        unsigned long Status;
        unsigned long StackSize;
        struct TaskRegisters Registers;
        unsigned long CR3;

        char Name[32];
        struct Task *NextTask;

        StructureQueue<struct Event>EventQueue;

        int DemandTime;
    };

    class TaskQueue {
        friend struct SchedulingManager;
        public:
            void Initialize(void);
            void AddTask(struct Task *Task);
            void RemoveTask(unsigned long TaskID);

            struct Task *GetCurrentTask(void);
            void SwitchToNextTask(void);

            struct Task *GetTask(unsigned long ID);
        private:
            struct Task *StartTask;
            struct Task *CurrentTask;

            struct Task *NextTaskLinkerToAllocate;
            int TaskCount;
    };
    struct SchedulingManager {
        friend class TaskQueue;
        void Initialize(void);
        struct Task *SwitchTask(void);
        
        struct Task *CurrentlyRunningTask;
        int CurrentMaxAllocatedID;
        int ExpirationDate;

        TaskQueue *Queues[3];
        int TotalTaskCount;
    };
    // to-do : tomorrow or 7-8th period!!
    class CoreSchedulingManager {
        friend class SchedulingManager;
        public:
            void Initialize(void);
            unsigned int AddTask(void); // Returns Core Count
            unsigned int AddTask(unsigned int CoreID);
            unsigned int RemoveTask(unsigned int CoreID);

            static struct CoreSchedulingManager *GetInstance(void) {
                static class CoreSchedulingManager *Instance = 0x00;
                if(Instance == 0x00) {
                    Instance = (struct CoreSchedulingManager *)SystemStructure::Allocate(sizeof(struct CoreSchedulingManager));
                    Instance->Initialize();
                }
                return Instance;
            }
        private:
            // Array type : holds task count per core, index = core ID
            unsigned int *TaskCountPerCore;
            unsigned int CoreCount = 0;

            unsigned int CurrentCoreOffset;
    };

    void Initialize(void);
    unsigned long CreateTask(unsigned long StartAddress , unsigned long Flags , unsigned long Status , unsigned long StackSize , const char *TaskName);
    unsigned long TerminateTask(unsigned long TaskID);
    bool ChangeTaskStatus(unsigned long TaskID , unsigned long Status);
    void ChangeDemandTime(unsigned long TaskID , unsigned long DemandTime);
    void SwitchTask(void);
    void SwitchTaskInTimerInterrupt(void);
    struct Task *GetCurrentlyRunningTask(void);
    unsigned long GetCurrentlyRunningTaskID(void);
    struct Task *GetTask(unsigned long ID);
    
    // _ZN14TaskManagement13SwitchContextEP13TaskRegistersS1_
    void SwitchContext(struct TaskRegisters *LastContext , struct TaskRegisters *ContextToChange);
    // _ZN14TaskManagement13SwitchContextEP13TaskRegisters
    void SwitchContext(struct TaskRegisters *ContextToChange);
}

#endif
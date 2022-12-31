#ifndef _SCHEDULER_HPP_
#define _SCHEDULER_HPP_

#include <Kernel.hpp>

#define TASK_COUNT_PER_QUEUE          256

#define TASK_FLAGS_PRIVILAGE_KERNEL         0x00
#define TASK_FLAGS_PRIVILAGE_DEVICE_DRIVER1 0x01
#define TASK_FLAGS_PRIVILAGE_DEVICE_DRIVER2 0x02
#define TASK_FLAGS_PRIVILAGE_USER_LEVEL     0x03
#define TASK_FLAGS_WORKING                  0x04
#define TASK_FLAGS_PAUSED                   0x08
#define TASK_FLAGS_SEPERATED_MEMORY_AREA    0x10

#define TASK_STATUS_WORKING           0x01
#define TASK_STATUS_PAUSED            0x02

#define TASK_STACK_SIZE               8*1024*1024

#define TASK_QUANTUMN                 1     // One ms = Approx. 50ms
#define TASK_PRIORITY_COUNT           10

namespace Kernel {
    struct TaskRegisters {
        unsigned long RAX;
        unsigned long RBX;
        unsigned long RCX;
        unsigned long RDX;
        
        unsigned long RSI;
        unsigned long RDI;

        unsigned long R8;
        unsigned long R9;
        unsigned long R10;
        unsigned long R11;
        unsigned long R12;
        unsigned long R13;
        unsigned long R14;
        unsigned long R15;

        unsigned long RSP;
        unsigned long RBP;

        unsigned long RIP;

        unsigned long CS;
        unsigned long DS;
        unsigned long ES;
        unsigned long FS;
        unsigned long GS;
        unsigned long SS;

        unsigned long RFlags;
        unsigned long CR3;
    };
    namespace TaskManagement {
        struct Task {
            unsigned long ID;
            unsigned long Flags;
            unsigned long Priority;
            
            struct TaskRegisters Registers;
            unsigned long CR3;
            
            char Name[32];

            struct Task *NextTask;
            struct Task *PreviousTask;
        };
        class PriorityQueue {
            friend class SchedulingManager;
            friend class PriorityQueueManager;
            friend class PriorityQueue;
            public:
                void Initialize(int MaxTaskCount , int Time);
                bool AddTask(struct Task *Task);
                bool RemoveTask(unsigned long TaskID);

                struct Task *GetCurrentTask(void);
                void SwitchToNextTask(void);
                
                bool IsPriorityQueueEmpty(void);
                bool IsPriorityQueueFull(void);
                
                int RunningTime;

                unsigned long NextQueue;
            private:
                struct Task *StartTask;
                struct Task *CurrentTask;
                int TaskCount = 0;

                int MaxTaskCount = 0;
        };
        class PriorityQueueManager {
            friend class SchedulingManager;
            friend class PriorityQueue;
            friend class PriorityQueueManager;
            public:
                void Initialize(int MaxTaskCount , int Time);
                bool AddTask(struct Task *Task);
                bool RemoveTask(unsigned long ID);
                
                struct Task *GetCurrentTask(void);
                void SwitchToNextTask(void);
                
                PriorityQueue *StartPriorityQueue;
                PriorityQueue *CurrentPriorityQueue;
            private:
                int DebugPriority; // For debug
                
                int CommonMaxTaskCount;
                int CommonDemandedTime;
                int PriorityQueueCount;

                int TotalTaskCount = 0;
        };
        class SchedulingManager {
            friend class PriorityQueue;
            public:
                void Initialize(void);
                bool AddTaskToPriorityQueue(struct Task *Task);
                struct Task *SwitchTask(void);

                bool IsTaskDone(void) {
                    if(ExpirationDate <= 0) {
                        return true;
                    }
                    return false;
                }
                void SlowlyExpirate(void) {
                    ExpirationDate -= 1;
                }
                void SetExpirationDate(int Time) {
                    ExpirationDate = Time;
                }

                struct Task *CurrentlyRunningTask;
                int CurrentMaxAllocatedID = 0x00;
            private:
                //PriorityQueue *PriorityQueues; // Change to PriorityQueueManager
                PriorityQueueManager *PriorityQueueManagerArray;
                int TotalTaskCount;
                int CurrentPriority = 0;

                int ExpirationDate;
        };

        void Initialize(void);
        unsigned long CreateTask(unsigned long StartAddress , unsigned long Flags , unsigned long Priority , const char *TaskName);
        void SwitchTask(void);
        void SwitchTaskInTimerInterrupt(void);
        struct Task *GetCurrentlyRunningTask(void);
        unsigned long GetCurrentlyRunningTaskID(void);
        
        extern "C" void SwitchContext(struct Kernel::TaskRegisters *LastContext , struct Kernel::TaskRegisters *ContextToChange);
    }
}

#endif
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
#define TASK_STATUS_WAITING           0x02
#define TASK_STATUS_SLEEPING          0x04
#define TASK_STATUS_INVALID           0x08

#define TASK_MAX_DEMANDED_TIME        100
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
            unsigned long StackSize;

            struct TaskRegisters Registers;
            unsigned long CR3;

            char Name[32];
            struct Task *NextTask;
        };

        class PriorityQueue {
            friend class SchedulingManager;
            friend class PriorityQueue;
            public:
                void Initialize(int Time);
                void AddTask(struct Task *Task);
                void RemoveTask(unsigned long TaskID);

                struct Task *GetCurrentTask(void);
                void SwitchToNextTask(void);

                int RunningTime;
            private:
                struct Task *StartTask;
                struct Task *CurrentTask;

                struct Task *NextTaskLinkerToAllocate;
                int TaskCount = 0;
        };
        class SchedulingManager {
            friend class PriorityQueue;
            public:
                void Initialize(void);
                void AddTaskToPriorityQueue(struct Task *Task);
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
                PriorityQueue *PriorityQueues;
                int TotalTaskCount;
                int CurrentPriority = 0;

                int ExpirationDate;
        };

        void Initialize(void);
        unsigned long CreateTask(unsigned long StartAddress , unsigned long Flags , unsigned long Priority , unsigned long StackSize , const char *TaskName);
        void SwitchTask(void);
        void SwitchTaskInTimerInterrupt(void);
        struct Task *GetCurrentlyRunningTask(void);
        unsigned long GetCurrentlyRunningTaskID(void);
        // _ZN6Kernel14TaskManagement13SwitchContextEPNS_13TaskRegistersES2_
        void SwitchContext(struct Kernel::TaskRegisters *LastContext , struct Kernel::TaskRegisters *ContextToChange);
        // _ZN6Kernel14TaskManagement13SwitchContextEPNS_13TaskRegistersE
        void SwitchContext(struct Kernel::TaskRegisters *ContextToChange);
    }
}

#endif
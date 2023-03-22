#ifndef _MOUSE_HPP_
#define _MOUSE_HPP_

#include <Kernel.hpp>
#include <EssentialLibrary.hpp>
#include <Queue.hpp>
#include <MemoryManagement.hpp>

#define MOUSE_BUTTONLEFT    0x01
#define MOUSE_BUTTONRIGHT   0x02
#define MOUSE_BUTTONMIDDLE  0x04

namespace Kernel {
    namespace Mouse {
        struct MouseData {
            char RelativeX;
            char RelativeY;
            unsigned char ButtonData;
        };
        class DataManager {
            public:
                void Initialize(void) {
                    MouseDataQueue.Initialize(2048);
                }
                void ProcessMouseData(unsigned char Data);
                StructureQueue<struct MouseData>MouseDataQueue;
            private:
                int DataPhase = 0;
                struct MouseData TemporaryMouseData;
        };
        void Initialize(void);
        bool IsDataQueueEmpty(void);
        bool GetMouseDataQueue(struct MouseData *Data);
        void InterruptHandler(void);
        void MainInterruptHandler(void);
    }
}

#endif
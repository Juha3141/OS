#ifndef _MOUSE_HPP_
#define _MOUSE_HPP_

#include <Kernel.hpp>
#include <EssentialLibrary.hpp>
#include <Queue.hpp>

namespace Kernel {
    namespace Mouse {
        struct MouseData {
            long RelativeX;
            long RelativeY;
            unsigned char ButtonData;
        };
        class DataManager {
            public:
                void Initialize(void) {
                    MouseDataQueue.Initialize(2048);
                    Kernel::printf("0x%X\n" , &(DataPhase));
                }
                void ProcessMouseData(unsigned char Data);
                StructureQueue<struct MouseData>MouseDataQueue;
                
                struct MouseData TemproryMouseData;
                int DataPhase = 0;
        };
        void Initialize(void);
        int IsDataQueueEmpty(void);
        struct MouseData GetMouseDataQueue(void);
        void InterruptHandler(void);
        void MainInterruptHandler(void);
    };
};

#endif
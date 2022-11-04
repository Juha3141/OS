#ifndef _MOUSE_HPP_
#define _MOUSE_HPP_

#include <Kernel.hpp>
#include <EssentialLibrary.hpp>
#include <Queue.hpp>

namespace Kernel {
    namespace Mouse {
        class DataManager {
            public:
                void Initialize(void) {
                    DataQueue.Initialize(2048);
                }
            private:
                Queue<unsigned char>DataQueue;
        };
        void Initialize(void);
        void InterruptHandler(void);
        void MainInterruptHandler(void);
    };
};

#endif
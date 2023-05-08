#ifndef _MUTUAL_EXCLUSION_HPP_
#define _MUTUAL_EXCLUSION_HPP_

#include <Kernel.hpp>

namespace Kernel {
    namespace MutualExclusion {
        // What spinlock controller has to do
        // 1. Queue system(Job queue)
        // 2. Check if one's able to get resource
        extern "C" unsigned long _cmpxchg(volatile unsigned char *Current , unsigned char Compare , unsigned char New);
        class SpinLock {
            public:
                void Initialize(void);
                void Lock(void);
                void Unlock(void);
            private:
                volatile unsigned char Locked;
                volatile unsigned long LockedCoreID;
                volatile unsigned long LockedCount;
            };
    }   
}

#endif
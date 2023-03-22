#ifndef _MUTUAL_EXCLUSION_HPP_
#define _MUTUAL_EXCLUSION_HPP_

#include <Kernel.hpp>

namespace Kernel {
    namespace MutualExclusion {
        // What spinlock controller has to do
        // 1. Queue system(Job queue)
        // 2. Check if one's able to get resource
        unsigned int LockSet(unsigned long Pointer , unsigned int Compare , unsigned int Data);
        class SpinLock {
            public:
                void Initialize(void);
                void Lock(void);
                void Unlock(void);
            private:
                
        };
    }
}

#endif
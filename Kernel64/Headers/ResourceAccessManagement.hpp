#ifndef _RESOURCEACCESS_MANAGEMENT_H_
#define _RESOURCEACCESS_MANAGEMENT_H_

#include <EssentialLibrary.hpp>

void EnterCriticalSection(void);
void ExitCriticalSection(void);

namespace SpinLock {
    class Resource {
        public:
            void Initialize(void);
            void Lock(void);
            void Unlock(void);
        private:
            unsigned long Locked;
    };
}
namespace MutEx {
    class Resource {
        public:
            void Initialize(void);
            void Lock(void);
            void Unlock(void);
        private:
            int Locked;
    };
}

#endif
#include <MutualExclusion.hpp>

void Kernel::MutualExclusion::SpinLock::Initialize(void)  {
    Locked = 0;
    LockedCoreID = 0xFFFFFFFF;
    LockedCount = 0;
}

void Kernel::MutualExclusion::SpinLock::Lock(void) {
    if(!_cmpxchg(&(Locked) , 0 , 1)) {
        if(LockedCoreID == LocalAPIC::GetCurrentAPICID()) {
            // multiple lock
            LockedCount++;
            return;
        }
        while(!_cmpxchg(&(Locked) , 0 , 1)) {
            while(Locked == 1) {
                __asm__ __volatile__ ("pause":::"memory");
            }
        }
    }
    // First lock
    LockedCount = 1;
    LockedCoreID = LocalAPIC::GetCurrentAPICID();
}

void Kernel::MutualExclusion::SpinLock::Unlock(void)  {
    if((Locked == 0)||(LockedCoreID != LocalAPIC::GetCurrentAPICID())) {
        return;
    }
    if(LockedCount > 1) {
        LockedCount -= 1;
        return;
    }
    LockedCoreID = 0xFFFFFFFF;
    Locked = 0;
    LockedCount = 0;
}
#include <ResourceAccessManagement.hpp>

void EnterCriticalSection(void) {
    __asm__ ("cli");
}

void ExitCriticalSection(void) {
    __asm__ ("sti");
}

void MutEx::Resource::Initialize(void) {
    Locked = 0;
}

void MutEx::Resource::Lock(void) {
    if(Locked == 1) {
        return;
    }
    __asm__ ("cli");
    Locked = 1;
}

void MutEx::Resource::Unlock(void) {
    if(Locked == 0) {
        return;
    }
    Locked = 0;
    __asm__ ("sti");
}

void SpinLock::Resource::Initialize(void) {
    Locked = 0;
}
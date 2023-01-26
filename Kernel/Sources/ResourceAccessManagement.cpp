#include <ResourceAccessManagement.hpp>

void Kernel::EnterCriticalSection(void) {
    __asm__ ("cli");
}

void Kernel::ExitCriticalSection(void) {
    __asm__ ("sti");
}

void Kernel::MutEx::Resource::Initialize(void) {
    Locked = 0;
}

void Kernel::MutEx::Resource::Lock(void) {
    if(Locked == 1) {
        return;
    }
    __asm__ ("cli");
    Locked = 1;
}

void Kernel::MutEx::Resource::Unlock(void) {
    if(Locked == 0) {
        return;
    }
    Locked = 0;
    __asm__ ("sti");
}

void Kernel::SpinLock::Resource::Initialize(void) {
    Locked = 0;
}
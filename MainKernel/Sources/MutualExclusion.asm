[BITS 64]

SECTION .text

global _ZN6Kernel8SpinLock8Resource4LockEv      ; void Kernel::SpinLock::Lock(void)
global _ZN6Kernel8SpinLock8Resource6UnlockEv    ; void Kernel::SpinLock::Unlock(void)


_ZN6Kernel8SpinLock8Resource4LockEv:
    ; RDI : Locked flag
    mov rax , 1
    .L1:
        .L2:
            pause
            
            cmp qword[rdi] , 0
            jne .L2
        lock cmpxchg qword[rdi] , rax
        jne .L1
    
    ret

_ZN6Kernel8SpinLock8Resource6UnlockEv:
    mov qword[rdi] , 0
    ret
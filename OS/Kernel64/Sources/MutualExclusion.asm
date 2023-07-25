[BITS 64]

SECTION .text

global _cmpxchg

_cmpxchg:
    mov rax , rsi
    lock cmpxchg byte[rdi] , dl
    je .L1
    xor rax , rax
    ret

.L1:
    mov rax , 1
    ret
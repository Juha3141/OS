[BITS 64]

SECTION .text

global LongModeEntry
extern Main
extern APStartup

extern ActivatedCoreCount
extern KernelStackBase
extern KernelStackSize

LongModeEntry:
    mov ax , 0x10
    mov ds , ax
    mov es , ax
    mov fs , ax
    mov gs , ax
    mov ss , ax
    
    mov rbp , 0x4FFFF8
    mov rsp , 0x4FFFF8

    jmp .BSP ; skip for now
    
    cmp byte[0x8000+2] , 0x01
    je .BSP

    lock inc dword[ActivatedCoreCount]

    mov rcx , 27
    xor rax , rax
    xor rdx , rdx
    rdmsr

    xor rsi , rsi

    mov esi , eax
    and eax , 0b111111111111
    xor esi , eax
    shl rdx , 31
    or rsi , rdx

    xor rax , rax
    mov eax , dword[esi+0x20]
    shr eax , 24
    
    sub rax , 1 ; Exclude BSP
    imul eax , dword[KernelStackSize]

    xor rbp , rbp
    mov ebp , dword[KernelStackBase]
    sub rbp , rax
    mov rsp , rbp

    jmp .AP

.BSP:
    call Main

    jmp $

.AP:
    call APStartup

    jmp $
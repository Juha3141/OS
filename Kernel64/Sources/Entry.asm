[BITS 64]

SECTION .text

global LongModeEntry
extern Main
extern APStartup

extern ActivatedCoreCount
extern KernelStackBase
extern KernelStackSize
extern StackReady

LongModeEntry:
    mov ax , 0x10
    mov ds , ax
    mov es , ax
    mov fs , ax
    mov gs , ax
    mov ss , ax
    
    mov rbp , 0x4FFFF8
    mov rsp , 0x4FFFF8
    
    cmp byte[0x8000+2] , 0x01
    je .BSP

    lock inc dword[ActivatedCoreCount]

    .L1:
        mov eax , dword[StackReady]
        cmp eax , 0x01
        jnz .L1

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
    mov al , 0x11
    out 0x20 , al
    mov al , 0x20
    out 0x21 , al
    mov al , 0x04
    out 0x21 , al
    mov al , 0x01
    out 0x21 , al

    mov al , 0x11
    out 0xA0 , al
    mov al , 0x28
    out 0xA1 , al
    mov al , 0x02
    out 0xA1 , al
    mov al , 0x01
    out 0xA1 , al

    mov al , 0xFF
    out 0x21 , al
    out 0xA1 , al

    call Main

    jmp $

.AP:
    call APStartup

    jmp $
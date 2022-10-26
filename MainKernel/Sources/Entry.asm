[BITS 64]

global LongModeEntry
extern Main
extern APStartup

LongModeEntry:
    mov ax , 0x10
    mov ds , ax
    mov es , ax
    mov fs , ax
    mov gs , ax
    mov ss , ax
    
    mov rbp , 0x31FFF8
    mov rsp , 0x31FFF8
    
    cmp byte[0x8000+2] , 0x01
    je .BSP

    lock add word[0x8000+2+1] , 1

    mov rax , 0x00
    mov ax , word[0x8000+2+1]

    imul rax , 128*1024
    sub rbp , rax
    sub rsp , rax

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
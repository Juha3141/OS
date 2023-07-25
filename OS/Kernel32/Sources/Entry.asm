[BITS 32]

SECTION .text

global PreMain
extern Main

PreMain:
    mov ax , 0x10
    mov ds , ax
    mov es , ax
    mov fs , ax
    mov gs , ax
    mov ss , ax

    mov esp , 0xDFF8
    mov ebp , 0xDFF8

    call Main

    jmp $
[BITS 32]

SECTION .text

; Multiboot Header
	align 0x04 
	MagicNumber: dd 0x1BADB002
	Flags: dd 0x00
	Checksum: dd -(0x1BADB002+0x00)

global Entry
extern Main

Entry:
    cli

    mov ebp , KernelStack
    mov esp , KernelStack
    push ebx ; Multiboot information pointer
    call Main
    
    jmp $

SECTION .bss

    resb 4096
KernelStack:
[BITS 64]

SECTION .text

global GetUsableMemory

GetUsableMemory:
    push rbp
    mov rbp , rsp

    mov rax , 0x00

    .L1:
        mov r8 , qword[rdi]
        mov r9 , qword[rdi+8]
        mov r10 , qword[rdi+16]
        
        add rdi , 24
        
        cmp r10 , 0x01
        je .L2
        
        cmp r10 , 0x00
        je .L3

        jmp .L1
    
    .L2:
        cmp r8 , rdi
        jb .L1

        add rax , r9
        jmp .L1
    
    .L3:
        leave
        ret

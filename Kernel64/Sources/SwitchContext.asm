[BITS 64]

SECTION .text

global _ZN14TaskManagement13SwitchContextEP13TaskRegistersS1_ ; void SwitchContext(struct TaskRegisters *LastContext , struct TaskRegisters *ContetToChange);
global _ZN14TaskManagement13SwitchContextEP13TaskRegisters    ; void SwitchContext(struct TaskRegisters *ContextToChange);

_ZN14TaskManagement13SwitchContextEP13TaskRegistersS1_:
    ; RDI : CurrentRegisters
    ; RSI : NextRegisters

    ;Saving the context
    mov qword[rdi] , rax
    mov qword[rdi+8] , rbx
    mov qword[rdi+16] , rcx
    mov qword[rdi+24] , rdx
    
    mov qword[rdi+32] , rsi
    mov qword[rdi+40] , rdi
    
    mov qword[rdi+48] , r8
    mov qword[rdi+56] , r9
    mov qword[rdi+64] , r10
    mov qword[rdi+72] , r11
    mov qword[rdi+80] , r12
    mov qword[rdi+88] , r13
    mov qword[rdi+96] , r14
    mov qword[rdi+104] , r15

    mov qword[rdi+112] , rsp
    mov qword[rdi+120] , rbp
    add qword[rdi+112] , 8

    mov rax , qword[rsp]
    mov qword[rdi+128] , rax

    xor rax , rax
    mov ax , cs
    mov qword[rdi+136] , rax
    mov ax , ds
    mov qword[rdi+144] , rax
    mov ax , es
    mov qword[rdi+152] , rax
    mov ax , fs
    mov qword[rdi+160] , rax
    mov ax , gs
    mov qword[rdi+168] , rax
    mov ax , ss
    mov qword[rdi+176] , rax

    pushfq
    pop rax
    mov qword[rdi+184] , rax    ; RFlags

    mov rax , cr3
    mov qword[rdi+192] , rax    ; CR3

    ; Loading the context

    mov rbx , qword[rsi+8]
    mov rcx , qword[rsi+16]
    mov rdx , qword[rsi+24]
    
    mov r8 , qword[rsi+48]
    mov r9 , qword[rsi+56]
    mov r10 , qword[rsi+64]
    mov r11 , qword[rsi+72]
    mov r12 , qword[rsi+80]
    mov r13 , qword[rsi+88]
    mov r14 , qword[rsi+96]
    mov r15 , qword[rsi+104]
    
    mov rdi , qword[rsi+40]

    mov rax , qword[rsi+144]
    mov ds , rax
    mov rax , qword[rsi+152]
    mov es , rax
    mov rax , qword[rsi+160]
    mov fs , rax
    mov rax , qword[rsi+168]
    mov gs , rax

    mov rax , qword[rsi+192]
    mov cr3 , rax

    push qword[rsi+176]
    push qword[rsi+112]
    push qword[rsi+184]
    push qword[rsi+136]
    push qword[rsi+128]
    
    mov rbp , qword[rsi+120]
    mov rsi , qword[rsi+32]
    mov rax , qword[rsi]

    iretq

_ZN14TaskManagement13SwitchContextEP13TaskRegisters:
    mov rbx , qword[rdi+8]
    mov rcx , qword[rdi+16]
    mov rdx , qword[rdi+24]
    
    mov r8 , qword[rdi+48]
    mov r9 , qword[rdi+56]
    mov r10 , qword[rdi+64]
    mov r11 , qword[rdi+72]
    mov r12 , qword[rdi+80]
    mov r13 , qword[rdi+88]
    mov r14 , qword[rdi+96]
    mov r15 , qword[rdi+104]
    
    mov rdi , qword[rdi+40]

;    mov rax , qword[rdi+144]
;    mov ds , rax
;    mov rax , qword[rdi+152]
;    mov es , rax
;    mov rax , qword[rdi+160]
;    mov fs , rax
;    mov rax , qword[rdi+168]
;    mov gs , rax

;    mov rax , qword[rdi+192]
;    mov cr3 , rax

    push qword[rdi+176]
    push qword[rdi+112]
    push qword[rdi+184]
    push qword[rdi+136]
    push qword[rdi+128]
    
    mov rbp , qword[rdi+120]
    mov rdi , qword[rdi+32]
    mov rax , qword[rdi]

    jmp $

    iretq
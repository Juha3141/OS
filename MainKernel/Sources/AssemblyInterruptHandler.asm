;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; File "AssemblyInterruptHandler.asm"              ;;
;; Written by : Juha Cho                            ;;
;; Started Date : 2022.10.27                        ;;
;; Description : Contains a IRQ handling functions. ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[BITS 64]

SECTION .text

; Interrupt handler : 
global _ZN6Kernel3PIT16InterruptHandlerEv           ; Kernel::PIT::InterruptHandler
global _ZN6Kernel8Keyboard16InterruptHandlerEv      ; Kernel::Keyboard::InterruptHandler 

; Functions being called by Interrupt handler(the actual handlers) : 
extern _ZN6Kernel3PIT20MainInterruptHandlerEv       ; Kernel::PIT::MainInterruptHandler
extern _ZN6Kernel8Keyboard20MainInterruptHandlerEv  ; Kernel::Keyboard::MainInterruptHandler

%macro SAVE_REGISTERS_TO_STACK 0    ; Save the registers to stack
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro LOAD_REGISTERS_FROM_STACK 0  ; Load the registers from stack
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

; Interrupt handler for IRQ #0 (Timer Interrupt)
_ZN6Kernel3PIT16InterruptHandlerEv:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    call _ZN6Kernel3PIT20MainInterruptHandlerEv             ; call the main function

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

; Interrupt handler for IRQ #1 (Keyboard Interrupt)
_ZN6Kernel8Keyboard16InterruptHandlerEv:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    call _ZN6Kernel8Keyboard20MainInterruptHandlerEv        ; call the main function
    
    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt
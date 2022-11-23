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
global _ZN6Kernel5Mouse16InterruptHandlerEv         ; Kernel::Mouse::InterruptHandler

global DividedByZero
global Debug
global NonMaskableInterrupt
global Breakpoint
global Overflow
global BoundRangeExceeded
global InvalidOpcode
global DeviceNotAvailable
global DoubleFault
global CorprocessorSegmentOverrun
global InvalidTSS
global SegmentNotPresent
global StackSegmentFault
global GeneralProtectionFault
global PageFault
global Reserved15
global x87FloatPointException
global AlignmentCheck
global MachineCheck
global SIMDFloatingPointException
global VirtualizationException
global ControlProtectionException
global Reserved22
global Reserved23
global Reserved24
global Reserved25
global Reserved26
global Reserved27
global HypervisorInjectionException
global VMMCommunicationException
global SecurityException

; Functions being called by Interrupt handler(the actual handlers) : 
extern _ZN6Kernel3PIT20MainInterruptHandlerEv       ; Kernel::PIT::MainInterruptHandler
extern _ZN6Kernel8Keyboard20MainInterruptHandlerEv  ; Kernel::Keyboard::MainInterruptHandler
extern _ZN6Kernel5Mouse20MainInterruptHandlerEv     ; Kernel::Mouse::MainInterruptHandler

extern _ZN6Kernel10Exceptions17ProcessExceptionsEim

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

DividedByZero:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 0
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Debug:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 1
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

NonMaskableInterrupt:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 2
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Breakpoint:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 3
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Overflow:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 4
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt
    
BoundRangeExceeded:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 5
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

InvalidOpcode:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 6
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

DeviceNotAvailable:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 7
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

DoubleFault:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 8
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

CorprocessorSegmentOverrun:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 9
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

InvalidTSS:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 10
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

SegmentNotPresent:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 11
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

StackSegmentFault:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 12
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

GeneralProtectionFault:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 13
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

PageFault:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 14
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Reserved15:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 15
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

x87FloatPointException:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 16
    mov rsi ,  0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

AlignmentCheck:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi ,  17
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

MachineCheck:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 18
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

SIMDFloatingPointException:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 19
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

VirtualizationException:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 20
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

ControlProtectionException:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 21
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Reserved22:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 22
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Reserved23:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 23
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Reserved24:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 24
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Reserved25:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 25
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Reserved26:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 26
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Reserved27:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 27
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

HypervisorInjectionException:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 28
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

VMMCommunicationException:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 29
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

SecurityException:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 30
    mov rsi , 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

; Interrupt handler for IRQ #0 (Timer Interrupt)
_ZN6Kernel3PIT16InterruptHandlerEv:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    call _ZN6Kernel3PIT20MainInterruptHandlerEv         ; call the main function

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

; Interrupt handler for IRQ #1 (Keyboard Interrupt)
_ZN6Kernel8Keyboard16InterruptHandlerEv:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    call _ZN6Kernel8Keyboard20MainInterruptHandlerEv    ; call the main function
    
    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

_ZN6Kernel5Mouse16InterruptHandlerEv:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    call _ZN6Kernel5Mouse20MainInterruptHandlerEv      ; call the main function
    
    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt
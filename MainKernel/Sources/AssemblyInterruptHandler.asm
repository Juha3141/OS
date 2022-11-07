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

    push 0
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Debug:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 1
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

NonMaskableInterrupt:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 2
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Breakpoint:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 3
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Overflow:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 4
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt
    
BoundRangeExceeded:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 5
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

InvalidOpcode:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 6
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

DeviceNotAvailable:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 7
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

DoubleFault:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 8
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

CorprocessorSegmentOverrun:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 9
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

InvalidTSS:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 10
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

SegmentNotPresent:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 11
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

StackSegmentFault:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 12
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

GeneralProtectionFault:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 13
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

PageFault:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 14
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Reserved15:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 15
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

x87FloatPointException:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 16
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

AlignmentCheck:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 17
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

MachineCheck:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 18
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

SIMDFloatingPointException:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 19
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

VirtualizationException:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 20
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

ControlProtectionException:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 21
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Reserved22:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 22
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Reserved23:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 23
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Reserved24:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 24
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Reserved25:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 25
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Reserved26:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 26
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

Reserved27:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 27
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

HypervisorInjectionException:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 28
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

VMMCommunicationException:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 29
    push 0
    call _ZN6Kernel10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    
    pop rbp                     ; Load stack base from the stack
    iretq                       ; Return from the interrupt

SecurityException:
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    push 30
    push 0
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
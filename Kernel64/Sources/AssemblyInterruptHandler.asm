;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; File "AssemblyInterruptHandler.asm"              ;;
;; Written by : Juha Cho                            ;;
;; Started Date : 2022.10.27                        ;;
;; Description : Contains a IRQ handling functions. ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[BITS 64]

SECTION .text

; Interrupt handler : 
global _ZN3PIT16InterruptHandlerEv                   ; PIT::InterruptHandler
global _ZN8Keyboard16InterruptHandlerEv              ; Keyboard::InterruptHandler 
global _ZN5Mouse16InterruptHandlerEv                 ; Mouse::InterruptHandler
global _ZN9LocalAPIC5Timer16InterruptHandlerEv       ; APIC::Timer::InterruptHandler
global _ZN3IDE22InterruptHandler_IRQ14Ev             ; IDE::InterruptHandler_IRQ14
global _ZN3IDE22InterruptHandler_IRQ15Ev             ; IDE::InterruptHandler_IRQ15

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
extern _ZN3PIT20MainInterruptHandlerEv               ; PIT::MainInterruptHandler
extern _ZN8Keyboard20MainInterruptHandlerEv          ; Keyboard::MainInterruptHandler
extern _ZN5Mouse20MainInterruptHandlerEv             ; Mouse::MainInterruptHandler
extern _ZN9LocalAPIC5Timer20MainInterruptHandlerEv   ; APIC::Timer::MainInterruptHandler
extern _ZN3IDE20MainInterruptHandlerEb               ; IDE::MainInterruptHandler

extern _ZN9LocalAPIC7SendEOIEv                       ; LocalAPIC::SendEOI

extern _ZN10Exceptions17ProcessExceptionsEim

%macro SAVE_REGISTERS_TO_STACK 0    ; Save the registers to stack, exactly IST
    push rbp                    ; Save stack base to the stack
    mov rbp , rsp               ; Set the stack base to current location of stack

    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8

    push rsi
    push rdi

    push rdx
    push rcx
    push rbx
    push rax
    
    mov ax , ds
    push rax
    mov ax , es
    push rax
    push fs
    push gs

    mov ax , 0x10
    mov ds , ax
    mov es , ax
    mov fs , ax
    mov gs , ax
%endmacro

%macro LOAD_REGISTERS_FROM_STACK 0  ; Load the registers from stack
    pop gs
    pop fs
    pop rax      ; es
    mov es , ax
    pop rax      ; ds
    mov ds , ax
    
    pop rax
    pop rbx
    pop rcx
    pop rdx
    
    pop rdi
    pop rsi

    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
    pop rbp                     ; Load stack base from the stack
%endmacro

DividedByZero:
    SAVE_REGISTERS_TO_STACK     ; Save the registers

    mov rdi , 0
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

Debug:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 1
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

NonMaskableInterrupt:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 2
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

Breakpoint:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 3
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

Overflow:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 4
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt
    
BoundRangeExceeded:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 5
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

InvalidOpcode:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 6
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

DeviceNotAvailable:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 7
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

DoubleFault:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 8
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

CorprocessorSegmentOverrun:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 9
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

InvalidTSS:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 10
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

SegmentNotPresent:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 11
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

StackSegmentFault:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 12
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

GeneralProtectionFault:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 13
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

PageFault:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 14
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

Reserved15:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 15
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

x87FloatPointException:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 16
    mov rsi ,  0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

AlignmentCheck:
    SAVE_REGISTERS_TO_STACK

    mov rdi ,  17
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

MachineCheck:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 18
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

SIMDFloatingPointException:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 19
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

VirtualizationException:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 20
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

ControlProtectionException:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 21
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

Reserved22:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 22
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

Reserved23:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 23
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

Reserved24:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 24
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

Reserved25:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 25
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

Reserved26:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 26
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

Reserved27:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 27
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

HypervisorInjectionException:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 28
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

VMMCommunicationException:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 29
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

SecurityException:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 30
    mov rsi , 0
    call _ZN10Exceptions17ProcessExceptionsEim

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

; Interrupt handler for IRQ #0 (Timer Interrupt)
_ZN3PIT16InterruptHandlerEv:
    SAVE_REGISTERS_TO_STACK

    call _ZN3PIT20MainInterruptHandlerEv         ; call the main function

    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

; Interrupt handler for IRQ #1 (Keyboard Interrupt)
_ZN8Keyboard16InterruptHandlerEv:
    SAVE_REGISTERS_TO_STACK

    call _ZN8Keyboard20MainInterruptHandlerEv    ; call the main function
    
    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

_ZN5Mouse16InterruptHandlerEv:
    SAVE_REGISTERS_TO_STACK

    call _ZN5Mouse20MainInterruptHandlerEv      ; call the main function
    
    LOAD_REGISTERS_FROM_STACK   ; Load the registers
    iretq                       ; Return from the interrupt

_ZN9LocalAPIC5Timer16InterruptHandlerEv:
    SAVE_REGISTERS_TO_STACK
    
    ; Get Current APIC ID
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
    
    ; rax = APIC ID

    mov rcx , rax
    imul rax , 2

    mov rsi , 0xB8000+(77*2)
    add rsi , rax
    lock add byte[rsi] , 1
    lock add byte[rsi+1] , 1

    test rax , rax
    jnz .DONE

    call _ZN9LocalAPIC5Timer20MainInterruptHandlerEv
.DONE:
    LOAD_REGISTERS_FROM_STACK
    iretq

_ZN3IDE22InterruptHandler_IRQ14Ev:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 1
    call _ZN3IDE20MainInterruptHandlerEb

    LOAD_REGISTERS_FROM_STACK
    iretq

_ZN3IDE22InterruptHandler_IRQ15Ev:
    SAVE_REGISTERS_TO_STACK

    mov rdi , 0
    call _ZN3IDE20MainInterruptHandlerEb

    LOAD_REGISTERS_FROM_STACK
    iretq
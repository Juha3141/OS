[BITS 32]

global DoBIOSInterrupt
global JumpToKernel64

SECTION .text

JumpToKernel64:
    cli 
    xor edx , edx
                            ; Switch to Long Mode
    mov eax , cr4           ; 1. Set PAE(Physical Address Extension) bit to 1 (5th bit)
                            ; If you set both PAE bit and PS bit, page size is set to 2MB.
    or eax , 0b1000100000   ; Also set OSFXSR to 1
    mov cr4 , eax
    
    mov eax , 0x10000       ; CR3 register needs to contain the location of page entry
    mov cr3 , eax           ; 2. Set the location of the PML4 Entry (0x10000, check 140th line in current code)
    
    mov ecx , 0xC0000080
    rdmsr                   ; Access 0xC0000080 of MSR(Model Specific Register)

    or eax , 0b100000001    ; 3. Set LME(Long Mode Enable) bit and SCE(System Call Enable) bit (8th bit , 0th bit respectively)
                            ; - which both exist in MSR register
                            ; (System Call will be featured in this Operating System,
                            ;  that's why I'm currently setting SCE bit to 1)
    wrmsr
    
    mov eax , cr0
    or eax , 0x80000000     ; Set PG(Paging Enable) bit to 1
    mov cr0 , eax
    lgdt [LongModeGDTR]     ; Load long mode GDT so that we can use Data and Code segments without an issue
    
    jmp 0x08:0x100000       ; Finally jump to long mode kernel (Main Kernel)

ProtectModeCR0: dd 0x00
ProtectModeESP: dd 0x00
ProtectModeEBP: dd 0x00
ReturnAddress: dd 0x00

Successed: db 0x00

ArgumentAX: dw 0x00
ArgumentBX: dw 0x00
ArgumentCX: dw 0x00
ArgumentDX: dw 0x00
ArgumentSI: dw 0x00
ArgumentDI: dw 0x00

DoBIOSInterrupt:
    push ebp
    mov ebp , esp
    pushad

    mov eax , ebp
    mov ebx , esp
    mov dword[ProtectModeEBP] , eax
    mov dword[ProtectModeESP] , ebx
    mov eax , dword[ebp+4]
    mov dword[ReturnAddress] , eax

    mov eax , dword[ebp+8]
    mov byte[InterruptNumber] , al
    mov eax , dword[ebp+12]
    mov word[ArgumentAX] , ax
    mov eax , dword[ebp+16]
    mov word[ArgumentBX] , ax
    mov eax , dword[ebp+20]
    mov word[ArgumentCX] , ax
    mov eax , dword[ebp+24]
    mov word[ArgumentDX] , ax
    mov eax , dword[ebp+28]
    mov word[ArgumentSI] , ax
    mov eax , dword[ebp+32]
    mov word[ArgumentDI] , ax

    cli

    lgdt [RealModeGDTR]
    
    jmp 0x08:x16ProtectMode

[BITS 16]

x16ProtectMode:
    mov ax , 0x10
    mov ds , ax
    mov es , ax
    mov fs , ax
    mov gs , ax
    mov ss , ax
    
    mov eax , cr0
    mov [ProtectModeCR0] , eax
    and eax , 0x7FFFFFFE
    mov cr0 , eax
    
    jmp 0x00:RealMode

RealMode:
    mov ax , 0x00
    mov ds , ax
    mov es , ax
    mov fs , ax
    mov gs , ax
    mov ss , ax

    sti

    mov ax , word[ArgumentAX]
    mov bx , word[ArgumentBX]
    mov cx , word[ArgumentCX]
    mov dx , word[ArgumentDX]
    mov si , word[ArgumentSI]
    mov di , word[ArgumentDI]

    db 0xCD
    InterruptNumber: db 0x00

    jc Failed

    mov byte[Successed] , 0x01

ReadyForProtectMode:

	lgdt [ProtectModeGDTR]

	cli

	mov eax , cr0
    or eax , 0x01
	mov cr0 , eax

    jmp 0x08:ProtectModeAgain

[BITS 32]

ProtectModeAgain:
    mov ax , 0x10
    mov ds , ax
    mov es , ax
    mov fs , ax
    mov gs , ax
    mov ss , ax

    mov esp , dword[ProtectModeESP]
    mov ebp , dword[ProtectModeEBP]
    
    popad
    mov esp , ebp
    pop ebp

    ret 

Failed:
    mov byte[Successed] , 0x00
    jmp ReadyForProtectMode

RealModeIDTR:
    dw 0x3FF
    dd 0x00

RealModeGDTR:
    dw RealModeGDT_END-RealModeGDT
    dd RealModeGDT

RealModeGDT:
    NullSegment:
        dw 0x00
        dw 0x00
        db 0x00
        db 0x00
        db 0x00
        db 0x00
    x16CodeSegment:
        dw 0xFFFF
        dw 0x00
        db 0x00
        db 0b10011010
        db 0b00001111
        db 0x00
    x16DataSegment:
        dw 0xFFFF
        dw 0x00
        db 0x00
        db 0b10010010
        db 0b00001111
        db 0x00

RealModeGDT_END:

ProtectModeIDTR:
    dw 0x00
    dd 0x00

ProtectModeGDTR:
	dw ProtectModeGDT_END-ProtectModeGDT
	dd ProtectModeGDT

ProtectModeGDT:
	x32NullSegment:
        dw 0x00
        dw 0x00
        db 0x00
        db 0x00
        db 0x00
        db 0x00
	
	x32CodeSegment:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0b10011010
		db 0b11001111
		db 0x00
	
	x32DataSegment:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0b10010010
		db 0b11001111
		db 0x00
	
ProtectModeGDT_END:

LongModeGDT:
    ; In long mode
    .NullSegment:           ; NullSegment : Literally set to 0
        dw 0x00             ; Limit [0:15] = 0x00
        dw 0x00             ; Base [0:15] = 0x00
        db 0x00             ; Base [16:23] = 0x00
        db 0x00             ; Set All flagss to 9
        db 0x00             ; Limit [16:19] = 0x00
        db 0x00             ; Base [24:31] = 0x00
    
    .CodeSegment:           ; Code Segment (0x08)
                            ; 0x00 ~ 0xFFFFF*4096 
                            ; = 0x00 ~ 0xFFFFF000(4GB) -> Set Limit and Base to entire 4GB
                            ; so that Linear address is same as Physical Address
        dw 0xFFFF           ; Limit [0:15] = 0xFFFF
        dw 0x00             ; Base [0:15] = 0x00
        db 0x00             ; Base [16:23] = 0x00
        db 0b10011010       ; P = 1 , DPL = 0 , S = 1 ,  E = 1 , DC = 0 , RW = 1 , A = 0
                            ; Set Code Segment to executable segment (Since we have to execute code)
        db 0b10101111       ; G = 1 , D/B = 0 , L = 1 , Limit [16:19] = 0x0F
                            ; Set L bit to 1 (Long Mode Bit), and D/B to 0
                            ; (Because long mode segment is not 16bit, nor 32bit)
                            ; Set Granularity to 1 so that we can set entire 4GB area to 
                            ; Code Segment Area
        db 0x00             ; Base [24:31] = 0x00
    
    .DataSegment:           ; Data Segment (0x10)
                            ; 0x00 ~ 0xFFFFF*4096 
                            ; = 0x00 ~ 0xFFFFF000(4GB) -> Set Limit and Base to entire 4GB
                            ; so that Linear address is same as Physical Address
        dw 0xFFFF           ; Limit [0:15] = 0xFFFF
        dw 0x00             ; Base [0:15] = 0x00
        db 0x00             ; Base [16:23] = 0x00
        db 0b10010010       ; P = 1 , DPL = 0 , S = 1 , E = 0 , DC = 0 , RW = 1 , A = 0
                            ; Set Data Segment to not executable segment (Since we can't execute 
                            ; data)
        db 0b10101111       ; G = 1 , D/B = 0 , L = 1 , Limit [16:19] = 0x0F
        db 0x00             ; Base [24:31] = 0x00
LongModeGDT_END:

LongModeGDTR:
    dw LongModeGDT_END-LongModeGDT
    dd LongModeGDT
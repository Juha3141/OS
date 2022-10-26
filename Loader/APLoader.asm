[ORG 0x8000]
[BITS 16]

    jmp StartAP

IsAPStarted: db 0x01
RunningAPCoreCount: dw 0x00

StartAP:
    mov byte[IsAPStarted] , 0xFF

    cli                 ; Disable interrupt to switch to PMode
    lgdt [GDTR]         ; Load GDT

    mov eax , cr0       ; We can't write CR0 register directly(just like ES register)
    or eax , 0x01       ; Set CR0 PE bit to 1
    mov cr0 , eax
    
    jmp 0x08:APProtectMode

[BITS 32]

APProtectMode:
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

    jmp 0x08:0x100000       ; Finally jump to long modekernel (Main Kernel)
    
    jmp $

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

GDTR:
    dw GDTEND-GDT
    dd GDT

GDT:
    NullSegment:            ; NullSegment : Literally set to 0
        dw 0x00             ; Limit [0:15] = 0x00
        dw 0x00             ; Base [0:15] = 0x00
        db 0x00             ; Base [16:23] = 0x00
        db 0x00             ; Set All flagss to 9
        db 0x00             ; Limit [16:19] = 0x00
        db 0x00             ; Base [24:31] = 0x00
    
    CodeSegment:            ; Code Segment (0x08)
                            ; 0x00 ~ 0xFFFFF*4096 
                            ; = 0x00 ~ 0xFFFFF000(4GB) -> Set Limit and Base to entire 4GB
                            ; so that Linear address is same as Physical Address
        dw 0xFFFF           ; Limit [0:15] = 0xFFFF
        dw 0x00             ; Base [0:15] = 0x00
        db 0x00             ; Base [16:23] = 0x00
        db 0b10011010       ; P = 1 , DPL = 0 , S = 1 ,  E = 1 , DC = 0 , RW = 1 , A = 0
                            ; Set Code Segment to executable segment (Since we have to execute code)
        db 0b11001111       ; G = 1 , D/B = 1 , L = 0 , Limit [16:19] = 0x0F
                            ; Set Granularity to 1 so that we can set entire 4GB area to 
                            ; Code Segment Area
        db 0x00             ; Base [24:31] = 0x00
    
    DataSegment:            ; Data Segment (0x10)
                            ; 0x00 ~ 0xFFFFF*4096 
                            ; = 0x00 ~ 0xFFFFF000(4GB) -> Set Limit and Base to entire 4GB
                            ; so that Linear address is same as Physical Address
        dw 0xFFFF           ; Limit [0:15] = 0xFFFF
        dw 0x00             ; Base [0:15] = 0x00
        db 0x00             ; Base [16:23] = 0x00
        db 0b10010010       ; P = 1 , DPL = 0 , S = 1 , E = 0 , DC = 0 , RW = 1 , A = 0
                            ; Set Data Segment to not executable segment (Since we can't execute 
                            ; data)
        db 0b11001111       ; G = 1 , D/B = 1 , L = 0 , Limit [16:19] = 0x0F
        db 0x00             ; Base [24:31] = 0x00

GDTEND:                     ; End of GDT Table

times (2048-($-$$)) db 0x00
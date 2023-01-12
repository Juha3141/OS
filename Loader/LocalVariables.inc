;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; File "LocalVariables.inc"                              ;;
;; Dscription : Contains local variables for Kernel32.asm ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

DAP:                ; DAP Area(Disk Address Packet)
    db 0x10         ; Size of the DAP
    db 0x00
    dw 0x00
    dd 0x00
    dd 0x00
    dd 0x00

DriveNumberLocation: equ 0x7C00+0x1F
FoundKernelMessage: db "Found Kernel" , 0x00
ErrorMessage: db "Error ["
ErrorCode: dd 0x00
           db "]" , 0x00
MainKernelFileName: db "KERNEL.KRN"
APLoaderFileName: db "APLOADER.LDR"
MainKernelLocation: dd 0x00
APLoaderLocation: dd 0x00
APLoaderSize: dw 0x01
MainKernelSize: dd 0x00
MainKernelSectorSize: dw 0x00
LoopingCount: db 0x00
LeftSectorCountToRead: dw 0x00

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
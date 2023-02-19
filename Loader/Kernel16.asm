;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; File "Kernel16.asm"                                  ;;
;; Description : Source of the 16bit Kernel,            ;;
;; which is a transitional part between 16bit and 32bit ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[ORG 0x8400]
[BITS 16]

    jmp Kernel16        ; Skip the local variable parts

Checksum:               ; Size : 16 bytes
    dd 0xCAFEBABE
    dd 0x4B45524E
    dd 0x454C3438
    dd 0x00000000

DAP:                ; DAP Area(Disk Address Packet)
    db 0x10         ; Size of the DAP : 16 bytes
    db 0x00
    dw 0x00
    dd 0x00
    dd 0x00
    dd 0x00

EnableGraphicMode: db 0x00

Kernel16:
    mov ax , 0x00               ; Initialize segments as we did in boot loader
    mov ds , ax
    
    cmp byte[EnableGraphicMode] , 1
    je SwitchToGraphicMode

L1:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;     Initialization for E820 Memory Map     ;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor ax , ax                 ; Set ES register to 0x00 - Location saving E820 Map is ES:DI
	mov es , ax                 ; (ES can't be written directly)
	mov di , 0xE000             ; 0x00:0xA000 is where all the memory maps are saved.
	
	mov ebx , 0x00              ; Always set EBX to 0

	CheckMemory:
		mov eax , 0xE820
		mov edx , 0x534D4150    ; Signature(it's actually "SMAP")
		mov ecx , 24            ; Size of the one memory map
		int 0x15

		add di , 24             ; Increase the offset

		cmp ebx , 0x00
        jne CheckMemory         ; If EBX is 0, it means reading memory map is done.
    
    EnableA20:                  ; DON'T FORGET TO ENABLE A20
        mov ax , 0x2401         ; Enable A20 so the protect mode kernel can access above 1MB
        int 0x15                ; memory area
                                ; Enable A20 by using BIOS service

    cli                 ; Disable interrupt to switch to PMode
    lgdt [GDTR]         ; Load GDT

    mov eax , cr0       ; We can't write CR0 register directly(just like ES register)
    or eax , 0x01       ; Set CR0 PE bit to 1
    mov cr0 , eax
    
    jmp 0x08:0x8C00     ; Far jump to 32bit kernel
    
SwitchToGraphicMode:
    mov ax , 0x00
    mov es , ax
    mov di , 0x8C09
    mov ax , 0x4F01
    mov cx , word[0x8C07]
    int 0x10

    mov ax , 0x4F02
    mov bx , word[0x8C07]
    or bx , 0x4000
    int 0x10
    
    jmp L1

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
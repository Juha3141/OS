;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; File "Kernel32.asm"                                                      ;;
;; Description : Loads Long Mode Kernel(Main Kernel) at appointed location, ;;
;; creates Page Map Level 4 Entry, and switches to Long Mode                ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[BITS 32]
    
global Kernel32

Kernel32:
    jmp 0x08:Start                              ; Instruction size : 7 bytes

                                                ; Fields that is going to be used for 16bit kernel
GraphicMode: dw 0x118                           ; Location : 0x8C00+7 = 0x8C07
                                                ; Reserved 512 bytes for VBE information structure
VBEInfoStructure: resb 512                      ; Location : 0x8C00+7+2 = 0x8C09

Start:                                          ; Location : 0x8C00+7+2+512 = 0x8E09
    mov ax , 0x10           ; Set data segment and other segments to 0x10
    mov ds , ax             ; 0x10 is data segment which we previously set in GDT
    mov es , ax
    mov fs , ax
    mov gs , ax
    mov ss , ax

    mov ebp , 0xAFF8        ; Stack Location : 0x9FF8 (Same location as 16bit kernel)
    mov esp , 0xAFF8        ; No limit(There's no segment to limit the stack, only data and code exist)

    mov esi , 0x500+2048+2048   ; Location of Directory Sector (We are going to find Main Kernel
    xor ebx , ebx               ; as we did in boot loader before)
    
    .L1:
        add esi , ebx           ; EBX : Offset
        mov bl , byte[esi]      ; BL register saves first byte of Directory Record - 
                                ; which is the length of the Directory Record
        mov edi , esi           ; EDI = ESI
        mov ecx , dword[esi+2]  ; Location of the file location : At 0x02 of the Directory Record
        mov dword[MainKernelLocation] , ecx ; Save location of the file - if it is Kernel File , 
                                            ; It is saved in MainKernelLocation
                                            ; If it isn't, overwrite to new file location
        mov ecx , dword[esi+2+4+4]          ; Location of the file size : At 0x0A of the Directory Record
        mov dword[MainKernelSize] , ecx     ; (Also save the value, if it's not, overwrite it)
        add edi , 0x21          ; Is my english bad?

        push edi
        push MainKernelFileName
        push 10
        call memcmp             ; Compare file name, sets AX register to 1 if the file name is same
        add esp , 12

        cmp eax , 0x01          ; If AX = 1 -> go to .FoundFile (We found the file!)
        je .FoundKernel

        push edi
        push APLoaderFileName
        push 12
        call memcmp
        add esp , 12
        
        cmp eax , 0x01
        je .FoundAPLoader

        jmp .L1
    
    .FoundAPLoader:
        mov ecx , dword[esi+2]
        mov dword[APLoaderLocation] , ecx   ; Save location of the file (as we did previously)
        jmp .L1
    
    .FoundKernel:
        mov esi , MainKernelLocation        ; Save the location of the kernel
        
        cmp dword[MainKernelSize] , 0       ; If size of the kernel file is zero -> Error
        mov dword[ErrorCode] , 0x5A53464B   ; Handling Error : KFSZ(Kernel File Size Zero)
        je Error

        xor edx , edx                       ; DON'T FORGET TO CLEAR EDX BEFORE DIVIDING SOMETHING
        mov eax , dword[MainKernelSize]     ; Divide kernel size to 2048 so that we can figure out
        mov ebx , 2048                      ; the sector count of kernel file
        idiv ebx
        
        cmp edx , 0                         ; If EDX > 0 -> Need one more sector
        ja .AddEAX                          ; Add EAX (Add one more sector), EAX is sector count of kernel file
        
        jmp .Continue
        
    .AddEAX:
        add eax , 1
        
    .Continue:
        mov ecx , eax
        xor edx , edx
        mov ebx , 14    ; Divide the sector count by 14
                        ; (14 is the most sector count that we can read in one operation)
                        ; (Memory location to save kernel file temporary : 0x500~0x7C00
                        ; = 0x7700, 0x7700/0x800 = 0x0E(14) sectors can be saved in the memory area)
        idiv ebx        ; EAX : Sector Count to Read
                        ; ECX : Looping Count (How many times should we loop)
                        ; EDX : Left Sector Count to Read(Remainder of the result)
        xchg eax , ecx                      ; Exchange EAX and ECX value
        mov word[MainKernelSectorSize] , ax ; Save calculated values
        mov byte[LoopingCount] , cl
        mov word[LeftSectorCountToRead] , dx

        mov edi , 0x100000                  ; Real location of the Kernel file
        mov ebx , dword[MainKernelLocation] ; Sector location of Kernel File
        cmp ecx , 0
        je .L3
        .L2:
            mov esi , DAP
            mov byte[RMA_AH] , 0x42
            mov al , byte[DriveNumberLocation]
            mov byte[RMA_DL] , al
            mov word[RMA_SI] , si

            mov word[DAP+2] , 14
            mov dword[DAP+4] , 0x500
            mov dword[DAP+8] , ebx
            mov dword[DAP+12] , 0x00
            push 0x13
            call DoBIOSInterrupt

            cmp byte[RMA_Successed] , 0
            mov dword[ErrorCode] , 0x3152454B 
            je Error

            push 0x500
            push edi
            push 2048*14
            call memcpy

            add edi , 2048*14
            add ebx , 14
            
            loop .L2
        
        mov edi , 0x100000
        cmp dword[edi] , 0x00
        mov dword[ErrorCode] , 0xEEEEEEEE
        je Error

        .L3:
            cmp word[LeftSectorCountToRead] , 0x00
            je LoadAPLoader                      ; Done

            mov dx , word[LeftSectorCountToRead]

            mov esi , DAP
            mov byte[RMA_AH] , 0x42
            mov al , byte[DriveNumberLocation]
            mov byte[RMA_DL] , al
            mov word[RMA_SI] , si
            
            mov word[DAP+2] , dx
            mov dword[DAP+4] , 0x500
            mov dword[DAP+8] , ebx
            mov dword[DAP+12] , 0x00
            push 0x13
            call DoBIOSInterrupt
            
            cmp byte[RMA_Successed] , 0
            mov dword[ErrorCode] , 0x3252454B
            je Error
            
            imul edx , 2048

            push edi
            push 0x500
            push edx
            call memcpy
        
    LoadAPLoader:
        
        mov esi , DAP
        mov byte[RMA_AH] , 0x42
        mov al , byte[DriveNumberLocation]
        mov byte[RMA_DL] , al
        mov word[RMA_SI] , si
        mov cx , word[APLoaderSize]
        mov word[DAP+2] , cx
        mov ecx , dword[APLoaderLocation]
        mov dword[DAP+4] , 0x8000
        mov dword[DAP+8] , ecx
        mov dword[DAP+12] , 0x00
        push 0x13
        call DoBIOSInterrupt

    CreatePML4Entry:
                                            ; PML4 Entry - One entry can map 512GBs of memory
        mov edi , 0x10000                   ; Location : 0x10000
        mov eax , 0x10000+(512*8)           ; Pointing 0x11000 (Location of PDPT entry)
        or eax , 0b000000000011             ; RW = 1 , P = 1
        mov dword[edi] , eax
        mov dword[edi+4] , 0x00
        xor eax , 0b000000000011
        mov edi , eax
                                            ; PDPT Entry - One entry can map 1GB of memory
                                            ; Location : 0x10000+(512*8) = 0x11000
        mov eax , 0x10000+(512*8)+(512*8)   ; EAX : Lower bit 
        or eax , 0b000000000011             ; RW = 1 , P = 1
        mov ebx , 0x00                      ; EBX : Upper bit
        mov ecx , 128                       ; Map 128GBs of memory
        
        .L1:
            mov dword[edi] , eax            ; EDI : Lower bits of Base Address (Since Intel processor follows little endian)
            mov dword[edi+4] , ebx          ; EDI+4 : Higher bits of Base Address
            
            add eax , 0x1000                ; Add size of 512 PDE(1GB, which equals to 1 PDPT Entry)
            add edi , 8                     ; Add size of one entry data

            loop .L1
                                            ; Finally PDE Entry - One entry can map 2MBs of memory
        mov edi , 0x10000+(512*8)+(512*8)   ; Location : 0x10000+(512*8)+(512*8) = 0x12000

        xor eax , eax
        xor ebx , ebx
        mov ecx , 512*128                   ; Number of 2MB Page Directory Entry
        
        .L2:
            mov dword[edi] , eax            ; EAX : Lower bit + Flags
            mov dword[edi+4] , 0x00         ; EBX : Upper bit

            or dword[edi] , 0b000010000011  ; PS = 1 , RW = 1 , P = 1
                                            ; Set PS bit to 1 so that we can use 2MB PML4 paging
            add edi , 8                     ; Add size of one entry(8 Bytes)
            add eax , 0x200000              ; Add size of one page(2MB, which is 0x200000)
            
            cmp eax , 0x00                  ; Jump of EAX value is overflowed(= 0x00, which actually is 4294967296, 0x100000000)
            je .L3                          ; In this case, we have to increase the upper value of Base Address, so we are going to increase
                                            ; EBX to one

            loop .L2
        
        .L3:
            add bl , 0x01                  ; Increase EBX to 1
            loop .L2
        
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

Error:
    push 0
    push ErrorMessage
    push 0x04
    call PrintString

    jmp $


%include "LocalVariables.inc"
%include "Kernel32.inc"

times (4096-($-$$)) db 0x00

; Font data
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,248,24,24,24,24,24,24,24,0,0
db 24,24,24,24,24,24,31,0,0,0,0,0,0,0,0,0
db 24,24,24,24,24,24,248,0,0,0,0,0,0,0,0,0
db 24,24,24,24,24,24,24,24,24,24,24,24,24,24,0,0
db 0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0
db 0,0,0,0,60,126,126,126,60,0,0,0,0,0,0,0
db 255,255,255,255,231,195,195,195,231,255,255,255,255,255,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,31,7,15,15,24,24,56,108,108,56,0,0,0,0
db 0,0,0,127,65,89,69,93,65,89,65,127,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,7,63,55,63,51,51,51,55,119,112,0,0,0,0
db 0,0,126,126,60,60,126,60,60,126,126,0,0,0,0,0
db 24,24,24,24,24,24,255,24,24,24,24,24,24,24,0,0
db 0,3,7,15,31,63,127,63,31,15,7,3,0,0,0,0
db 24,60,126,24,24,24,24,24,24,24,126,60,24,0,0,0
db 0,54,54,54,54,54,54,54,54,0,0,54,54,0,0,0
db 0,62,126,126,126,126,126,62,30,30,30,30,30,0,0,0
db 24,24,24,24,24,24,255,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,255,24,24,24,24,24,24,24,0,0
db 24,24,24,24,24,24,248,24,24,24,24,24,24,24,0,0
db 0,24,60,126,24,24,24,24,24,24,24,24,24,0,0,0
db 24,24,24,24,24,24,31,24,24,24,24,24,24,24,0,0
db 0,0,0,0,0,12,6,127,6,12,0,0,0,0,0,0
db 0,0,0,0,0,24,48,127,48,24,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,24,24,24,24,24,24,24,0,24,24,0,0,0,0
db 0,0,108,108,108,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,60,60,254,60,60,120,254,120,120,0,0,0,0
db 0,0,4,62,108,104,120,62,15,11,11,126,16,16,0,0
db 0,0,98,148,156,104,24,16,38,105,73,198,0,0,0,0
db 0,0,56,108,108,108,56,126,222,206,206,123,0,0,0,0
db 0,0,24,24,24,0,0,0,0,0,0,0,0,0,0,0
db 0,0,4,12,24,24,48,48,48,48,48,24,24,12,4,0
db 0,0,32,48,24,24,12,12,12,12,12,24,24,48,32,0
db 0,0,24,90,60,60,90,24,0,0,0,0,0,0,0,0
db 0,0,0,0,0,24,24,24,255,24,24,24,0,0,0,0
db 0,0,0,0,0,0,0,0,0,24,28,12,12,48,0,0
db 0,0,0,0,0,0,0,0,60,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0
db 0,0,2,6,4,12,12,24,24,16,48,32,96,0,0,0
db 0,0,0,56,108,198,206,214,230,198,108,56,0,0,0,0
db 0,0,0,24,56,88,24,24,24,24,24,126,0,0,0,0
db 0,0,0,60,70,6,6,12,24,48,32,126,0,0,0,0
db 0,0,0,124,6,6,6,60,6,6,6,124,0,0,0,0
db 0,0,0,28,60,60,108,76,204,254,12,12,0,0,0,0
db 0,0,0,124,96,96,124,6,6,6,12,120,0,0,0,0
db 0,0,0,30,48,96,110,115,99,99,35,62,0,0,0,0
db 0,0,0,126,6,12,12,24,24,48,48,96,0,0,0,0
db 0,0,0,62,99,99,118,30,119,99,99,62,0,0,0,0
db 0,0,0,62,102,99,99,99,63,3,6,60,0,0,0,0
db 0,0,0,0,0,24,24,0,0,0,24,24,0,0,0,0
db 0,0,0,0,0,24,24,0,0,24,28,12,12,48,0,0
db 0,0,0,0,4,12,24,48,48,24,12,4,0,0,0,0
db 0,0,0,0,0,0,0,126,0,126,0,0,0,0,0,0
db 0,0,0,0,32,48,24,12,12,24,48,32,0,0,0,0
db 0,0,56,14,6,6,60,48,48,0,48,48,0,0,0,0
db 0,0,28,36,66,90,186,186,186,186,186,188,128,68,124,0
db 0,0,0,56,40,44,108,76,68,254,198,130,0,0,0,0
db 0,0,0,124,102,102,102,124,102,102,102,124,0,0,0,0
db 0,0,0,28,50,96,96,96,96,96,50,28,0,0,0,0
db 0,0,0,124,102,99,99,99,99,99,102,124,0,0,0,0
db 0,0,0,124,96,96,96,124,96,96,96,124,0,0,0,0
db 0,0,0,124,96,96,96,124,96,96,96,96,0,0,0,0
db 0,0,0,60,98,192,192,206,198,198,102,62,0,0,0,0
db 0,0,0,99,99,99,99,127,99,99,99,99,0,0,0,0
db 0,0,0,126,24,24,24,24,24,24,24,126,0,0,0,0
db 0,0,0,124,12,12,12,12,12,12,76,56,0,0,0,0
db 0,0,0,102,108,108,104,120,120,108,108,102,0,0,0,0
db 0,0,0,96,96,96,96,96,96,96,96,124,0,0,0,0
db 0,0,0,198,198,170,170,170,146,130,130,130,0,0,0,0
db 0,0,0,115,115,115,123,107,111,103,103,103,0,0,0,0
db 0,0,0,56,108,198,198,198,198,198,108,56,0,0,0,0
db 0,0,0,124,102,102,102,102,124,96,96,96,0,0,0,0
db 0,0,0,56,108,198,198,198,198,198,108,56,24,24,14,0
db 0,0,0,124,102,102,102,120,108,108,100,102,0,0,0,0
db 0,0,0,56,100,96,112,60,14,6,70,60,0,0,0,0
db 0,0,0,126,24,24,24,24,24,24,24,24,0,0,0,0
db 0,0,0,99,99,99,99,99,99,99,99,62,0,0,0,0
db 0,0,0,97,99,99,34,54,54,20,28,28,0,0,0,0
db 0,0,0,130,130,130,146,170,170,170,198,198,0,0,0,0
db 0,0,0,230,124,60,56,56,56,108,102,198,0,0,0,0
db 0,0,0,195,102,102,60,60,24,24,24,24,0,0,0,0
db 0,0,0,126,4,12,8,24,16,48,32,126,0,0,0,0
db 0,0,60,48,48,48,48,48,48,48,48,48,48,48,60,0
db 0,0,96,32,48,16,24,8,12,4,6,2,3,0,0,0
db 0,0,60,12,12,12,12,12,12,12,12,12,12,12,60,0
db 0,0,0,16,40,108,108,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0
db 0,96,48,24,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,28,38,6,62,102,102,62,0,0,0,0
db 0,0,96,96,96,110,115,99,99,99,102,124,0,0,0,0
db 0,0,0,0,0,30,48,96,96,96,112,30,0,0,0,0
db 0,0,3,3,3,31,51,99,99,99,103,63,0,0,0,0
db 0,0,0,0,0,30,51,99,127,96,112,31,0,0,0,0
db 0,0,15,24,24,24,126,24,24,24,24,24,0,0,0,0
db 0,0,0,0,0,63,102,102,102,124,96,126,99,99,62,0
db 0,0,96,96,96,124,118,102,102,102,102,102,0,0,0,0
db 0,0,24,24,0,120,24,24,24,24,24,126,0,0,0,0
db 0,0,6,6,0,126,6,6,6,6,6,6,6,78,60,0
db 0,0,96,96,96,102,108,120,120,108,108,102,0,0,0,0
db 0,0,120,24,24,24,24,24,24,24,24,126,0,0,0,0
db 0,0,0,0,0,255,219,219,219,219,219,219,0,0,0,0
db 0,0,0,0,0,124,118,102,102,102,102,102,0,0,0,0
db 0,0,0,0,0,30,51,99,99,99,114,60,0,0,0,0
db 0,0,0,0,0,126,115,99,99,99,102,124,96,96,96,0
db 0,0,0,0,0,31,51,99,99,99,103,63,3,3,3,0
db 0,0,0,0,0,108,118,102,96,96,96,96,0,0,0,0
db 0,0,0,0,0,60,96,112,60,6,6,124,0,0,0,0
db 0,0,0,48,48,252,48,48,48,48,48,28,0,0,0,0
db 0,0,0,0,0,102,102,102,102,102,110,62,0,0,0,0
db 0,0,0,0,0,97,99,34,54,54,20,28,0,0,0,0
db 0,0,0,0,0,130,130,146,214,238,108,108,0,0,0,0
db 0,0,0,0,0,195,102,60,24,44,102,199,0,0,0,0
db 0,0,0,0,0,99,99,34,54,52,28,24,24,16,224,0
db 0,0,0,0,0,126,12,12,24,16,48,126,0,0,0,0
db 0,0,28,48,48,48,48,48,224,48,48,48,48,48,28,0
db 0,24,24,24,24,24,24,24,24,24,24,24,24,24,24,0
db 0,0,112,24,24,24,24,24,14,24,24,24,24,24,112,0
db 0,0,0,0,0,0,0,115,219,206,0,0,0,0,0,0
db 0,0,240,192,192,192,192,192,192,192,192,240,0,0,0,0
db 0,0,0,28,50,96,252,96,252,96,50,28,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,24,28,12,12,48,0,0
db 0,0,14,8,24,24,24,126,24,16,16,48,48,224,0,0
db 0,0,0,0,0,0,0,0,0,0,54,54,18,108,0,0
db 0,0,0,0,0,0,0,0,0,0,219,219,0,0,0,0
db 0,0,24,24,126,24,24,24,24,24,0,0,0,0,0,0
db 0,0,24,24,126,24,24,24,24,126,24,24,0,0,0,0
db 0,16,56,56,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,96,176,177,110,112,128,102,219,219,102,0,0,0,28
db 28,8,0,56,100,96,112,60,14,6,70,60,0,0,0,0
db 0,0,0,0,0,12,24,48,48,24,12,0,0,0,0,0
db 0,0,0,62,108,204,204,206,204,204,108,126,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28
db 28,8,0,126,4,12,8,24,16,48,32,126,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,6,24,24,28,12,0,0,0,0,0,0,0,0,0
db 0,0,24,28,12,12,48,0,0,0,0,0,0,0,0,0
db 0,0,54,72,108,108,0,0,0,0,0,0,0,0,0,0
db 0,0,54,54,18,108,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,56,124,124,124,56,0,0,0,0,0
db 0,0,0,0,0,0,0,0,126,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0
db 0,52,52,44,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,250,90,94,86,0,0,0,0,0,0,0,0,0
db 0,56,56,16,0,60,96,112,60,6,6,124,0,0,0,0
db 0,0,0,0,0,48,24,12,12,24,48,0,0,0,0,0
db 0,0,0,0,0,118,219,219,223,216,216,127,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,56,56,16,0,126,12,12,24,16,48,126,0,0,0,0
db 108,108,0,195,102,102,60,60,24,24,24,24,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,24,24,0,24,24,24,24,24,24,24,0
db 0,0,4,4,30,56,104,104,104,104,62,24,16,16,0,0
db 0,0,0,28,50,48,48,124,48,48,48,126,0,0,0,0
db 0,0,0,0,66,126,102,102,102,126,66,0,0,0,0,0
db 0,0,0,195,102,52,60,24,126,24,126,24,0,0,0,0
db 0,24,24,24,24,24,24,0,0,24,24,24,24,24,24,0
db 0,0,30,48,48,60,110,103,115,59,14,6,6,124,0,0
db 0,0,54,54,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,60,66,157,177,177,177,157,66,60,0,0,0,0
db 0,0,0,120,4,60,100,124,0,124,0,0,0,0,0,0
db 0,0,0,0,0,50,54,108,108,54,50,0,0,0,0,0
db 0,0,0,0,0,0,0,0,126,6,6,0,0,0,0,0
db 0,0,0,0,0,0,0,0,60,0,0,0,0,0,0,0
db 0,0,60,66,153,145,153,66,60,0,0,0,0,0,0,0
db 0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,60,102,102,102,60,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,24,24,126,24,24,0,126,0,0,0,0
db 0,0,60,12,12,24,60,0,0,0,0,0,0,0,0,0
db 0,0,62,6,28,6,60,0,0,0,0,0,0,0,0,0
db 0,6,12,24,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,102,102,102,102,102,110,127,96,96,96,0
db 0,0,31,123,123,123,123,59,3,3,3,3,35,30,0,0
db 0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,24,48,0,0
db 0,0,24,56,24,24,60,0,0,0,0,0,0,0,0,0
db 0,0,0,60,102,102,102,60,0,126,0,0,0,0,0,0
db 0,0,0,0,0,200,108,54,54,108,200,0,0,0,0,0
db 0,0,98,228,44,40,24,16,38,110,79,194,0,0,0,0
db 0,0,98,228,44,40,24,16,46,98,68,206,0,0,0,0
db 0,0,242,116,60,232,24,16,38,110,79,194,0,0,0,0
db 0,0,0,0,0,12,12,0,12,12,60,96,96,112,28,96
db 48,24,0,56,40,44,108,76,68,254,198,130,0,0,0,6
db 12,24,0,56,40,44,108,76,68,254,198,130,0,0,0,8
db 28,28,0,56,40,44,108,76,68,254,198,130,0,0,0,18
db 42,36,0,56,40,44,108,76,68,254,198,130,0,0,0,0
db 108,108,0,56,40,44,108,76,68,254,198,130,0,0,0,0
db 56,40,56,56,40,44,108,76,68,254,198,130,0,0,0,0
db 0,0,0,31,60,44,44,79,76,252,140,143,0,0,0,0
db 0,0,0,28,50,96,96,96,96,96,50,30,12,24,0,96
db 48,24,0,124,96,96,96,124,96,96,96,124,0,0,0,6
db 12,24,0,124,96,96,96,124,96,96,96,124,0,0,0,8
db 28,28,0,124,96,96,96,124,96,96,96,124,0,0,0,0
db 108,108,0,124,96,96,96,124,96,96,96,124,0,0,0,96
db 48,24,0,126,24,24,24,24,24,24,24,126,0,0,0,6
db 12,24,0,126,24,24,24,24,24,24,24,126,0,0,0,8
db 28,28,0,126,24,24,24,24,24,24,24,126,0,0,0,0
db 108,108,0,126,24,24,24,24,24,24,24,126,0,0,0,0
db 0,0,0,124,102,99,99,251,99,99,102,124,0,0,0,18
db 42,36,0,115,115,115,123,107,111,103,103,103,0,0,0,96
db 48,24,0,56,108,198,198,198,198,198,108,56,0,0,0,6
db 12,24,0,56,108,198,198,198,198,198,108,56,0,0,0,8
db 28,28,0,56,108,198,198,198,198,198,108,56,0,0,0,18
db 42,36,0,56,108,198,198,198,198,198,108,56,0,0,0,0
db 108,108,0,56,108,198,198,198,198,198,108,56,0,0,0,0
db 0,0,0,0,0,0,102,124,24,60,102,0,0,0,0,0
db 0,0,12,60,108,222,222,214,214,246,108,120,96,0,0,96
db 48,24,0,99,99,99,99,99,99,99,99,62,0,0,0,6
db 12,24,0,99,99,99,99,99,99,99,99,62,0,0,0,8
db 28,28,0,99,99,99,99,99,99,99,99,62,0,0,0,0
db 108,108,0,99,99,99,99,99,99,99,99,62,0,0,0,6
db 12,24,0,195,102,102,60,60,24,24,24,24,0,0,0,0
db 0,0,0,96,124,102,102,102,102,124,96,96,0,0,0,0
db 0,0,60,102,102,110,108,108,103,99,99,110,0,0,0,0
db 0,96,48,24,0,28,38,6,62,102,102,62,0,0,0,0
db 0,6,12,24,0,28,38,6,62,102,102,62,0,0,0,0
db 0,16,56,56,0,28,38,6,62,102,102,62,0,0,0,0
db 0,52,52,44,0,28,38,6,62,102,102,62,0,0,0,0
db 0,0,54,54,0,28,38,6,62,102,102,62,0,0,0,0
db 0,28,20,28,0,28,38,6,62,102,102,62,0,0,0,0
db 0,0,0,0,0,254,155,27,127,216,216,239,0,0,0,0
db 0,0,0,0,0,30,48,96,96,96,112,62,12,24,0,0
db 0,96,48,24,0,30,51,99,127,96,112,31,0,0,0,0
db 0,6,12,24,0,30,51,99,127,96,112,31,0,0,0,0
db 0,16,56,56,0,30,51,99,127,96,112,31,0,0,0,0
db 0,0,54,54,0,30,51,99,127,96,112,31,0,0,0,0
db 0,96,48,24,0,120,24,24,24,24,24,126,0,0,0,0
db 0,6,12,24,0,120,24,24,24,24,24,126,0,0,0,0
db 0,16,56,56,0,120,24,24,24,24,24,126,0,0,0,0
db 0,0,54,54,0,120,24,24,24,24,24,126,0,0,0,0
db 0,0,30,60,14,62,51,99,99,99,102,60,0,0,0,0
db 0,52,52,44,0,124,118,102,102,102,102,102,0,0,0,0
db 0,96,48,24,0,30,51,99,99,99,114,60,0,0,0,0
db 0,6,12,24,0,30,51,99,99,99,114,60,0,0,0,0
db 0,16,56,56,0,30,51,99,99,99,114,60,0,0,0,0
db 0,52,52,44,0,30,51,99,99,99,114,60,0,0,0,0
db 0,0,54,54,0,30,51,99,99,99,114,60,0,0,0,0
db 0,0,0,0,0,24,24,0,126,0,24,24,0,0,0,0
db 0,0,0,0,6,30,63,111,123,123,126,60,48,0,0,0
db 0,96,48,24,0,102,102,102,102,102,110,62,0,0,0,0
db 0,6,12,24,0,102,102,102,102,102,110,62,0,0,0,0
db 0,16,56,56,0,102,102,102,102,102,110,62,0,0,0,0
db 0,0,54,54,0,102,102,102,102,102,110,62,0,0,0,0
db 0,6,12,24,0,99,99,34,54,52,28,24,24,16,224,0
db 0,0,96,96,96,110,115,99,99,99,102,124,96,96,96,0
db 0,0,54,54,0,99,99,34,54,52,28,24,24,16,224
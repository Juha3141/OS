;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; File "Kernel32.asm"                                                      ;;
;; Description : Loads Long Mode Kernel(Main Kernel) at appointed location, ;;
;; creates Page Map Level 4 Entry, and switches to Long Mode                ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[BITS 32]

global Kernel32

Kernel32:
    mov ax , 0x10           ; Set data segment and other segments to 0x10
    mov ds , ax             ; 0x10 is data segment which we previously set in GDT
    mov es , ax
    mov fs , ax
    mov gs , ax
    mov ss , ax

    mov ebp , 0x9FF8        ; Stack Location : 0x9FF8 (Same location as 16bit kernel)
    mov esp , 0x9FF8        ; No limit(There's no segment to limit the stack, only data and code exist)

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
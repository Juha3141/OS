;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; File "BootLoader.asm"                                                       ;;
;; Description : Source of the boot loader of the Main OS                      ;;
;; Loads file KernelLoader.ldr to appointed location(which is location 0x8400) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[ORG 0x7C00]
[BITS 16]

    jmp Start       ; Skip the local variable parts

                                                ; Description                                      Offset
PrimaryVolumeDescriptorAddress: dw 0x500        ;                                                  0x02
PathTableAddress: dw 0x500+2048                 ; One sector after PVD area                        0x04
DirectorySectorAddress: dw 0x500+2048+2048      ; One sector after Directory Sector area           0x06
DirectoryRecordSize: db 0x00                    ; Temporary Variables                               0x08
KernelLoaderFileName: db "KERN~DDP.LDR" , 0x00  ; Name of the Kernel Loader                        0x09
                                                ; Note : If you're in windows, change KernelLoaderFileName to "KERNEL~1.LDR", 
                                                ; or if you're in linux, change it to "KERN~DDP.LDR".
KernelLoaderLocation: dd 0x00                   ; Location of the Kernel Loader                    0x12
KernelLoaderSectorSize: dd 0x00                 ; Size of the Kernel Loader(Static)                0x16
StaticKernelLoaderStartAddress: dd 0x8400       ; Address of the Kernel Loader                     0x18
StaticAPLoaderStartAddress: dd 0x8000           ; Address of the AP Loader                         0x1B

DriveNumber: db 0x00                            ; Save number of the driver that is booted         0x1F
                                                ; (for the situation that is using EDX register)

DAP:                ; DAP Area(Disk Address Packet)                                           0x1C
    db 0x10         ; Size of the DAP
    db 0x00
    dw 0x00
    dd 0x00
    dd 0x00
    dd 0x00

Start:
    mov si , 0x8000
    mov byte[si] , 0x01
    xor ax , ax     ; Set DS segment to 0x00 : Because of 'ORG 0x7C00', All local variables are
                    ; located in the actual existing address
    mov ds , ax

    mov ax , 0xAC0   ; End of the stack   : SS*16 = 0x9C00
    mov ss , ax     ; Start of the stack : SS*16+BP = 0x9FF8
    mov sp , 0x3F8
    mov bp , 0x3F8

    mov byte[DriveNumber] , dl
    
    ReadPVD:
        mov ah , 0x42   ; Extended Read Sector From Drive : 
                        ; SI : DAP Address
        mov si , DAP
        mov byte[si+2] , 0x01   ; Sectors count to read (Bytes Per Sector : 2048)
        xor ebx , ebx
        mov bx , [PrimaryVolumeDescriptorAddress] ; Primary Volume Descriptor is going to be saved in 
                                                  ; Physical Memory 0x500
        mov dword[si+4] , ebx   ; Physical memory that the data is going to be copied
        mov dword[si+8] , 0x10  ; Sector offset to read (PVD, Primary Volume Descriptor starts at 
                                ; #16 sector)
        mov dword[si+12] , 0x00 ; Empty(Originally start of the sector field was 8 byte long, but
                                ; in real mode, we can't do that, so we just empty the field)
        mov dl , byte[DriveNumber]
        int 0x13 ; Saves PVD to Address 0x500 (PVD provides the sector address of root directory)
        
        cmp dword[0x501] , 0x30304443 ; Check if it's ISO9660 (CD001 - signature)
        jne HandleError               ; If it's not equal, return error - not ISO9660 file system
        cmp byte[0x505] , 0x31        ; Check the signature of ISO9660
        jne HandleError

    ReadPathTable:
        mov ah , 0x42
        mov si , DAP
        mov ecx , dword[0x500+0x8C]   ; Offset 0x8C : Where Directory Sector is stored
        mov byte[si+2] , 0x01         ; Path Table Size : 1 sector
        mov dword[si+4] , 0x500+2048  ; Right after PVD Area
        mov dword[si+8] , ecx         ; The location of Directory Sector is saved in ecx register
        mov dword[si+12] , 0x00
        mov dl , byte[DriveNumber]

        int 0x13

    ReadDirectorySector:
        mov ah , 0x42
        mov si , DAP
        mov ecx , dword[0x500+2048+0x02]  ; The first directory sector is a root directory, indicating address
        mov byte[si+2] , 0x01             ; Which is in offset 0x02
        mov dword[si+4] , 0x500+2048+2048 ; Right after Path Table Area
        mov dword[si+8] , ecx
        mov dword[si+12] , 0x00
        mov dl , byte[DriveNumber]

        int 0x13
    
    FindKernelLoader:
        mov si , 0x500+2048+2048
        xor bx , bx
        .Loop:
            add si , bx
            mov bl , byte[si]       ; BL register saves first byte of Directory Record - 
                                    ; which is the length of the Directory Record
            mov di , si
            mov ecx , dword[si+2]
            mov dword[KernelLoaderLocation] , ecx   ; LocationL 
            mov ecx , dword[si+2+4+4]
            mov dword[KernelLoaderSectorSize] , ecx ; DataLengthL
            add di , 0x21

            push di
            call CompareFileName    ; Compare file name, sets AX register to 1 if the file name is same
            add sp , 4

            cmp ax , 0x01           ; If AX = 1 -> go to .FoundFile
            jne .Loop

        .FoundFile:
            xor edx , edx
            mov eax , dword[KernelLoaderSectorSize]
            mov ebx , 2048
            div ebx
            test edx , edx
            jz .L2
            
            inc eax
        .L2:
            mov dword[KernelLoaderSectorSize] , eax
            mov edi , dword[StaticAPLoaderStartAddress]
            ; Load AP Loader
            
            mov ah , 0x42
            mov si , DAP
            mov dword[si+2] , 1
            mov ecx , dword[StaticAPLoaderStartAddress]
            mov dword[si+4] , ecx
            mov ecx , dword[KernelLoaderLocation]
            add ecx , dword[KernelLoaderSectorSize]
            sub ecx , 1
            mov dword[si+8] , ecx
            mov dword[si+12] , 0x00
            mov dl , byte[DriveNumber]
            int 0x13

            mov bl , byte[KernelLoaderSectorSize]
            mov ah , 0x42
            mov si , DAP
            mov byte[si+2] , bl
            mov ecx , dword[StaticKernelLoaderStartAddress]
            mov dword[si+4] , ecx
            mov ecx , dword[KernelLoaderLocation]
            mov dword[si+8] , ecx
            mov dword[si+12] , 0x00
            mov dl , byte[DriveNumber]
            int 0x13
            
            jmp 0x8400

HandleError:                 ; Handling all error
	mov ax , 0xB800
	mov es , ax

	mov byte[es:0x00] , "E"
	mov byte[es:0x02] , "R"
	mov byte[es:0x04] , "R"
	mov byte[es:0x06] , "O"
	mov byte[es:0x08] , "R"
	jmp $

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CompareFileName Function                                                       ;;
;; Description : Compares file name with name of the kernel file.                 ;;
;; If the file name is same, return(Sets AX register value to) 1, else, return 0. ;;
; Argument : File Name                                                            ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

CompareFileName:
    push bp
    mov bp , sp
    pusha       ; push all registers into stack

    mov di , word[bp+4]             ; DI : Argument
    mov si , KernelLoaderFileName   ; SI : Kernel file name

    .Loop:
        mov al , byte[di]           ; Get one character(respectively)
        mov ah , byte[si]
        
        cmp ah , 0x00               ; If AH(One character of Kernel File Name) is 0, it means it is 
        je .Equal                   ; the end of the string(Equal)

        add di , 1                  ; Go to next character(respectively)
        add si , 1

        cmp al , ah                 ; Compare AL(Argument) , AH(Kernel File Name)
        je .Loop                    ; Loops only when two of them are true - so that when we reach
                                    ; the end of the string, we know that two strings are same
    
    .NotEqual:                      ; Not Equal
        popa                        ; Pop all registers
        pop bp
        xor ax , ax                 ; Return 0 (AX = 0x00)
        ret
    .Equal:
        popa
        pop bp
        mov ax , 0x01               ; Return 1 (AX = 0x01)
        ret

times (2046-($-$$)) db 0x00         ; The size of the bootloader in ISO9660 is 2048 byte (Align to 2048)
dw 0xAA55                           ; Signature of boot loader

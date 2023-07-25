;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; File "BootLoader.asm"                                                       ;;
;; Description : Source of the boot loader of the Main OS                      ;;
;; Loads file KernelLoader.ldr to appointed location(which is location 0x8400) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[BITS 16]

    db 0xEB , 0x3C , 0x90

;;;;;;;;;;; Disk Info ;;;;;;;;;;;
;; Total Sector Count : 32768  ;;
;; Total Disk Size    : 16MB   ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

OEMID:                      db "POTATOOS"
BytesPerSector:             dw 512
SectorsPerCluster:          db 4
ReservedSectorCount:        dw 4
NumberOfFAT:                db 2
RootDirectoryEntryCount:    dw 512
TotalSector16:              dw 65534
MediaType:                  db 0xF8
FATSize16:                  dw 64
SectorsPerTrack:            dw 63 ; Default CHS
NumberOfHeads:              dw 16
HiddenSectors:              dd 0x00
TotalSectors32:             dd 0x00
INT0x13DriveNumber:         db 0x80
Reserved:                   db 0x00
BootSignature:              db 0x27
SerialNumber:               dd 0x31415926
VolumeLabel:                db "NO NAME    "
FileSystemType:             db "FAT16   "

Start:
    mov ax , 0x00
    mov ds , ax
    mov ecx , dword[0x7C00-4] ; Location of PartitionStartAddress

    mov ax , 0x07C0
    mov ds , ax
    mov dword[PartitionStartAddress] , ecx

    mov sp , 0x9FF8
    mov bp , 0x9FF8
    
    mov byte[DriveNumber] , dl

    xor eax , eax
    xor ebx , ebx
    xor ecx , ecx
    mov ax , word[ReservedSectorCount]

    mov bx , word[FATSize16]
    mov cl , byte[NumberOfFAT]
    imul ebx , ecx

    add ebx , eax
    ; EBX : Root Directory Location
    mov dword[RootDirectoryLocation] , ebx
    
    mov ax , word[RootDirectoryEntryCount]
    imul eax , 32 ; One SFN entry size
    xor edx , edx
    movzx ecx , word[BytesPerSector]
    idiv ecx

    mov word[RootDirectorySize] , ax

    ReadRootDirectory:
        mov ah , 0x42   ; Extended Read Sector From Drive : 
                        ; SI : DAP Address
        mov si , DAP
        
        movzx ecx , word[RootDirectorySize]
        mov byte[si+2] , cl
        mov ecx , dword[RootDirectoryLocation]

        mov dword[si+4] , 0x500   ; Physical memory that the data is going to be copied
        add ecx , dword[PartitionStartAddress] 

        mov dword[si+8] , ecx   ; Root directory address
        mov dword[si+12] , 0x00 ; Empty(Originally start of the sector field was 8 byte long, but
                                ; in real mode, we can't do that, so we just leave it empty)
        mov dl , byte[DriveNumber]
        int 0x13 ; Saves Root Directory to Address 0x500
    
    ; Change data segment to zero, since we got to read address 0x500
    mov ax , 0x00
    mov ds , ax

    xor ecx , ecx
    mov di , 0x500 ; RootDirectory buffer
    mov cx , word[RootDirectoryEntryCount+0x7C00]
    
    ; Find Kernel Loader from the root directory
    ; Compare file name of each entry until finding the kernel loader
    FindKernelLoader:
        push di
        push KernelLoaderName+0x7C00
        push 11
        call memcmp
        add sp , 6

        test ax , ax
        jz FoundKernel

        add di , 32

        loop FindKernelLoader
    
    KernelNotFound:
        mov eax , 0xEE00EE00

        jmp $

    FoundKernel:
        ; Starting Cluster High
        movzx ebx , word[di+(8+3+1+(2*4))]
        shl ebx , 16
        ; Starting Cluster Low
        or bx , word[di+(8+3+1+(2*7))]
        
        ; File size
        mov ecx , dword[di+(32-4)]
        
        ; Reset the data segment
        mov ax , 0x07C0
        mov ds , ax

        ; Convert Cluster number to Sector number 
        sub ebx , 2
        movzx eax , byte[SectorsPerCluster]
        imul ebx , eax
        add ebx , dword[RootDirectoryLocation]
        add bx , word[RootDirectorySize]
        ; Save converted sector number to variable
        mov dword[KernelLoaderLocation] , ebx

        ; Convert byte to sector
        xor edx , edx
        mov eax , ecx
        movzx ebx , word[BytesPerSector]
        idiv ebx
        mov dword[KernelLoaderSectorSize] , eax
    
    LoadAPLoader:

        mov ah , 0x42   ; Extended Read Sector From Drive : 
                        ; SI : DAP Address
        mov si , DAP
        
        mov byte[si+2] , 1
        ; AP Loader is at the rear of the loader file

        mov ecx , dword[KernelLoaderLocation]
        add ecx , dword[KernelLoaderSectorSize]
        sub ecx , 4

        mov dword[si+4] , 0x8000   ; Physical memory that the data is going to be copied
        add ecx , dword[PartitionStartAddress]
        mov dword[si+8] , ecx
        
        mov dword[si+12] , 0x00
        
        mov ebx , edx
        mov dl , byte[DriveNumber]
        int 0x13 ; Saves Root Directory to Address 0x500
    
    LoadKernelLoader:
        mov ah , 0x42   ; Extended Read Sector From Drive : 
                        ; SI : DAP Address
        mov ebx , dword[KernelLoaderSectorSize]
        mov byte[si+2] , bl
        
        ; AP Loader is at the rear of the loader file
        
        mov ecx , dword[KernelLoaderLocation]

        mov dword[si+4] , 0x8400 ; Physical memory that the data is going to be copied
        add ecx , dword[PartitionStartAddress]
        mov dword[si+8] , ecx
        
        mov dword[si+12] , 0x00
        
        mov ebx , edx
        mov dl , byte[DriveNumber]
        int 0x13 ; Saves Root Directory to Address 0x500
        
        jmp 0x00:0x8400

memcmp:
    push bp
    mov bp , sp
    pusha

    mov cx , word[bp+4]
    mov di , word[bp+6]
    mov si , word[bp+8]

    .L1:
        mov bl , byte[si]
        cmp byte[di] , bl
        jne .L2

        add di , 1
        add si , 1

        loop .L1
    
    popa
    pop bp
    mov ax , 0x00
    ret

    .L2:
        popa
        pop bp
        mov ax , 0x01
        ret

RootDirectoryLocation: dd 0x00
RootDirectorySize: dw 0x00
KernelLoaderLocation: dd 0x00
KernelLoaderSectorSize: dd 0x00

DAP:                ; DAP Area(Disk Address Packet)
    db 0x10         ; Size of the DAP
    db 0x00
    dw 0x00
    dd 0x00
    dd 0x00
    dd 0x00

DriveNumber db 0x00
KernelLoaderName: db "KERNEL~1LDR" , 0x00
PartitionStartAddress: dd 0x00

times (510-($-$$)) db 0x00
dw 0xAA55

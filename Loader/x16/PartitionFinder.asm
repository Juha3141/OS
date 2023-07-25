; This bootloader is a bootloader that is stored in SECTOR 0 IN THE PHYSICAL DISK, 
; NOT THE LOGICAL DISK.
; This bootloader will load the bootable partition to the bootloader, and give its
; partition address via predetermined memory address.

[BITS 16]

    jmp START

DriveNumber: db 0x00
ErrorMessage: db "No bootable partition found." , 0x0D , 0x0A , 0x00

DAP:                ; DAP Area(Disk Address Packet)
    db 0x10         ; Size of the DAP
    db 0x00
    dw 0x00
    dd 0x00
    dd 0x00
    dd 0x00

START:
    mov ax , 0x00
    mov ds , ax
    
    mov si , 0x7C00
    mov di , 0x4000
    mov cx , 512
    rep movsb 
    
    jmp 0x0400:START2

START2:
    mov ax , 0x400
    mov ds , ax

    mov si , 446
    mov cx , 4

    mov byte[DriveNumber] , dl

    .L1:
        cmp byte[si] , 0x80
        je .BOOTABLE

        add si , 16
        loop .L1
    
    mov si , ErrorMessage
    mov ah , 0x0E
    .L3:
        mov al , byte[si]
        test al , al
        jz .PANIC
        inc si

        int 0x10

        jmp .L3
    
    .PANIC:
        jmp $

    .BOOTABLE:
        mov ebx , dword[si+8] ; si+8 : Start of StartingLBA

        mov ah , 0x42   ; Extended Read Sector From Drive : 
                        ; SI : DAP Address
        mov si , DAP
        
        mov byte[si+2] , 1
        mov dword[si+4] , 0x7C00
        mov dword[si+8] , ebx
        
        mov dword[si+12] , 0x00
        
        mov dl , byte[DriveNumber]
        int 0x13

        mov dl , byte[DriveNumber]
        
        mov ax , 0x7B0
        mov es , ax

        mov dword[es:0xFC] , ebx

        jmp 0x07C0:0x00
    
    jmp $

times (510-($-$$)) db 0x00
dw 0xAA55

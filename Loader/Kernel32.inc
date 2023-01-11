;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; File "Kernel32.inc"                        ;;
;; Description : Extensional file of Kernel32 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; PrintString Function                                                 ;;
;; Description : Prints string to text screen (In video memory 0xB8000) ;;
;; Arguments : 1. Offset to print the string                            ;;
;;             2. String(Pointer of the String)                         ;;
;;             3. Color                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

CurrentOffset: db 0x00

PrintString:
    push ebp
    mov ebp , esp
    
    push eax
    push ebx
    push ecx
    push edi
    push esi

    xor ecx , ecx
    mov esi , dword[ebp+12]  ; String
    mov edi , dword[ebp+16]  ; Offset
    imul edi , 0x02
    add edi , 0xB8000
    
    .Loop:
        mov ebx , dword[ebp+8]   ; Color
        mov edi , 0xB8000
        
        mov al , byte[esi]
        cmp al , 0x00
        je .End

        cmp al , 0x0A
        je .ChangeLine
        
        add edi , ecx

        mov byte[edi] , al
        mov byte[edi+1] , bl

        add ecx , 2
        add esi , 1
        
        jmp .Loop
    
    .ChangeLine:
        mov eax , ecx
        mov ebx , 80
        xor edx , edx
        div ebx
        imul eax , 80*2
        mov ecx , eax

        add esi , 1
        add edi , ecx

        jmp .Loop
    
    .End:
        pop esi
        pop edi
        pop ecx
        pop ebx
        pop eax
        pop ebp
        ret

strlen:
    push ebp
    mov ebp , esp
    push edi
    push eax
    push ecx

    mov edi , dword[ebp+8]
    mov dword[ebp-4] , 0x00

    .Loop:
        mov al , byte[edi]
        add edi , 1
        add dword[ebp-4] , 1
        cmp al , 0x00
        jne .Loop
    
    pop eax
    pop edi
    dec dword[ebp-4]
    mov eax , dword[ebp-4]
    ret

memcmp:
    push ebp
    mov ebp , esp

    push edi
    push esi
    push eax
    push ecx

    mov edi , dword[ebp+16]
    mov esi , dword[ebp+12]
    mov ecx , dword[ebp+8]

    .Loop:
        mov al , byte[edi]
        mov ah , byte[esi]

        cmp ah , al
        jne .NotEqual
        
        add edi , 1
        add esi , 1

        loop .Loop
    
    .Equal:
        pop ecx
        pop eax
        pop esi
        pop edi
        pop ebp

        mov eax , 0x01
        ret

    .NotEqual:
        pop ecx
        pop eax
        pop esi
        pop edi
        pop ebp

        mov eax , 0x00
        ret

memcpy:
    push ebp
    mov ebp , esp
    push esi
    push edi
    push eax
    push ecx

    mov edi , dword[ebp+16]   ; Destintion
    mov esi , dword[ebp+12]   ; Source
    mov ecx , dword[ebp+8]    ; Size
    
    .Loop:
        mov al , byte[esi]
        mov byte[edi] , al
        
        add esi , 1
        add edi , 1

        loop .Loop

    pop ecx
    pop eax
    pop edi
    pop esi
    pop ebp
    ret

ProtectModeCR0: dd 0x00
ProtectModeESP: dd 0x00
ProtectModeEBP: dd 0x00

RMA_Successed: db 0x00

RMA_AH: db 0x00
RMA_AL: db 0x00
RMA_BH: db 0x00
RMA_BL: db 0x00
RMA_CH: db 0x00
RMA_CL: db 0x00
RMA_DH: db 0x00
RMA_DL: db 0x00
RMA_SI: dw 0x00
RMA_DI: dw 0x00

DoBIOSInterrupt:
    push ebp
    mov ebp , esp
    pushad

    mov eax , ebp
    mov ebx , esp
    mov dword[ProtectModeEBP] , eax
    mov dword[ProtectModeESP] , ebx

    mov eax , dword[ebp+8]
    mov byte[InterruptNumber] , al

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

    mov ah , byte[RMA_AH]
    mov al , byte[RMA_AL]
    mov bh , byte[RMA_BH]
    mov bl , byte[RMA_BL]
    mov ch , byte[RMA_CH]
    mov cl , byte[RMA_CL]
    mov dh , byte[RMA_DH]
    mov dl , byte[RMA_DL]
    mov si , word[RMA_SI]
    mov di , word[RMA_DI]

    db 0xCD
    InterruptNumber: db 0x00
    
    mov byte[RMA_AH] , ah
    mov byte[RMA_AL] , al
    mov byte[RMA_BH] , bh
    mov byte[RMA_BL] , bl
    mov byte[RMA_CH] , ch
    mov byte[RMA_CL] , cl
    mov byte[RMA_DH] , dh
    mov byte[RMA_DL] , dl
    mov word[RMA_SI] , si
    mov word[RMA_DI] , di

    jc Failed

    mov byte[RMA_Successed] , 0x01

ReadyForProtectMode:
	lgdt [ProtectModeGDTR]

	cli

	mov eax , cr0
    or eax , 0x01
	mov cr0 , eax

    jmp 0x08:ProtectMode

[BITS 32]

ProtectMode:
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

    xor eax , eax
    mov al , byte[RMA_Successed]

    ret 

Failed:
    mov byte[RMA_Successed] , 0x00
    jmp ReadyForProtectMode

RealModeIDTR:
    dw 0x3FF
    dd 0x00

RealModeGDTR:
    dw RealModeGDT_END-RealModeGDT
    dd RealModeGDT

RealModeGDT:
    .NullSegment:
        dw 0x00
        dw 0x00
        db 0x00
        db 0x00
        db 0x00
        db 0x00
    .CodeSegment:
        dw 0xFFFF
        dw 0x00
        db 0x00
        db 0b10011010
        db 0b00001111
        db 0x00
    .DataSegment:
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
	.NullSegment:
        dw 0x00
        dw 0x00
        db 0x00
        db 0x00
        db 0x00
        db 0x00
	
	.CodeSegment:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0b10011010
		db 0b11001111
		db 0x00
	
	.DataSegment:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0b10010010
		db 0b11001111
		db 0x00
	
ProtectModeGDT_END:
    
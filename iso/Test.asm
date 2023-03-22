[BITS 64]

SECTION .text

Main:
	push rbp
	mov rbp , rsp
	
	sub rsp , 20
	
	mov dword[rsp] , 0x6C6C6568
	mov dword[rsp+4] , 0x6F77206F
	mov dword[rsp+8] , 0x00646C72
	mov dword[rsp+12] , 0x00000000
	
	mov rdi , 0xB8000
	mov rcx , 11
	.L1:
		mov al , byte[rsp]
		mov byte[rdi] , al
		add rdi , 2
		add rsp , 1
		loop .L1
	
	mov rsp , rbp
	pop rbp
	ret

[BITS 32]

EXTERN scrollup, print
GLOBAL _start

;affichage d'un message
_start:
	mov eax, msg
	push eax
	call print
	pop eax
	
	mov eax, msg2
	push eax
	call print
	pop eax
	
	mov eax, 2
	push eax
	call scrollup
	pop eax
	
end:
	jmp end

msg db 'Kernel charge !', 10, 0
msg2 db 'Ce sont les premieres paroles de ce kernel :)', 10, 0

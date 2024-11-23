global do_switch
global do_thread

do_switch:
	;recupere l'adresse de *current
	mov esi, [esp]
	pop eax ;depile current
	
	push dword [esi + 1]	;eax
	push dword [esi + 5]	;ecx
	push dword [esi + 9]	;edx
	push dword [esi + 13]	;ebx
	push dword [esi + 21]	;ebp
	push dword [esi + 25]	;esi
	push dword [esi + 29]	;edi
	push dword [esi + 45]	;ds
	push dword [esi + 47]	;es
	push dword [esi + 49]	;fs
	push dword [esi + 51]	;gs

	;enleve le mask du PIC
	mov al, 0x20
	out 0x20, al
	
	;charge table des pages
	mov eax, [esi + 53]
	mov cr3, eax
	
	;charge les registres
	
	pop gs
	pop fs
	pop es
	pop ds
	pop edi
	pop esi
	pop ebp
	pop ebx
	pop edx
	pop ecx
	pop eax

	iret

do_thread:
    iret

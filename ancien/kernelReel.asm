[BITS 16]
[ORG 0x0]

jmp start

%include "UTIL.INC"

start:
;init data seg en 0x100
	mov ax, 0x100
	mov ds, ax
	mov es, ax
;init pile
	mov ax, 0x8000
	mov ss, ax
	mov sp, 0xf000
;affich msg
	mov si, msgKernel
	call afficher
	
	mov si, dmdCaract
	call afficher

dmd:
	
	mov ah, 0x0
	int 0x16 ;récupération d'un caractère
	mov byte [repUse], al ;transfert du caractère
	mov si, repUse ;affichage
	call afficher
	cmp ax, 0x3C00 ;si le caractère = F2
	je aff
	cmp ax, 0x11B ;si le caractère = echap
	je end
	cmp ax, 0xE08
	je erase
	cmp ax, 0x5300
	je erase
	cmp ax, 0x1C0D
	je saut
	jmp dmd

end:
	int 0x19 ;redémarage
	
aff:
	mov si, repDeUser
	call afficher
	jmp dmd
erase:
	mov si, vide
	call afficher
	jmp dmd
saut:
	inc dh ;passage ligne suivante
	xor dl, dl ;colone 0
	


msgKernel: 	db	'Kernel now speak',	13,	10, 	0
dmdCaract:	db	'Entrez un caractere', 	13,	10, 	0
repDeUser:	db	'test',			13,	10,	0
repUse:		db	'a',			0,	10,	0
vide:		db	' ',		0,	10,	0

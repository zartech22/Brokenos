global _start, start
extern kmain

%define MULTIBOOT_HEADER_MAGIC	    0x1BADB002
%define MULTIBOOT_HEADER_FLAGS	    0x00000007
%define MULTIBOOT_HEADER_MODE_TYPE  0x00000000

;%define MULTIBOOT_HEADER_WIDTH	    0x00000480
;%define MULTIBOOT_HEADER_HEIGHT	    0x00000360

%define MULTIBOOT_HEADER_WIDTH	    0x00000780
%define MULTIBOOT_HEADER_HEIGHT	    0x00000438

%define MULTIBOOT_HEADER_DEPTH	    0x00000020
%define CHECKSUM -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

SECTION .multiboot
align 4
multiboot_header:
dd MULTIBOOT_HEADER_MAGIC
dd MULTIBOOT_HEADER_FLAGS
dd CHECKSUM
times 5 dd 0x00000000
dd MULTIBOOT_HEADER_MODE_TYPE
dd MULTIBOOT_HEADER_WIDTH
dd MULTIBOOT_HEADER_HEIGHT
dd MULTIBOOT_HEADER_DEPTH

SECTION .text

_start:
	jmp start

start:
	push ebx
	call kmain

        cli
        hlt

#include <utils/types.h>
#include <utils/lib.h>
#include <memory/mm.h>

#define __GDT__
#include "gdt.h"


void init_gdt_desc(const u32 base, const u32 limite, const u8 acces, const u8 other, gdtdesc *desc)
{
	desc->lim0_15 = (limite & 0xffff);
	desc->base0_15 = (base & 0xffff);
	desc->base16_23 = (base & 0xff0000) >> 16;
	desc->acces = acces;
	desc->lim16_19 = (limite & 0xf0000) >> 16;
	desc->other = (other & 0xf);
	desc->base24_31 = (base & 0x0ff000000) >> 24;
}

void init_gdt()
{
	default_tss.debug_flag = 0x00;
	default_tss.io_map = 0x00;
	default_tss.esp0 = 0x1FFF0;
	default_tss.ss0 = 0x18;
	
	//Kernel
	init_gdt_desc(0x0, 0x0, 0x0, 0x0, &kgdt[0]);
    init_gdt_desc(0x0, 0xFFFFF, 0x9B, 0x0D, &kgdt[1]);	// Code
    init_gdt_desc(0x0, 0xFFFFF, 0x93, 0x0D, &kgdt[2]);	// Data
    init_gdt_desc(0x0, 0x0, 0x97, 0x0D, &kgdt[3]);      // Stack
	
	//User
    init_gdt_desc(0x0, 0xFFFFF, 0xFF, 0x0D, &kgdt[4]);	// Code
    init_gdt_desc(0x0, 0xFFFFF, 0xF3, 0x0D, &kgdt[5]);	// Data
    init_gdt_desc(0x0, 0x0, 0xF7, 0x0D, &kgdt[6]);      // Stack
	
    init_gdt_desc(reinterpret_cast<u32>(&default_tss), 0x67, 0xE9, 0x00, &kgdt[7]);	// TSS
	
	kgdtr.limite = GDTSIZE * 8;
	kgdtr.base = GDTBASE;
	
	memcpy(reinterpret_cast<char *>(kgdtr.base), reinterpret_cast<char *>(kgdt), kgdtr.limite);
	
	asm("lgdtl (kgdtr)");
		
	asm("   movw $0x10, %ax	\n \
            movw %ax, %ds	\n \
            movw %ax, %es	\n \
            movw %ax, %fs	\n \
            movw %ax, %gs	\n \
            ljmp $0x08, $next	\n \
            next:		\n");
		
}

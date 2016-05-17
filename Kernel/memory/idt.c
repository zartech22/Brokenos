#include <utils/types.h>
#include <utils/lib.h>
#include <core/io.h>

#define __IDT__

#include "idt.h"

#ifdef __cplusplus
	extern "C" {
#endif
void _asm_default_int();
void _asm_default_exc();
void _asm_irq_0();
void _asm_irq_1();
void _asm_syscalls();
void _asm_exc_GP();
void _asm_exc_PF();
#ifdef __cplusplus
	}
#endif

void init_idt_desc(u16 select, u32 offset, u16 type, struct idtdesc *desc)
{
	desc->offset0_15 = (offset & 0xffff);
	desc->select = select;
	desc->type = type;
	desc->offset16_31 = (offset & 0xffff0000) >> 16;
}

void init_idt()
{
	//init desc sys par def.
    for(u8 i = 0; i < IDTSIZE; i++)
		init_idt_desc(0x08, (u32) _asm_default_int, INTGATE, &kidt[i]);
	
	//exceptions
	init_idt_desc(0x08, (u32) _asm_exc_GP, INTGATE, &kidt[13]);
	init_idt_desc(0x08, (u32) _asm_exc_PF, INTGATE, &kidt[14]);
		
	//interrupt
	init_idt_desc(0x08, (u32) _asm_irq_0, INTGATE, &kidt[32]); //horloge
	init_idt_desc(0x08, (u32) _asm_irq_1, INTGATE, &kidt[33]); //clavier
	init_idt_desc(0x08, (u32) _asm_syscalls, TRAPGATE, &kidt[48]); //appels systèmes
	
	//init struct pr IDTR
	kidtr.limite = IDTSIZE * 8;
	kidtr.base = IDTBASE;
	
	memcpy((char*)kidtr.base, (char*)kidt, kidtr.limite);
	
	//charement du registre idt
	asm("lidtl (kidtr)");
}

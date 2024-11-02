#pragma once

#include "mm.h"

#define INTGATE 0x8E00
#define TRAPGATE 0xEF00

//desc de seg
struct idtdesc
{
	u16 offset0_15;
	u16 select;
	u16 type;
	u16 offset16_31;
} __attribute__ ((packed));

struct idtr
{
	u16 limite;
	u32 base;
} __attribute__ ((packed));

#ifdef __IDT__
	idtr kidtr;
	idtdesc kidt[IDTSIZE];
#else
	extern idtr kidtr;
	extern idtdesc kidt[];
#endif

void init_idt_desc(u16, u32, u16, idtdesc *desc);
void init_idt();

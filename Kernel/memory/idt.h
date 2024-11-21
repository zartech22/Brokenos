#pragma once

#include "mm.h"

#define INTGATE 0x8E00
#define TRAPGATE 0xEF00

//desc de seg
struct idtdesc
{
	uint16_t offset0_15;
	uint16_t select;
	uint16_t type;
	uint16_t offset16_31;
} __attribute__ ((packed));

struct idtr
{
	uint16_t limite;
	uint32_t base;
} __attribute__ ((packed));

#ifdef __IDT__
	idtr kidtr;
	idtdesc kidt[IDTSIZE];
#else
	extern idtr kidtr;
	extern idtdesc kidt[];
#endif

void init_idt_desc(uint16_t, uint32_t, uint16_t, idtdesc *desc);
void init_idt();

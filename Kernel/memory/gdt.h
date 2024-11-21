#pragma once

#include "mm.h"

//desc de segment
struct gdtdesc
{
	uint16_t lim0_15;
	uint16_t base0_15;
	uint8_t base16_23;
	uint8_t acces;
	uint8_t lim16_19 : 4;
	uint8_t other : 4;
	uint8_t base24_31;
} __attribute__ ((packed));

//registre GDTR
struct gdtr
{
	uint16_t limite;
	uint32_t base;
} __attribute__ ((packed));

struct tss
{
	uint16_t previous_task, __previous_task_unused;
	uint32_t esp0;
	uint16_t ss0, __ss0_unused;
	uint32_t esp1;
	uint16_t ss1, __ss1_unused;
	uint32_t esp2;
	uint16_t ss2, __ss2_unused;
	uint32_t cr3;
	uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	uint16_t es, __es_unused;
	uint16_t cs, __cs_unused;
	uint16_t ss, __ss_unused;
	uint16_t ds, __ds_unused;
	uint16_t fs, __fs_unused;
	uint16_t gs, __gs_unused;
	uint16_t ldt_selector, __ldt_sel_unused;
	uint16_t debug_flag, io_map;
} __attribute__ ((packed));

void init_gdt_desc(uint32_t, uint32_t, uint8_t, uint8_t, gdtdesc*);
void init_gdt();

#ifdef __GDT__
	gdtdesc kgdt[GDTSIZE];
	gdtr kgdtr;
	tss default_tss;
#else
	extern gdtdesc kgdt[];
	extern gdtr kgdtr;
	extern tss default_tss;
#endif

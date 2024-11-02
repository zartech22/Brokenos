#pragma once

#include "mm.h"

//desc de segment
struct gdtdesc
{
	u16 lim0_15;
	u16 base0_15;
	u8 base16_23;
	u8 acces;
	u8 lim16_19 : 4;
	u8 other : 4;
	u8 base24_31;
} __attribute__ ((packed));

//registre GDTR
struct gdtr
{
	u16 limite;
	u32 base;
} __attribute__ ((packed));

struct tss
{
	u16 previous_task, __previous_task_unused;
	u32 esp0;
	u16 ss0, __ss0_unused;
	u32 esp1;
	u16 ss1, __ss1_unused;
	u32 esp2;
	u16 ss2, __ss2_unused;
	u32 cr3;
	u32 eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	u16 es, __es_unused;
	u16 cs, __cs_unused;
	u16 ss, __ss_unused;
	u16 ds, __ds_unused;
	u16 fs, __fs_unused;
	u16 gs, __gs_unused;
	u16 ldt_selector, __ldt_sel_unused;
	u16 debug_flag, io_map;
} __attribute__ ((packed));

void init_gdt_desc(u32, u32, u8, u8, gdtdesc*);
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

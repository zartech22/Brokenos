#pragma once

//desactive interruptions
#define cli asm("cli"::)

//reactive
#define sti asm("sti"::)

//ecrit octet sur un port
#define outb(port,value) \
	asm volatile ("outb %%al, %%dx" :: "d" (port), "a" (value));
	
//ecrit un octet sur un port + temporisation
#define outbp(port,value) \
	asm volatile ("outb %%al, %%dx; jmp 1f; 1:" :: "d" (port), "a" (value));

//lit octet sur un port
#define inb(port) ({	\
		uint8_t _v; \
		asm volatile ("inb %%dx, %%al" : "=a" (_v) : "d" (port)); \
		_v;		\
	})

#define outw(port, value) \
		asm volatile("outw %%ax, %%dx" :: "d" (port), "a" (value));

#define inw(port) ({	\
		uint16_t _v;			\
		asm volatile("inw %%dx, %%ax" : "=a" (_v) : "d" (port));	\
		_v;				\
})

#define outl(port, value) \
		asm volatile("outl %%eax, %%dx" :: "d" (port), "a" (value));

#define inl(port) ({	\
		uint32_t _v;			\
		asm volatile("inl %%dx, %%eax" : "=a" (_v) : "d" (port));	\
		_v;				\
})

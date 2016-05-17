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
		unsigned char _v; \
		asm volatile ("inb %%dx, %%al" : "=a" (_v) : "d" (port)); \
		_v;		\
	})

#define outw(port, value) \
		asm volatile("outw %%ax, %%dx" :: "d" (port), "a" (value));

#define inw(port) ({	\
		u16 _v;			\
		asm volatile("inw %%dx, %%ax" : "=a" (_v) : "d" (port));	\
		_v;				\
})

#define outl(port, value) \
		asm volatile("outl %%eax, %%dx" :: "d" (port), "a" (value));

#define inl(port) ({	\
		u32 _v;			\
		asm volatile("inl %%dx, %%eax" : "=a" (_v) : "d" (port));	\
		_v;				\
})

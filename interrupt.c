#include "types.h"
#include "strLib.h"
#include "kbd.h"
#include "io.h"
#include "gdt.h"
#include "scheduler.h"
#include "interrupt.h"

void love();

void isr_default_int()
{
	//print("An interrupt has been catch !\n");
}

void isr_default_exc()
{
	Screen::getScreen().println("An Exception occurred !");
}

void isr_clock_int()
{
	static int tic = 0;
	static int sec = 0;
	
	tic++;
	
	if(tic % 2000 == 0)
	{
		sec++;
		tic = 0;
		
		if(Screen::getScreen().isLoading())
			Screen::getScreen().showTic();
		/*else
			putcar('.');*/
	}
	
	schedule();
}

void isr_GP_exc()
{
	Screen::getScreen().print("GP Fault\n");
	
	asm("hlt");
}

void isr_PF_exc()
{
	u32 faulting_addr;
	u32 eip;
	
	asm("	movl 60(%%ebp), %%eax;	\
			mov %%eax, %0;			\
			mov %%cr2, %%eax;		\
			mov %%eax, %1": "=m"(eip), "=m"(faulting_addr): );
	Screen::getScreen().println("#PF");
	Screen::getScreen().dump((uchar*) &faulting_addr, 4);
	Screen::getScreen().println("");
	Screen::getScreen().dump((uchar*) &eip, 4);
	
	asm("hlt");
}

void isr_kbd_int()
{
	uchar i;
	static int lshift_enable;
	static int rshift_enable;
	static int alt_enable;
	static int ctrl_enable;
	
	do
	{
		i = inb(0x64);
	}while((i & 0x01) == 0);
	
	i = inb(0x60);
	i--;
	
	if(i < 0x80)
	{
		switch(i)
		{
			case 0x29:
				lshift_enable = 1;
				break;
			case 0x35:
				rshift_enable = 1;
				break;
			case 0x1C:
				ctrl_enable = 1;
				break;
			case 0x37:
				alt_enable = 1;
				break;
			default:
				Screen::getScreen().putcar(kbdmap[i * 4 + (lshift_enable || rshift_enable)]);
		}
	}
	else
	{
		i -= 0x80;
		
		switch(i)
		{
			case 0x29:
				lshift_enable = 0;
				break;
			case 0x35:
				rshift_enable = 0;
				break;
			case 0x1C:
				ctrl_enable = 0;
				break;
			case 0x37:
				alt_enable = 0;
				break;
		}
	}
	
	if(kbdmap[i * 4] == 'l')
		Screen::getScreen().showLoadScreen();
}

void love()
{
	Screen::getScreen().print("Coucou ! Je suis le kernel et je t'aime fort !!\n");
	Screen::getScreen().show_cursor();
}

void do_syscall(int sys_num)
{
	if(sys_num == 1)
	{
		char *u_str;
	
		asm("mov %%ebx, %0" : "=m"(u_str) :);
		for(int i = 0; i < 10000; i++); //temporisation
		cli;
		Screen::getScreen().print(u_str);
		sti;
	}
	else
		Screen::getScreen().printError("Unknown syscall !");
	return;
}

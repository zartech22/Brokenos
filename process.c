#include "strLib.h"
#include "lib.h"
#include "mm.h"
#include "io.h"
#include "kmalloc.h"
#include "types.h"

#define __PLIST__
#include "process.h"

int load_task(char *fn, u32 code_size)
{
	struct page_directory *pd;
	struct page_list *pglist;
	struct page *kstack;
	
	char *v_addr;
	char *p_addr;
	char *ustack;
	
	int pid;
	
	//Calcul du PID du new process. On assume jamais arrive au max
	//FIXME: reutiliser slots libres
	pid = 1;
	while(p_list[pid].state != 0 && pid++ < MAXPID);
	
	if(p_list[pid].state != 0)
	{
		Screen::getScreen().printError("not enough slot for processes");
		return 0;
	}
	
	//Creer un rep de pages
	pd = pd_create();
	
	//On change d'espace d'adressage pour passer sur le nouveau rep
	// de pages. Permet de mettre a jour facilement en utilisant la fct
	// pd_add_page()
	asm("mov %0, %%eax; mov %%eax, %%cr3" :: "m"(pd->base->p_addr));
	
	//Alloc de pages pour y copier la tache
	pglist = (struct page_list*) kmalloc(sizeof(struct page_list));
	pglist->page = 0;
	pglist->next = 0;
	pglist->prev = 0;
	
	int i = 0;
	
	while(i < code_size)
	{
		//Creation de l'espace d'adressage a partir de l'adresse 0x40000
		p_addr = get_page_frame();
		v_addr = (char*) (USER_OFFSET + i);
		pd_add_page(v_addr, p_addr, PG_USER, pd);
		
		//Maj de la liste de page utilisee
		pglist->page = (struct page*) kmalloc(sizeof(struct page));
		pglist->page->p_addr = p_addr;
		pglist->page->v_addr = v_addr;
		
		pglist->next = (struct page_list*) kmalloc(sizeof(struct page_list));
		pglist->next->page = 0;
		pglist->next->next = 0;
		pglist->next->prev = pglist;
		
		pglist = pglist->next;
		
		i += PAGESIZE;
	};
	
	//Copie du code
	memcpy((char*) USER_OFFSET, fn, code_size);
	
	//Cree la pile user
	ustack = get_page_frame();
	pd_add_page((char*) USER_STACK, ustack, PG_USER, pd);
	
	//Cree la pile noyau
	kstack = get_page_from_heap();
	
	n_proc++;
	
	p_list[pid].pid = pid;

	/* Initialisation des registres */
	p_list[n_proc].regs.ss = 0x33;
	p_list[n_proc].regs.esp = USER_STACK + PAGESIZE - 16;
	p_list[n_proc].regs.eflags = 0x0;
	p_list[n_proc].regs.cs = 0x23;
	p_list[n_proc].regs.eip = 0x40000000;
	p_list[n_proc].regs.ds = 0x2B;
	p_list[n_proc].regs.es = 0x2B;
	p_list[n_proc].regs.fs = 0x2B;
	p_list[n_proc].regs.gs = 0x2B;
	p_list[n_proc].regs.cr3 = (u32) pd->base->p_addr;
	
	p_list[n_proc].kstack.ss0 = 0x18;
	p_list[n_proc].kstack.esp0 = (u32) kstack->v_addr + PAGESIZE - 16;
	
	p_list[n_proc].regs.eax = 0;
    p_list[n_proc].regs.ecx = 0;
    p_list[n_proc].regs.edx = 0;
    p_list[n_proc].regs.ebx = 0;

    p_list[n_proc].regs.ebp = 0;
    p_list[n_proc].regs.esi = 0;
    p_list[n_proc].regs.edi = 0;
    
    p_list[pid].pd = pd;
    p_list[pid].pglist = pglist;
    
    p_list[pid].state = 1;
    
    asm("mov %0, %%eax; mov %%eax, %%cr3" :: "m"(current->regs.cr3));

	return pid;
	
}

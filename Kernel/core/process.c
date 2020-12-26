#include <video/Screen.h>
#include <utils/lib.h>
#include <memory/mm.h>
#include <core/io.h>
#include <memory/kmalloc.h>
#include <utils/types.h>
#include <disk/FileSystems/FileSystem.h>
#include <utils/elf.h>

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

int load_task(const char *filename)
{
    if(!(&FileSystem::getFsList()) || FileSystem::getFsList().empty())
    {
        sScreen.printError("Error loading file \"%s\". No filesystem available", filename);
        return 0;
    }

    struct page_directory *pd;
    struct page_list *pglist;
    struct page *kstack;

    char *file;
    char *ustack;
    u32 e_entry;

    int pid;

    pid = 1;

    while (p_list[pid].state != 0 && pid++ < MAXPID);

    if (p_list[pid].state != 0)
    {
        sScreen.printk("PANIC: not enough slot for processes\n");
        return 0;
    }

    /* Cree un repertoire de pages */
    pd = pd_create();

    asm("mov %0, %%eax;"
        "mov %%eax, %%cr3" :: "m"(pd->base->p_addr));

    file = FileSystem::getFsList().at(0)->readFile(filename);

    pglist = (struct page_list*)kmalloc(sizeof(struct page_list));
    pglist->page = 0;
    pglist->next = 0;
    pglist->prev = 0;

    e_entry = (u32) loadElf(file, pd, pglist);

    delete file;

    if(e_entry == 0)
    {
        asm("mov %0, %%eax;"
            "mov %%eax, %%cr3" :: "m"(current->regs.cr3));

        pd_destroy(pd);
        return 0;
    }

    /* Cree la pile utilisateur */
        ustack = get_page_frame();
        pd_add_page((char *) USER_STACK, ustack, PG_USER, pd);

        /* Cree la pile noyau */
        kstack = get_page_from_heap();

        n_proc++;

        p_list[pid].pid = pid;

        /* Initialise les registres */
        p_list[pid].regs.ss = 0x33;
        p_list[pid].regs.esp = USER_STACK + PAGESIZE - 16;
        p_list[pid].regs.eflags = 0x0;
        p_list[pid].regs.cs = 0x23;
        p_list[pid].regs.eip = e_entry;
        p_list[pid].regs.ds = 0x2B;
        p_list[pid].regs.es = 0x2B;
        p_list[pid].regs.fs = 0x2B;
        p_list[pid].regs.gs = 0x2B;
        p_list[pid].regs.cr3 = (u32) pd->base->p_addr;

        p_list[pid].kstack.ss0 = 0x18;
        p_list[pid].kstack.esp0 = (u32) kstack->v_addr + PAGESIZE - 16;

        p_list[pid].regs.eax = 0;
        p_list[pid].regs.ecx = 0;
        p_list[pid].regs.edx = 0;
        p_list[pid].regs.ebx = 0;

        p_list[pid].regs.ebp = 0;
        p_list[pid].regs.esi = 0;
        p_list[pid].regs.edi = 0;

        p_list[pid].pd = pd;
        p_list[pid].pglist = pglist;

        p_list[pid].state = 1;

        asm("mov %0, %%eax;"
            "mov %%eax, %%cr3":: "m"(current->regs.cr3));

        return pid;
}

void test(u32 test)
{
    //Screen::getScreen().printDebug("Salut !");
    //Screen::getScreen().printError("Test ! %p", function);
    //function();

    asm("nop;"
        "nop;");

    volatile register int a = test;


    asm("nop;"
        "nop;");

    int d = 5;

    Screen::getScreen().printDebug("Test = %x. Youpie !", test);

    for(;;)
        asm("hlt");
}

void createThread(void *fn)
{
//    struct page_directory *pd;
//    struct page_list *pglist;
//    struct page *kstack;

//    char *v_addr;
//    char *p_addr;
//    char *ustack;

//    int pid;

//    //Calcul du PID du new process. On assume jamais arrive au max
//    //FIXME: reutiliser slots libres
//    pid = 1;
//    while(p_list[pid].state != 0 && pid++ < MAXPID);

//    if(p_list[pid].state != 0)
//    {
//        Screen::getScreen().printError("not enough slot for processes");
//        return;
//    }

//    //Cree la pile user

//    //Cree la pile noyau
//    kstack = get_page_from_heap();

//    n_proc++;

//    p_list[pid].pid = pid;

//    /* Initialisation des registres */
//    p_list[n_proc].regs.ss = 0x18;
//    p_list[n_proc].regs.esp = (u32)kstack->v_addr + PAGESIZE - 16;
//    p_list[n_proc].regs.eflags = 0x0;
//    p_list[n_proc].regs.cs = 0x08;
//    p_list[n_proc].regs.eip = (u32)&test;
//    p_list[n_proc].regs.ds = 0x10;
//    p_list[n_proc].regs.es = 0x10;
//    p_list[n_proc].regs.fs = 0x10;
//    p_list[n_proc].regs.gs = 0x10;
//    p_list[n_proc].regs.cr3 = 0;

//    p_list[n_proc].kstack.ss0 = 0x18;
//    p_list[n_proc].kstack.esp0 = (u32) kstack->v_addr + PAGESIZE - 16;

//    p_list[n_proc].regs.eax = 0;
//    p_list[n_proc].regs.ecx = 0;
//    p_list[n_proc].regs.edx = 0;
//    p_list[n_proc].regs.ebx = 0;

//    p_list[n_proc].regs.ebp = 0;
//    p_list[n_proc].regs.esi = 0;
//    p_list[n_proc].regs.edi = 0;

//    p_list[pid].pd = 0;
//    p_list[pid].pglist = 0;

//    p_list[pid].state = 1;


    cli;

    struct thread *th = new struct thread;

    struct page *kstack = get_page_from_heap();

    th->regs.ss = 0x18;
    th->regs.esp = (u32) kstack->v_addr + PAGESIZE - 16;

    th->regs.eax = 0;
    th->regs.ebx = 0;
    th->regs.ecx = 0;
    th->regs.edx = 0;

    th->regs.ebp = 0;
    th->regs.esi = 0;
    th->regs.edi = 0;

    th->regs.eflags = 0;
    th->regs.cs = 0x8;
    th->regs.ds = 0x10;
    th->regs.es = 0x10;
    th->regs.fs = 0x10;
    th->regs.gs = 0x10;

    th->state = 0;

    void (*testFn)(u32) = &test;

    asm("mov %0, %%eax;" :: "m"(th->regs.esp));

    Screen::getScreen().printError("Test addr de ta CARROTE = %p", createThread);

    asm(
    "mov %0, %%ss;"
        "mov %1, %%esp;"
    ::
    "m"(th->regs.ss), "m"(th->regs.esp));

    asm("pushl $0xDEADBEEF;"
        "pushl $0x0;"
        "pushfl;"
        "orl $0x200, (%%esp);"
        "andl $0xFFFFBFFF, (%%esp);"
        "push %%cs;"
        "push %0;"
        "iret;"
        ::
        "m"(testFn));
}

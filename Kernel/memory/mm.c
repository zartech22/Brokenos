#include <utils/types.h>
#include <video/Screen.h>
#include <memory/kmalloc.h>

#define __MM__
#include "mm.h"

char* get_page_frame()
{
	int page = -1;
	
    for(unsigned int byte = 0; byte < RAM_MAXPAGE / 8; byte++)
		if(mem_bitmap[byte] != 0xFF)
            for(u8 bit = 0; bit < 8; bit++)
				if(!(mem_bitmap[byte] & (1 << bit)))
				{
					page = 8 * byte + bit;
					set_page_frame_used(page);
					return reinterpret_cast<char *>(page * PAGESIZE);
				}
	
	return reinterpret_cast<char *>(page);
}

page* get_page_from_heap()
{
	page *pg;
	char *v_addr, *p_addr;
	
	//Prend page phys libre
	p_addr = get_page_frame();
	
	if(p_addr < static_cast<char *>(nullptr))
	{
        Screen::getScreen().printError("[%()] : no page frame available. STOP", __FUNCTION__);
		asm("hlt");
	}
	
	//Verifie si il y a une page virtuelle libre
	if(free_vm->vm_end == free_vm->vm_start)
	{
        Screen::getScreen().printError("[%s()] : no more memory in page heap. STOP", __FUNCTION__);
		asm("hlt");
	}
	
	//Prend une page virtuelle libre
	v_addr = free_vm->vm_start;
	
	if(free_vm->vm_end - free_vm->vm_start == PAGESIZE)
	{
		if(free_vm->next)
		{
			vm_area *p;
			p = free_vm;
			free_vm = free_vm->next;
			free_vm->prev = nullptr;
			kfree(p);
		}
	}
	else
		free_vm->vm_start += PAGESIZE;
	
	//Maj de l'espace d'adressage noyau
    pd0_add_page(v_addr, p_addr, 0);
	
	//Renvoie la page
	pg = static_cast<page *>(kmalloc(sizeof(page)));
	pg->v_addr = v_addr;
	pg->p_addr = p_addr;
	
	return pg;
}

page* get_page_from_heap(char *p_addr, const char *end)
{
	page *pg;
	vm_area *p;
    char *v_addr;
    char * begin_p_addr = p_addr;
    char *begin_v_addr;

    do
    {
        //Prend page phys libre
        set_page_frame_used(PAGE(p_addr));

        //Verifie si il y a une page virtuelle libre
        if(free_vm->vm_end == free_vm->vm_start)
        {
            //Screen::getScreen().printError("[%s()] : no more memory in page heap. STOP", __FUNCTION__);
            asm("hlt");
        }

        //Prend une page virtuelle libre
        v_addr = free_vm->vm_start;

        if(free_vm->vm_end - free_vm->vm_start == PAGESIZE)
        {
            if(free_vm->next)
            {
                p = free_vm;
                free_vm = free_vm->next;
                free_vm->prev = nullptr;
                kfree(p);
            }
        }
        else
            free_vm->vm_start += PAGESIZE;

        //Maj de l'espace d'adressage noyau
        pd0_add_page(v_addr, p_addr, 0);

        if(p_addr == begin_p_addr)
            begin_v_addr = v_addr;

    }while((p_addr += PAGESIZE) < end);


    pg = new page;
    pg->p_addr = begin_p_addr;
    pg->v_addr = begin_v_addr;

    return pg;
}

int release_page_from_heap(char *v_addr)
{
	vm_area *p;
	char *p_addr;
	
	//Retourne la page frame associee a v_addr et la libere
	p_addr = get_p_addr(v_addr);
	
	if(p_addr)
		release_page_frame(p_addr);
	else
	{
		Screen::getScreen().printInfo("release_page_from_heap : no page frame associated with v_addr");
		return 1;
	}
	
	//Maj rep de pages
	pd_remove_page(v_addr);
	
	//Maj liste addr virtuelles libres
	p = free_vm;
	
	while(p->vm_start < v_addr && p->next)
		p = p->next;
	
	if(v_addr + PAGESIZE == p->vm_start)
	{
		p->vm_start = v_addr;
		
		if(p->prev && p->prev->vm_end == p->vm_start)
		{
			vm_area *to_del;
			to_del = p->prev;
			p->vm_start = p->prev->vm_start;
			p->prev = p->prev->prev;
			
			if(p->prev)
				p->prev->next = p;
			
			kfree(to_del);
		}
	}
	else if(p->prev && p->prev->vm_end == v_addr)
		p->prev->vm_end += PAGESIZE;
	else if(v_addr + PAGESIZE < p->vm_start)
	{
		vm_area *new_vm_area;
		new_vm_area = static_cast<vm_area *>(kmalloc(sizeof(vm_area)));
		new_vm_area->vm_start = v_addr;
		new_vm_area->vm_end = v_addr + PAGESIZE;
		new_vm_area->prev = p->prev;
		new_vm_area->next = p;
		p->prev = new_vm_area;
		
		if(new_vm_area->prev)
			new_vm_area->prev->next = new_vm_area;
	}
	else
	{
        Screen::getScreen().printError("[%s()] : corrupted linked list. STOP", __FUNCTION__);
		asm("hlt");
	}
	
	return 0;
}

void init_mm(u32 high_mem)
{
	int pg_limit;

	pg_limit = (high_mem * 1024) / PAGESIZE;
	
	//init bitmap
    for(unsigned int pg = 0; pg < pg_limit / 8; pg++)
		mem_bitmap[pg] = 0;
	
    for(unsigned int pg = pg_limit / 8; pg < RAM_MAXPAGE / 8; pg++)
		mem_bitmap[pg] = 0xFF;
		
    for(unsigned int pg = PAGE(0x0); pg < PAGE(reinterpret_cast<u32>(pg1_end)); pg++)
		set_page_frame_used(pg);
	
	//Init rep de pages
	pd0[0] = reinterpret_cast<u32>(pg0) | (PG_PRESENT | PG_WRITE | PG_4MB);
	pd0[1] = reinterpret_cast<u32>(pg1) | (PG_PRESENT | PG_WRITE | PG_4MB);
	
	for(unsigned long i = 2; i < 1023; i++)
		pd0[i] = reinterpret_cast<u32>(pg1) + PAGESIZE * i | (PG_PRESENT | PG_WRITE);
	
	//Astuce acces pd
	pd0[1023] = reinterpret_cast<u32>(pd0) | (PG_PRESENT | PG_WRITE);
	
	//Mode pagination
	asm("	mov %0, %%eax 		\n\
			mov %%eax, %%cr3	\n\
			mov %%cr4, %%eax	\n\
			or %2, %%eax		\n\
			mov %%eax, %%cr4	\n\
			mov %%cr0, %%eax	\n\
			or %1, %%eax		\n\
			mov %%eax, %%cr0"::"m"(pd0), "i"(PAGING_FLAG), "i"(PSE_FLAG));
	
	//Init du heap noyau utilise par kmalloc
	kern_heap = reinterpret_cast<char *>(KERN_HEAP);
	ksbrk(1);
	
	//Init de la liste d'adresse virtuelles libre
	free_vm = static_cast<vm_area *>(kmalloc(sizeof(vm_area)));
	free_vm->vm_start = reinterpret_cast<char *>(KERN_PG_HEAP);
	free_vm->vm_end = reinterpret_cast<char *>(KERN_PG_HEAP_LIM);
	free_vm->next = nullptr;
	free_vm->prev = nullptr;
}

//creer un rep de page pour une tache
page_directory* pd_create()
{
	//prend et init une page pour le page dir
	auto *pd = static_cast<struct page_directory *>(kmalloc(sizeof(struct page_directory)));
	pd->base = get_page_from_heap();
	
	//Espace kernel. Les v_addr < USER_OFFSET sont adresse par table
	//page du noyau
	auto pdir = reinterpret_cast<u32 *>(pd->base->v_addr);
	for(int i = 0; i < 256; i++)
		pdir[i] = pd0[i];
	
	//Espace user
	for(int i = 256; i < 1023; i++)
		pdir[i] = 0;
	
	//Astuce acces
	pdir[1023] = reinterpret_cast<u32>(pd->base->p_addr) | (PG_PRESENT | PG_WRITE);
	
	//Maj tables de page espace user
	pd->pt = nullptr;
		
	return pd;
}

int pd_destroy(const page_directory *pd)
{
	//Libere la page correspondant au rep
	release_page_from_heap(pd->base->v_addr);
	
	//Libere les pages correspondant aux tables
	page_list *pgh = pd->pt;
	
	while(pgh)
	{
		release_page_from_heap(pgh->page->v_addr);
		page_list *oldpgh = pgh;
		pgh = pgh->next;
		kfree(oldpgh);
	}
	
	kfree(pd);
	
	return 1;
}

int pd0_add_page(const char *v_addr, char *p_addr, int flags)
{
	u32 *pde;
	u32 *pte;
	
	if(v_addr > reinterpret_cast<char *>(USER_OFFSET))
	{
		Screen::getScreen().printError("pd0_add_page() : vaddr not in kernel space");
		return 0;
	}
	
	//On verifie que la table de page est bien presente
	pde = reinterpret_cast<u32 *>(0xFFFFF000 | ((reinterpret_cast<u32>(v_addr) & 0xFFC00000) >> 20));
	
	if((*pde & PG_PRESENT) == 0)
	{
		Screen::getScreen().printError("pd0_add_page() : kernel page table not found for vaddr. STOP");
		asm("hlt");
	}
	
	//Modification de l'entree dans la table de page
	pte = reinterpret_cast<u32 *>(0xFFC00000 | ((reinterpret_cast<u32>(v_addr) & 0xFFFFF000) >> 10));
	*pte = reinterpret_cast<u32>(p_addr) | (PG_PRESENT | PG_WRITE | flags);
	
	return 0;
}

/* 
 * Met a jour le rep. de pages courant
 * input:
 * 	v_addr : adresse lineaire de la page 
 * 	p_addr : adresse physique de la page allouee 
 * 	pd     : structure qui doit etre mise a jour avec les pages allouees
 */
int pd_add_page(char *v_addr, char *p_addr, const int flags, page_directory *pd)
{
	/* adresse virtuelle de l'entree du repertoire de pages */
	/* adresse virtuelle de l'entree de la table de pages */

	//// printk("DEBUG: pd_add_page(%p, %p, %d)\n", v_addr, p_addr, flags); /* DEBUG */

	/*
	 * La derniere entree du PageDir pointe sur lui-meme.
	 * Les adresses commencant par 0xFFC00000 utilisent cette entree et il
	 * s'ensuite que :
	 * - les 10 bits en 0x003FF000 sont un index dans le PageDir et designent une
	 *   PageTable. Les 12 derniers bits permettent de modifier une entree du PageTable
	 * - l'adresse 0xFFFFF000 designe le PageDir lui-meme
	 */
	auto pde = reinterpret_cast<u32 *>(0xFFFFF000 | ((reinterpret_cast<u32>(v_addr) & 0xFFC00000) >> 20));

	/* 
	 * On cree la table de pages correspondante si elle n'est pas presente
	 */
	if ((*pde & PG_PRESENT) == 0) {
		/* 
		 * Allocation d'une page pour y mettre la table. 
		 */
		page *newpg = get_page_from_heap();

		/* On initialise la nouvelle table de pages */
		const auto pt = reinterpret_cast<u32 *>(newpg->v_addr);
        for (u16 i = 1; i < 1024; i++)
			pt[i] = 0;

		/* On ajoute l'entree correspondante dans le repertoire */
		*pde = reinterpret_cast<u32>(newpg->p_addr) | (PG_PRESENT | PG_WRITE | flags);

		/* On rajoute la nouvelle page dans la structure  passee en parametre */
		if (pd) {
			if (pd->pt) {
				auto *pglist = static_cast<page_list *>(kmalloc(sizeof(page_list)));
				pglist->page = newpg;
				pglist->next = pd->pt;
				pglist->prev = nullptr;
				pd->pt->prev = pglist;
				pd->pt = pglist;
			} else {
				pd->pt = static_cast<page_list *>(kmalloc(sizeof(page_list)));
				pd->pt->page = newpg;
				pd->pt->next = nullptr;
				pd->pt->prev = nullptr;
			}
		}

	}

	const auto pte = reinterpret_cast<u32 *>(0xFFC00000 | ((reinterpret_cast<u32>(v_addr) & 0xFFFFF000) >> 10));
	*pte = reinterpret_cast<u32>(p_addr) | (PG_PRESENT | PG_WRITE | flags);

	return 0;
}

int pd_remove_page(char *v_addr)
{
	if (get_p_addr(v_addr)) {
		u32 *pte;
		pte = reinterpret_cast<u32 *>(0xFFC00000 | ((reinterpret_cast<u32>(v_addr) & 0xFFFFF000) >> 10));
		*pte = (*pte & (~PG_PRESENT));
		asm("invlpg %0"::"m"(v_addr));
	}

	return 0;
}

/*
 * Retourne l'adresse physique de la page associee a l'adresse virtuelle passee
 * en argument
 */
char *get_p_addr(char *v_addr)
{
	/* adresse virtuelle de l'entree du repertoire de pages */

	auto pde = reinterpret_cast<u32 *>(0xFFFFF000 | ((reinterpret_cast<u32>(v_addr) & 0xFFC00000) >> 20));
	if ((*pde & PG_PRESENT)) {
		auto pte = (u32 *) (0xFFC00000 | ((reinterpret_cast<u32>(v_addr) & 0xFFFFF000) >> 10));
		if ((*pte & PG_PRESENT))
			return reinterpret_cast<char *>((*pte & 0xFFFFF000) + (VADDR_PG_OFFSET(reinterpret_cast<u32>(v_addr))));
	}

	return nullptr;
}

		

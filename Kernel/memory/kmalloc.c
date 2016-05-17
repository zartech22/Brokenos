#include <utils/types.h>
#include <utils/lib.h>
#include <memory/mm.h>
#include <memory/kmalloc.h>
#include <video/Screen.h>

void* operator new(size_t size)
{
	return kmalloc(size);
}

void* operator new[](size_t size)
{
	return kmalloc(size);
}

void operator delete(void *mem)
{
	kfree(mem);
}

void operator delete[](void *mem)
{
	kfree(mem);
}

void* ksbrk(int n)
{
	struct kmalloc_header *chunk;
	char *p_addr;
	
	if((kern_heap + (n * PAGESIZE)) > (char*) KERN_HEAP_LIM)
	{
		Screen::getScreen().printError("ksbrk() : no virtual memomry left for kernel heap !");
		return (char*) -1;
	}
	
	chunk = (struct kmalloc_header*) kern_heap;
	
	//Alloc page libre
	for(int i = 0; i < n; i++)
	{
		p_addr = get_page_frame();
		
		if(p_addr < 0)
		{
			Screen::getScreen().printError("ksbrk : no free page frame available !");
			return (char*) -1;
		}
		
		pd0_add_page(kern_heap, p_addr, 0);
		
		kern_heap += PAGESIZE;
	}
	
	//Marquage du bloc venant d'etre etendu
	chunk->size = PAGESIZE * n;
	chunk->used = 0;
	
	return chunk;
}

void* kmalloc(unsigned long size)
{
	unsigned long realsize; //taille total enregirstrement
	struct kmalloc_header *chunk, *other;
	
	if((realsize = sizeof(struct kmalloc_header) + size) < KMALLOC_MINSIZE)
		realsize = KMALLOC_MINSIZE;
	
	// Recherche d'un bloc libre de 'realsize' octets dans le heap
	chunk = (struct kmalloc_header*) KERN_HEAP;
	
	while(chunk->used || chunk->size < realsize)
	{
		if(chunk->size == 0)
		{
			Screen::getScreen().printError("kmalloc : corrupted chunk with null size in heap");
			asm("hlt");
		}
		
		chunk = (struct kmalloc_header*) ((char*) chunk + chunk->size);
		
		if(chunk == (struct kmalloc_header*) kern_heap)
		{
			if(ksbrk((realsize / PAGESIZE) + 1) < 0)
			{
				Screen::getScreen().printError("kmalloc() : no more memory for kernel. STOP");
				asm("hlt");
			}
			else if(chunk > (struct kmalloc_header*) kern_heap)
			{
				Screen::getScreen().printError("kmalloc() : chunk after heap limit");
				asm("hlt");
			}
		}
	}
	
	//On a trouve un bloc libre avec size >= 'realsize'
	//On reduit au minimum la taille du bloc
	if(chunk->size - realsize < KMALLOC_MINSIZE)
		chunk->used = 1;
	else
	{
		other = (struct kmalloc_header*) ((char*) chunk + realsize);
		other->size = chunk->size - realsize;
		other->used = 0;
		
		chunk->size = realsize;
		chunk->used = 1;
	}
	
	return (char*) chunk + sizeof(struct kmalloc_header);
}

void kfree(const void *v_addr)
{
	struct kmalloc_header *chunk, *other;
	
	//On libere le bloc
	chunk = (struct kmalloc_header*) (v_addr - sizeof(struct kmalloc_header));
	chunk->used = 0;
		
	//On merge le nouveau bloc libere avec le suivant ci lui aussi libre
	while((other = (struct kmalloc_header*) ((char*) chunk + chunk->size))
			&& other < (struct kmalloc_header*) kern_heap
			&& other->used == 0)
		chunk->size += other->size;
}
			

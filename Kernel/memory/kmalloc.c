#include <utils/types.h>
#include <utils/lib.h>
#include <memory/mm.h>
#include <memory/kmalloc.h>
#include <video/Screen.h>

void* operator new(const size_t size)
{
	return kmalloc(size);
}

void* operator new[](const size_t size)
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

void operator delete(void *mem, size_t)
{
	operator delete(mem);
}

void operator delete[](void *mem, size_t)
{
	operator delete(mem);
}

void* ksbrk(const unsigned int n)
{
	if((kern_heap + (n * PAGESIZE)) > reinterpret_cast<char *>(KERN_HEAP_LIM))
	{
		Screen::getScreen().printError("ksbrk() : no virtual memomry left for kernel heap !");
		return reinterpret_cast<char *>(-1);
	}
	
	auto *chunk = reinterpret_cast<struct kmalloc_header *>(kern_heap);
	
	//Alloc page libre
    for(unsigned int i = 0; i < n; i++)
	{
		char *p_addr = get_page_frame();
		
		if(p_addr < static_cast<char*>(nullptr))
		{
			Screen::getScreen().printError("ksbrk : no free page frame available !");
			return reinterpret_cast<char *>(-1);
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
	kmalloc_header *chunk;
	
	if((realsize = sizeof(kmalloc_header) + size) < KMALLOC_MINSIZE)
		realsize = KMALLOC_MINSIZE;
	
	// Recherche d'un bloc libre de 'realsize' octets dans le heap
	chunk = reinterpret_cast<kmalloc_header *>(KERN_HEAP);
	
	while(chunk->used || chunk->size < realsize)
	{
		if(chunk->size == 0)
		{
			Screen::getScreen().printError("kmalloc : corrupted chunk with null size in heap");
			asm("hlt");
		}
		
		chunk = reinterpret_cast<kmalloc_header *>(reinterpret_cast<char *>(chunk) + chunk->size);
		
		if(chunk == reinterpret_cast<kmalloc_header *>(kern_heap))
		{
			if(ksbrk((realsize / PAGESIZE) + 1) < static_cast<char*>(nullptr))
			{
				Screen::getScreen().printError("kmalloc() : no more memory for kernel. STOP");
				asm("hlt");
			}
			else if(chunk > reinterpret_cast<kmalloc_header *>(kern_heap))
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
		kmalloc_header *other;
		other = reinterpret_cast<kmalloc_header *>(reinterpret_cast<char *>(chunk) + realsize);
		other->size = chunk->size - realsize;
		other->used = 0;
		
		chunk->size = realsize;
		chunk->used = 1;
	}
	
	return reinterpret_cast<char *>(chunk) + sizeof(kmalloc_header);
}

void* krealloc(void *ptr, unsigned long size)
{
    asm("hlt");

    if(!ptr)
        return kmalloc(size);

    if(size == 0)
    {
        kfree(ptr);
        return nullptr;
    }

    auto *chunk = reinterpret_cast<struct kmalloc_header *>(static_cast<char *>(ptr) - sizeof(struct kmalloc_header));

    if(!chunk->used || !chunk->size)
        asm("hlt");

    if(size > chunk->size)
    {
        void *newPtr = kmalloc(size);
        memcpy(static_cast<char *>(newPtr), static_cast<char *>(ptr), chunk->size);
        kfree(ptr);

        return newPtr;
    }
    else
    {
        auto *other = reinterpret_cast<kmalloc_header *>(static_cast<char *>(ptr) + size);
        other->size = chunk->size - size;
        other->used = 0;

        chunk->size = size;
        return ptr;
    }
}

void kfree(const void *v_addr)
{
	kmalloc_header *other;
	
	//On libere le bloc
    auto *chunk = reinterpret_cast<struct kmalloc_header *>((char *) v_addr - sizeof(struct kmalloc_header));
	chunk->used = 0;
		
	//On merge le nouveau bloc libere avec le suivant ci lui aussi libre
	while(((other = reinterpret_cast<kmalloc_header *>(reinterpret_cast<char *>(chunk) + chunk->size)))
			&& other < reinterpret_cast<kmalloc_header *>(kern_heap)
			&& other->used == 0)
		chunk->size += other->size;
}
			

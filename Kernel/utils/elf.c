#include <utils/elf.h>
#include <memory/mm.h>
#include <utils/lib.h>
#include <memory/kmalloc.h>
#include <video/Screen.h>

bool isElf(char *file)
{
    Elf32_header *header;

    header = (Elf32_header*) file;

    return (header->ident[0] == ELFMAG0 && header->ident[1] == ELFMAG1
            && header->ident[2] == ELFMAG2 && header->ident[3] == ELFMAG3);
}

u32 loadElf(char *file, page_directory *pd, page_list *mmap)
{
    char *p;
    u32 v_begin, v_end, v_addr;

    int i;

    Elf32_header *header;
    Elf_program_header *entry;

    header = (Elf32_header*) file;
    entry = (Elf_program_header*) (file + header->ph_off);

    if(!isElf(file))
    {
        Screen::getScreen().printInfo("%s : file not in ELF format !", __FUNCTION__);
        asm("hlt");
        return 0;
    }

    for(int pe = 0; pe < header->ph_num; pe++, entry++)
    {
        if(entry->type == PT_LOAD)
        {
            v_begin = entry->vaddr;
            v_end = entry->vaddr + entry->mem_size;

            for(v_addr = v_begin; v_addr < v_end; v_addr += PAGESIZE)
            {
                if(v_addr < USER_OFFSET)
                {
                    Screen::getScreen().printInfo("File can't load exec below %p", USER_STACK);
                    return 0;
                }

                if(v_addr > USER_STACK)
                {
                    Screen::getScreen().printInfo("File can't load exec above %p", USER_STACK);
                    return 0;
                }

                if(get_p_addr((char*)v_addr) == 0)
                {
                    if(mmap->page)
                    {
                        mmap->next = (struct page_list*)kmalloc(sizeof(struct page_list));

                        mmap->next->next = 0;
                        mmap->next->prev = mmap;
                        mmap = mmap->next;
                    }

                    mmap->page = (struct page*)kmalloc(sizeof(struct page));

                    mmap->page->p_addr = get_page_frame();
                    mmap->page->v_addr = (char*)(v_addr & 0xFFFFF000);

                    pd_add_page((char*)(v_addr & 0xFFFFF000), mmap->page->p_addr, PG_USER, pd);
                }
            }

            memcpy((char*)v_begin, (char*) (file + entry->offset), entry->file_size);

            if(entry->mem_size > entry->file_size)
                for(i = entry->file_size, p = (char*)entry->vaddr; i < entry->mem_size; i++)
                    p[i] = 0;
                //memset((char*)entry->vaddr, 0, entry->mem_size - entry->file_size);
        }
    }

    return header->entry;
}

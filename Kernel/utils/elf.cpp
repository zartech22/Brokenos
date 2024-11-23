#include <source_location>
#include <utils/elf.h>
#include <memory/mm.h>
#include <utils/lib.h>
#include <memory/kmalloc.h>
#include <video/Screen.h>

bool isElf(const char *file)
{
    const auto *header = reinterpret_cast<const Elf32_header *>(file);

    return (header->ident[0] == ELFMAG0 && header->ident[1] == ELFMAG1
            && header->ident[2] == ELFMAG2 && header->ident[3] == ELFMAG3);
}

uint32_t loadElf(char *file, page_directory *pd, page_list *mmap)
{
    char *p;
    uint32_t v_begin, v_end, v_addr;

    int i;

    const auto *header = reinterpret_cast<Elf32_header *>(file);
    const auto *entry = reinterpret_cast<Elf_program_header *>(file + header->ph_off);

    if(!isElf(file))
    {
        sScreen.printInfo("%s : file not in ELF format !", std::source_location::current().function_name());
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
                    sScreen.printInfo("File can't load exec below %p", USER_STACK);
                    return 0;
                }

                if(v_addr > USER_STACK)
                {
                    sScreen.printInfo("File can't load exec above %p", USER_STACK);
                    return 0;
                }

                if(get_p_addr(reinterpret_cast<char *>(v_addr)) == nullptr)
                {
                    if(mmap->page)
                    {
                        mmap->next = static_cast<page_list *>(kmalloc(sizeof(page_list)));

                        mmap->next->next = nullptr;
                        mmap->next->prev = mmap;
                        mmap = mmap->next;
                    }

                    mmap->page = static_cast<page *>(kmalloc(sizeof(page)));

                    mmap->page->p_addr = get_page_frame();
                    mmap->page->v_addr = reinterpret_cast<char *>(v_addr & 0xFFFFF000);

                    pd_add_page(reinterpret_cast<char *>(v_addr & 0xFFFFF000), mmap->page->p_addr, PG_USER, pd);
                }
            }

            memcpy(reinterpret_cast<char *>(v_begin), file + entry->offset, entry->file_size);

            if(entry->mem_size > entry->file_size)
                for(i = entry->file_size, p = reinterpret_cast<char *>(entry->vaddr); i < entry->mem_size; i++)
                    p[i] = 0;
                //memset((char*)entry->vaddr, 0, entry->mem_size - entry->file_size);
        }
    }

    return header->entry;
}

#pragma once

#include <utils/types.h>

#define KERNELMODE 0
#define USERMODE 1

#define MAXPID	32

struct process
{
	int pid;
	
	struct
	{
        uint32_t eax, ecx, edx, ebx;
		uint32_t esp, ebp, esi, edi;
		uint32_t eip, eflags;
		uint32_t cs:16, ss:16, ds:16, es:16, fs:16, gs:16;
		uint32_t cr3;
	} regs __attribute__ ((packed));
	
	struct
	{
		uint32_t esp0;
		uint16_t ss0;
	} kstack __attribute__ ((packed));
	
	//Redondance entre regs.cr3 et pd->bae->p_addr;
	struct page_directory *pd;
	
	//Pages frame utilisées par l'exe
    struct page_list *pglist;

	int state; // 0 not used, 1 ready/running, 2 sleep
	
} __attribute__ ((packed));

struct thread
{
    struct
    {
        uint32_t eax, ebx, ecx, edx;
        uint32_t esp, ebp, esi, edi;
        uint32_t eip, eflags;
        uint32_t cs:16, ss:16, ds:16, es:16, fs:16, gs:16;
        uint32_t cr3;
    } regs __attribute__ ((packed));

    char *function;

    int state;
};

#ifdef __PLIST__
	process p_list[MAXPID + 1];
	process *current = 0;
	int n_proc = 0;
#else
    extern "C" process p_list[];
    extern "C" process *current;
    extern "C" int n_proc;
#endif


int load_task(char*, uint32_t);
int load_task(const char *filename);

void createThread(void *fn);


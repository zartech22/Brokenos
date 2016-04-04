#include "types.h"

#define KERNELMODE 0
#define USERMODE 1

#define MAXPID	32

struct process
{
	int pid;
	
	struct
	{
		u32 eax, ebx, ecx, edx;
		u32 esp, ebp, esi, edi;
		u32 eip, eflags;
		u32 cs:16, ss:16, ds:16, es:16, fs:16, gs:16;
		u32 cr3;
	} regs __attribute__ ((packed));
	
	struct
	{
		u32 esp0;
		u16 ss0;
	} kstack __attribute__ ((packed));
	
	//Redondance entre regs.cr3 et pd->bae->p_addr;
	struct page_directory *pd;
	
	//Pages frame utilisées par l'exe
	struct page_list *pglist;
	
	int state; // 0 not used, 1 ready/running, 2 sleep
	
} __attribute__ ((packed));

#ifdef __PLIST__
	struct process p_list[MAXPID + 1];
	struct process *current = 0;
	int n_proc = 0;
#else
	extern struct process p_list[];
	extern struct process *current;
	extern int n_proc;
#endif

#ifdef __cplusplus
	extern "C" {
#endif
int load_task(char*, u32);
#ifdef __cplusplus
}
#endif
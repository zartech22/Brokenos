#pragma once

#include <utils/types.h>

#define PAGESIZE	4096
#define RAM_MAXSIZE 0x100000000
#define RAM_MAXPAGE	0x100000

#define IDTSIZE 0xFF //nb desc 
#define GDTSIZE 0xFF //nb desc

//#define IDTBASE 0x00000000 // addr phys IDT
//#define GDTBASE 0x00000800 // addr phys GDT

#define IDTBASE 0x00001000 // addr phys IDT
#define GDTBASE 0x00001800 // addr phys GDT

//#define	KERN_PDIR		0x00001000
#define	KERN_PDIR           0x00002000
#define	KERN_STACK          0x0009FFF0
#define	KERN_BASE           0x00100000
#define KERN_PG_HEAP		0x00800000
#define KERN_PG_HEAP_LIM	0x10000000
#define KERN_HEAP           0x10000000
#define KERN_HEAP_LIM		0x40000000

#define	USER_OFFSET 		0x40000000
#define	USER_STACK          0xE0000000

#define VADDR_PD_OFFSET(addr)	((addr) & 0xFFC00000) >> 22
#define VADDR_PT_OFFSET(addr)	((addr) & 0x003FF000) >> 12
#define VADDR_PG_OFFSET(addr)	((addr) & 0x00000FFF)
#define PAGE(addr)				(addr) >> 12

#define PAGING_FLAG	0x80000000
#define PSE_FLAG	0x00000010

#define PG_PRESENT	0x00000001
#define PG_WRITE	0x00000002
#define PG_USER		0x00000004
#define PG_4MB		0x00000080


#ifndef __MM_STRUCT__
#define __MM_STRUCT__

struct page
{
	char *v_addr;
	char *p_addr;
};

struct page_list
{
	struct page *page;
	page_list *next;
	page_list *prev;
};

struct page_directory
{
	page *base;
	page_list *pt;
};

struct vm_area
{
	char *vm_start;
	char *vm_end; //exclue
	vm_area *next;
	vm_area *prev;
};

#endif

#ifdef __MM__
inline char *kern_heap;
inline vm_area *free_vm;

inline auto pd0 = reinterpret_cast<uint32_t *>(KERN_PDIR);	//kernel page dir
inline char *pg0 = nullptr; //kernel page 0 (4MB)
inline auto pg1 = reinterpret_cast<char *>(0x400000); //kernel page 1 (4MB)
inline auto pg1_end = reinterpret_cast<char *>(0x800000); //limite page 1
inline uint8_t mem_bitmap[RAM_MAXPAGE / 8];	//bitmap allocation page 1Go
#else
extern char *kern_heap;
extern struct vm_area *free_vm;

extern uint32_t *pd0;
extern uint8_t mem_bitmap[];
#endif

//page libere ou utiliser
#define set_page_frame_used(page)	mem_bitmap[((uint32_t) page) / 8] |= (1 << (((uint32_t) page) % 8))
#define release_page_frame(p_addr)	mem_bitmap[((uint32_t) p_addr / PAGESIZE) / 8] &= ~(1 << (((uint32_t) p_addr / PAGESIZE) % 8))

//select une page vide ds le bitmap
char *get_page_frame();

//selectionne / libere page libre dans le bitmap et l'associe a page virtuelle libre du heap
page* get_page_from_heap();
page* get_page_from_heap(char* p_addr, const char* end_p_addr = nullptr);
int release_page_from_heap(char *);

//init les struct de donnees de gestion de la memoire
void init_mm(uint32_t);

//creer un rep de page pour une tache
struct page_directory* pd_create();
int pd_destroy(const struct page_directory*);

//Ajoute entree dans l'espace noyau
int pd0_add_page(const char*, char*, int);

//Ajoute / enleve une entree dans repertoire de page courant
int pd_add_page(char*, char*, int, struct page_directory*);
int pd_remove_page(char*);

//Retourne adresse physique associe a une adresse virtuelle
char* get_p_addr(char*);

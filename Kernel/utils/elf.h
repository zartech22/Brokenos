#pragma once

#include <utils/types.h>
#include <memory/mm.h>

/*
 * ELF identification
 */
#define	EI_MAG0		0
#define	EI_MAG1		1
#define	EI_MAG2		2
#define	EI_MAG3		3
#define	EI_CLASS	4
#define	EI_DATA		5
#define	EI_VERSION	6
#define EI_PAD		7

/* EI_MAG */
#define	ELFMAG0		0x7f
#define	ELFMAG1		'E'
#define	ELFMAG2		'L'
#define	ELFMAG3		'F'

/* EI_CLASS */
#define	ELFCLASSNONE	0	/* invalid class */
#define	ELFCLASS32	1	/* 32-bit objects */
#define	ELFCLASS64	2	/* 64-bit objects */

/* EI_DATA */
#define	ELFDATANONE	0	/* invalide data encoding */
#define	ELFDATA2LSB	1	/* least significant byte first (0x01020304 is 0x04 0x03 0x02 0x01) */
#define	ELFDATA2MSB	2	/* most significant byte first (0x01020304 is 0x01 0x02 0x03 0x04) */

/* EI_VERSION */
#define	EV_CURRENT	1
#define	ELFVERSION	EV_CURRENT

// Program entry type
#define	PT_NULL             0
#define	PT_LOAD             1
#define	PT_DYNAMIC          2
#define	PT_INTERP           3
#define	PT_NOTE             4
#define	PT_SHLIB            5
#define	PT_PHDR             6
#define	PT_LOPROC  0x70000000
#define	PT_HIPROC  0x7fffffff

typedef struct
{
    uchar   ident[16];  // ELF ident

    u16     type;       // 2 = Exec file
    u16     machine;    // 3 = intel arch

    u32     version;    // 1
    u32     entry;      // Start point
    u32     ph_off;     // ProgramHeader table offset
    u32     sh_off;     // SectionHeader table offset
    u32     flags;

    u16     header_size;    // ELF header size
    u16     ph_entry_size;  // ProgramHeader entry size
    u16     ph_num;         // entry's number
    u16     sh_entry_size;
    u16     sh_num;
    u16     sh_str_idx;     // Name string's section's index

} Elf32_header;

// Program header
typedef struct
{
    u32 type;
    u32 offset;
    u32 vaddr;
    u32 paddr;
    u32 file_size;
    u32 mem_size;
    u32 flags;
    u32 align;
} Elf_program_header;

bool isElf(char*);
u32 loadElf(char*, page_directory*, page_list*);

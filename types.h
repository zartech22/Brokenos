#ifndef _TYPES
#define _TYPES

#include <stddef.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned char uchar;

struct mb_partial_info
{
	unsigned long flags;
	unsigned long low_mem;
	unsigned long high_mem;
	unsigned long boot_device;
	unsigned long cmdline;
};
#endif

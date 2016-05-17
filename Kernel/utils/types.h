#ifndef _TYPES
#define _TYPES

#include <stddef.h>

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef unsigned char   uchar;

struct mb_partial_info
{
    u32 flags;

    u32 low_mem;
    u32 high_mem;

    u32 boot_device;

    u32 cmdline;

    u32 mods_count;
    u32 mods_addr;

    u32 syms[4];
    u32 mmap_length;
    u32 mmap_addr;

    u32 drives_length;
    u32 drives_addr;

    u32 config_table;

    u32 boot_loader_name;

    u32 apm_table;

    u32 vbe_control_info;
    u32 vbe_mode_info;
    u16 vbe_mode;
    u16 vbe_interface_seg;
    u16 vbe_interface_off;
    u16 vbe_interface_len;
};

struct VbeModeInfo
{
    unsigned short  ModeAttributes;
    unsigned char   WinAAttributes;
    unsigned char   WinBAttributes;
    unsigned short  WinGranularity;
    unsigned short  WinSize;
    unsigned short  WinASegment;
    unsigned short  WinBSegment;
    unsigned long   WinFuncPtr;
    unsigned short  BytesPerScanLine;
    unsigned short  XResolution;
    unsigned short  YResolution;
    unsigned char   XCharSize;
    unsigned char   YCharSize;
    unsigned char   NumberOfPlanes;
    unsigned char   BitsPerPixel;
    unsigned char   NumberOfBanks;
    unsigned char   MemoryModel;
    unsigned char   BankSize;
    unsigned char   NumberOfImagePages;
    unsigned char   Reserved_page;
    unsigned char   RedMaskSize;
    unsigned char   RedMaskPos;
    unsigned char   GreenMaskSize;
    unsigned char   GreenMaskPos;
    unsigned char   BlueMaskSize;
    unsigned char   BlueMaskPos;
    unsigned char   ReservedMaskSize;
    unsigned char   ReservedMaskPos;
    unsigned char   DirectColorModeInfo;
    unsigned long   PhysBasePtr;
    unsigned long   OffScreenMemOffset;
    unsigned short  OffScreenMemSize;
    unsigned char   Reserved[206];
} __attribute__((packed));

#endif

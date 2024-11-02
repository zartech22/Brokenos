#pragma once

#include <cstddef>

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef unsigned long long   u64;
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

    u64 framebuffer_addr;
    u32 framebuffer_pitch;
    u32 framebuffer_width;
    u32 framebuffer_height;
    u8  framebuffer_bpp;
    u8  framebuffer_type;

    union
    {
        struct
        {
            u32 framebuffer_pallette_addr;
            u16 framebuffer_palette_num_colors;
        };

        struct
        {
            u8 framebuffer_red_field_position;
            u8 framebuffer_red_mask_size;
            u8 framebuffer_green_field_position;
            u8 framebuffer_green_mask_size;
            u8 framebuffer_blue_field_position;
            u8 framebuffer_blue_mask_size;
        };
    };
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

struct mmapInfo
{
    u32 size;
    u32 addr_low;
    u32 addr_high;
    u32 length_low;
    u32 length_high;
    u8  type;
} __attribute__((packed)) ;

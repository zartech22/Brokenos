#pragma once

#include <cstdint>

struct mb_partial_info
{
    uint32_t flags;

    uint32_t low_mem;
    uint32_t high_mem;

    uint32_t boot_device;

    uint32_t cmdline;

    uint32_t mods_count;
    uint32_t mods_addr;

    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;

    uint32_t drives_length;
    uint32_t drives_addr;

    uint32_t config_table;

    uint32_t boot_loader_name;

    uint32_t apm_table;

    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;

    union
    {
        struct
        {
            uint32_t framebuffer_pallette_addr;
            uint16_t framebuffer_palette_num_colors;
        };

        struct
        {
            uint8_t framebuffer_red_field_position;
            uint8_t framebuffer_red_mask_size;
            uint8_t framebuffer_green_field_position;
            uint8_t framebuffer_green_mask_size;
            uint8_t framebuffer_blue_field_position;
            uint8_t framebuffer_blue_mask_size;
        };
    };
};

struct VbeModeInfo
{
    unsigned short  ModeAttributes;
    uint8_t   WinAAttributes;
    uint8_t   WinBAttributes;
    unsigned short  WinGranularity;
    unsigned short  WinSize;
    unsigned short  WinASegment;
    unsigned short  WinBSegment;
    unsigned long   WinFuncPtr;
    unsigned short  BytesPerScanLine;
    unsigned short  XResolution;
    unsigned short  YResolution;
    uint8_t   XCharSize;
    uint8_t   YCharSize;
    uint8_t   NumberOfPlanes;
    uint8_t   BitsPerPixel;
    uint8_t   NumberOfBanks;
    uint8_t   MemoryModel;
    uint8_t   BankSize;
    uint8_t   NumberOfImagePages;
    uint8_t   Reserved_page;
    uint8_t   RedMaskSize;
    uint8_t   RedMaskPos;
    uint8_t   GreenMaskSize;
    uint8_t   GreenMaskPos;
    uint8_t   BlueMaskSize;
    uint8_t   BlueMaskPos;
    uint8_t   ReservedMaskSize;
    uint8_t   ReservedMaskPos;
    uint8_t   DirectColorModeInfo;
    unsigned long   PhysBasePtr;
    unsigned long   OffScreenMemOffset;
    unsigned short  OffScreenMemSize;
    uint8_t   Reserved[206];
} __attribute__((packed));

struct mmapInfo
{
    uint32_t size;
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t length_low;
    uint32_t length_high;
    uint8_t  type;
} __attribute__((packed)) ;

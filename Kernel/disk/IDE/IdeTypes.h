#ifndef IDETYPES_H
#define IDETYPES_H

enum DriveRole
{
    Master	= 0x00,
    Slave	= 0x01
};

enum BusRole
{
    PrimaryBus,
    SecundaryBus
};

enum ATA_Command
{
    ATA_READ			= 0x20,
    ATA_WRITE			= 0x30,

    ATA_IDENTIFY		= 0xEC,
    ATA_IDENTIFY_ATAPI	= 0xA1
};

enum ATA_Registers : u16
{
    ATA_DATA		= 0x00,

    ATA_FEATURES	= 0x01,

    ATA_SECT_COUNT	= 0x02,

    ATA_LBA_LOW		= 0x03,
    ATA_LBA_MID		= 0x04,
    ATA_LBA_HIGH	= 0x05,

    ATA_DRIVE		= 0x06,

    ATA_COMMAND		= 0x07,
    ATA_STATUS		= 0x07
};

struct Partition
{
    u8	bootable;
    u8	s_head;
    u16	s_sector	: 6;
    u16	s_cycl		: 10;
    u8	sys_id;
    u8	e_head;
    u16	e_sector	: 6;
    u16	e_cycl		: 10;
    u32	s_lba;
    u32	size;
} __attribute__ ((packed));

#endif // IDETYPES_H

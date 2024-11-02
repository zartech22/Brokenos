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

enum class ATA_Command
{
    ATA_READ			= 0x20,
    ATA_WRITE			= 0x30,

    ATA_IDENTIFY		= 0xEC,
    ATA_IDENTIFY_ATAPI	= 0xA1
};

enum ATA_Registers : u16
{
    ATA_DATA		= 0x00,

    ATA_ERROR       = 0x01,
    ATA_FEATURES	= 0x01,

    ATA_SECT_COUNT	= 0x02,

    ATA_LBA_LOW		= 0x03,
    ATA_LBA_MID		= 0x04,
    ATA_LBA_HIGH	= 0x05,

    ATA_DRIVE		= 0x06,

    ATA_COMMAND		= 0x07,
    ATA_STATUS		= 0x07
};

class ATA_Status {
public:
    enum StatusField : u8 {
        ERR = 0x01,
        IDX = 0x02,
        CORR = 0x04,
        DRQ = 0x08,
        SRV = 0x10,
        DF = 0x20,
        RDY = 0x40,
        BSY = 0x80
    };

    ATA_Status() : _status(0) {}
    explicit ATA_Status(const u8 status) : _status(status) {}

    [[nodiscard]] u8 getByte() const {
        return _status;
    }

    bool operator[](const StatusField field) const {
        return _status & field;
    }

    explicit operator bool() const {
        return _status;
    }

private:
    u8 _status;
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

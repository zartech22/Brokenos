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

enum ATA_Registers : uint16_t
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
    enum StatusField : uint8_t {
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
    explicit ATA_Status(const uint8_t status) : _status(status) {}

    [[nodiscard]] uint8_t getByte() const {
        return _status;
    }

    bool operator[](const StatusField field) const {
        return _status & field;
    }

    explicit operator bool() const {
        return _status;
    }

private:
    uint8_t _status;
};

struct Partition
{
    uint8_t	bootable;
    uint8_t	s_head;
    uint16_t	s_sector	: 6;
    uint16_t	s_cycl		: 10;
    uint8_t	sys_id;
    uint8_t	e_head;
    uint16_t	e_sector	: 6;
    uint16_t	e_cycl		: 10;
    uint32_t	s_lba;
    uint32_t	size;
} __attribute__ ((packed));

#endif // IDETYPES_H

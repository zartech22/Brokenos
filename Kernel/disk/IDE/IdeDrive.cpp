#include <disk/IDE/IdeDrive.h>
#include <disk/FileSystems/Ext2/Ext2FS.h>
#include <core/io.h>

#include <disk/IDE/Partitions.h>

IdeDrive::IdeDrive(u16 regPorts, u16 controlPort, enum DriveRole pos)
: _regPorts(regPorts), _controlPort(controlPort), _role(pos), _isConnected(false)
{
    memset(_modelName, '\0', 41);
    initDevice();

    if(!_isConnected)
        strcpy(_modelName, "Device not connected\0");
    else
    {
        _part = new Partitions(this);
        _part->fillPartitions();
    }
}

IdeDrive::IdeDrive(const IdeDrive &o)
{
    *this = o;
}

const IdeDrive& IdeDrive::operator=(const IdeDrive &o)
{
    _regPorts = o._regPorts;
    _controlPort = o._controlPort;
    _role = o._role;
    _isConnected = o._isConnected;

    memset(_modelName, 0, 41);
    strcpy(_modelName, o._modelName);

    _part = o._part;

    return *this;
}

void IdeDrive::displayPartitions()
{
    struct Partition p;
    for(int i = 0; i < _part->getPartitionsNumber(); ++i)
    {
        p = _part->getPartition(i);
        u32 size =  p.size * 512 / 1024 / 1024 / 1024;
        size += p.size % 1024;

        char *data = read(p.s_lba + 2, 2);

        if(!data)
            return;

        const char* isExt2 = (Ext2FS::isExt2FS(data) && p.size != 0) ? "Ext2 Part" : "Unk Part";

        Screen::getScreen().printDebug("Partition %d - start : %u, size %u Go, %s, SysId : %x, Bootable : %s", i + 1, p.s_lba, size, isExt2, p.sys_id, (p.bootable == 0x80) ? "True" : "False");
    }
}

/*IdeDrive::~IdeDrive()
{
    if(_part != 0)
        delete _part;
}*/

void IdeDrive::initDevice()
{
    bool isAta = true;

    if(inb(_regPorts + ATA_STATUS) & 0x01) // If there is an error stop
        return;

    u8 device = (_role == Master) ? 0xA0 : 0xB0;
    outb(_regPorts + ATA_DRIVE, device); // Select the device

    for(u16 i = 2; i <= 5; i++) // Send 0 to port 2-5
        outb(_regPorts + i, 0);


    outb(_regPorts + ATA_COMMAND, ATA_IDENTIFY);  // Send IDENTIFY command

    while(inb(_regPorts + ATA_STATUS) >> 7);

    if(!inb(_regPorts + ATA_STATUS)) // If status return 0, there is no device connected
        return;
    else if(inb(_regPorts + ATA_STATUS) & 0x01)  // If 'error' bit set
    {
        u8 data1 = inb(_regPorts + 4);
        u8 data2 = inb(_regPorts + 5);

        if(data1 == 0xEC && data2 == 0xEC)
            Screen::getScreen().printDebug("SATA Device");
        else if(data1 == 0x14 && data2 == 0xEB)
        {
            isAta = false;
            Screen::getScreen().printDebug("ATAPI Device");
            outb(_regPorts + ATA_COMMAND, ATA_IDENTIFY_ATAPI);
        }
        else
        {
            Screen::getScreen().printk("Unknown : %x, %x\n", data1, data2);
            isAta = false;
        }
    }

    getIdentifyData();

    _isConnected = true;

    if(isAta)
    {
        _part = new Partitions(this);
        _part->fillPartitions();
    }
}

void IdeDrive::getIdentifyData()
{
    while(!(inb(_regPorts + ATA_STATUS) & 0x08)); // Whait for data to be received

    char tmp[2];
    u16 word;

    for(int i = 0; i < 256; i++) // Get data
    {
        word = inw(_regPorts + ATA_DATA);

        if(i >= 27 && i <= 46)
        {
            int offset = (i - 27) * 2;

            if((word & 0x00FF) == ' ' && (word >> 8) == ' ')
            {
                tmp[0] = 0;
                tmp[1] = 0;
            }
            else
            {
                tmp[1] = (uchar) word;
                tmp[0] = (uchar) (word >> 8);
            }

            memcpy(&(_modelName[offset]), tmp, 2);
        }
    }
}

void IdeDrive::diskSelect(int block, int n)
{
    // LBA 28bits
    while(inb(_regPorts + ATA_STATUS) >> 7);

    outb(_regPorts + ATA_FEATURES, 0x00); // NULL byte to port 0x1F1
    outb(_regPorts + ATA_SECT_COUNT, n); //Sector count
    outb(_regPorts + ATA_LBA_LOW, (uchar) block); //Low 8 bits of the block address
    outb(_regPorts + ATA_LBA_MID, (uchar) (block >> 8)); //Next 8 bits
    outb(_regPorts + ATA_LBA_HIGH, (uchar) (block >> 16)); //Next

    //Drive indicator, magic bits and highest 4 bits of the block address
    outb(_regPorts + ATA_DRIVE, 0xE0 | (_role << 4) | ((block >> 24) & 0x0F));
}

char* IdeDrive::read(int numblock, int count)
{
    if(!_isConnected)
        return 0;

    diskSelect(numblock, count);

    u16 tmp;
    char *buffer = (char*)kmalloc(512 * count);
    memset(buffer, 0, 512 * count);

    outb(_regPorts + ATA_COMMAND, ATA_READ);

    while(!(inb(_regPorts + ATA_STATUS) & 0x08));

    for(int idx = 0; idx < 256 * count; idx++)
    {
        tmp = inw(_regPorts + ATA_DATA);
        buffer[idx * 2] = (uchar) tmp;
        buffer[idx * 2 + 1] = (uchar) (tmp >> 8);
        while((inb(_regPorts + ATA_STATUS) >> 7));
    }

    return buffer;
}

/**
 * @brief IdeDrive::read Reads <em>count</em> bytes of data from <em>offset</em> bytes on disk
 * and put the read data in <em>buffer</em>
 * @param offset Offset in bytes where we start getting data
 * @param buffer Buffer to fill
 * @param count Number in bytes to read
 */
void IdeDrive::read(int offset, char *buffer, int count)
{
    int blockBegin, blockEnd, blockNbr;

    blockBegin = offset / 512;
    blockEnd = (offset + count) / 512;

    blockNbr = blockEnd - blockBegin + 1;

    const char *data = read(blockBegin, blockNbr);

    if(data)
    {
        memcpy(buffer, (data + offset % 512), count);
        kfree(data);
    }
    else
        memset(buffer, 0, count);
}

void IdeDrive::write(int numblock, int count, const char * const data)
{
    if(!_isConnected)
        return;

    diskSelect(numblock, count);

    u16 tmp;

    outb(_regPorts + ATA_COMMAND, ATA_WRITE);

    while(!(inb(_regPorts + ATA_STATUS) & 0x08));

    for(int idx = 0; idx < 256 * count; idx++)
    {
        tmp = (data[idx * 2 + 1] << 8) | data[idx * 2];
        outw(_regPorts + ATA_DATA, tmp);
    }
}

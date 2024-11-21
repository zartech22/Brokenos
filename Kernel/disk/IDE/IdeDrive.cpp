#include <disk/IDE/IdeDrive.h>
#include <disk/FileSystems/Ext2/Ext2FS.h>
#include <core/io.h>

#include <disk/IDE/Partitions.h>

IdeDrive::IdeDrive(const uint16_t regPorts, const uint16_t controlPort, const DriveRole pos)
: _isConnected(false), _modelName("Device not connected"), _regPorts(regPorts), _controlPort(controlPort), _role(pos)
{
    initDevice();
}

IdeDrive::IdeDrive(const IdeDrive &o)
{
    *this = o;
}

const IdeDrive& IdeDrive::operator=(const IdeDrive &o)
{
    if(this == &o)
        return *this;

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
    for(int i = 0; i < _part->getPartitionsNumber(); ++i)
    {
        const Partition &p = _part->getPartition(i);
        uint32_t size =  p.size * 512 / 1024 / 1024 / 1024;
        size += p.size % 1024;

        char *data = read(p.s_lba + 2, 2);

        if(!data)
            return;

        const char* isExt2 = (Ext2FS::isExt2FS(data) && p.size != 0) ? "Ext2 Part" : "Unk Part";

        sScreen.printDebug("Partition %d - start : %u, size %u Go, %s, SysId : %x, Bootable : %s", i + 1, p.s_lba, size, isExt2, p.sys_id, (p.bootable == 0x80) ? "True" : "False");
    }
}

/*IdeDrive::~IdeDrive()
{
    if(_part != 0)
        delete _part;
}*/

void IdeDrive::initDevice()
{
    sScreen.printDebug("show status: %x", readStatus());
    diskSelect();

    sendCommand(ATA_Command::ATA_IDENTIFY);

    ATA_Status status;

    do {
        status = readStatus();
    } while(readStatus()[ATA_Status::BSY]);

    sScreen.printDebug("Device ID : %x", status.getByte());

    if(!status || status.getByte() == 0x7F) {
        sScreen.printDebug("No device found");
        return;
    }

    bool isAta = true;

    if(inb(_regPorts + ATA_LBA_MID) != 0 && _regPorts + ATA_LBA_HIGH != 0) {
        sScreen.printDebug("IT is no ATA... Investigating...");
        isAta = false;

        uint16_t data = (inb(_regPorts + 4) << 8);
        data |= inb(_regPorts + 5);

        if(data == 0xECEC)
            sScreen.printDebug("SATA Device");
        else if(data == 0x14EB)
        {
            isAta = false;
            sScreen.printDebug("ATAPI Device");
            sendCommand(ATA_Command::ATA_IDENTIFY_ATAPI);
        }
        else
        {
            sScreen.println("Unknown IDE device : %x", data);
            isAta = false;
        }
    } else {
        do {
            status = readStatus();
        } while(!status[ATA_Status::DRQ] && !status[ATA_Status::ERR]);
    }

    getIdentifyData();

    _isConnected = true;

    if(isAta) {
        _part = new Partitions(this);
        _part->fillPartitions();
    }
}

void IdeDrive::getIdentifyData()
{
    if(waitStatus()) {
        char tmp[2];
        uint16_t word;

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
                    tmp[1] = (uint8_t) word;
                    tmp[0] = (uint8_t) (word >> 8);
                }

                memcpy(&(_modelName[offset]), tmp, 2);
            }
        }
    }
}

void IdeDrive::sendCommand(ATA_Command command) const {
    outb(_regPorts + ATA_COMMAND, command);
    waitStatus();
}

ATA_Status IdeDrive::readStatus() const {
    return ATA_Status(inb(_controlPort));
}

bool IdeDrive::waitStatus() const {
    ATA_Status status = readStatus();
    bool stop = false;
    bool ok = false;

    while(!stop) {
        if(status[ATA_Status::ERR]) {
            sScreen.printError("ERROR on IDE drive. Status is: %b", status);
            sScreen.printError("Error is: %x", inb(_regPorts + ATA_ERROR));
            stop = true;
        }
        else if(status[ATA_Status::DRQ] && !status[ATA_Status::BSY]) {
            ok = stop = true;
        }
        else if(status[ATA_Status::DF]) {
            sScreen.printError("Drive Fault Error");
            stop = true;
        } else if(!status[ATA_Status::BSY]) {
            ok = stop = true;
        } else {
            status = readStatus();
        }
    }

    return ok;
}

void IdeDrive::diskSelect() const {
    uint8_t device = (_role == Master) ? 0xA0 : 0xB0;
    outb(_regPorts + ATA_DRIVE, device); // Select the device
    for(uint16_t i = 2; i <= 5; i++) // Send 0 to port 2-5
        outb(_regPorts + i, 0);

    for(auto i = 0; i < 13; ++i) // waits for drive to take the lead
        readStatus();
}


void IdeDrive::diskSelect(int block, int n) const {
    // LBA 28bits
    while(readStatus()[ATA_Status::BSY]) {}

    outb(_regPorts + ATA_FEATURES, 0x00); // NULL byte to port 0x1F1
    outb(_regPorts + ATA_SECT_COUNT, n); //Sector count
    outb(_regPorts + ATA_LBA_LOW, static_cast<uint8_t>(block)); //Low 8 bits of the block address
    outb(_regPorts + ATA_LBA_MID, static_cast<uint8_t>(block >> 8)); //Next 8 bits
    outb(_regPorts + ATA_LBA_HIGH, static_cast<uint8_t>(block >> 16)); //Next

    //Drive indicator, magic bits and highest 4 bits of the block address
    outb(_regPorts + ATA_DRIVE, 0xE0 | (_role << 4) | ((block >> 24) & 0x0F));
}

char* IdeDrive::read(int numblock, int count)
{
    if(!_isConnected)
        return nullptr;

    diskSelect(numblock, count);

    uint16_t tmp;
    auto buffer = new char[512 * count];
    memset(buffer, 0, 512 * count);

    sendCommand(ATA_Command::ATA_READ);

    for(int idx = 0; idx < 256 * count; idx++)
    {
        tmp = inw(_regPorts + ATA_DATA);
        buffer[idx * 2] = (uint8_t) tmp;
        buffer[idx * 2 + 1] = (uint8_t) (tmp >> 8);
        while(readStatus()[ATA_Status::BSY]) {};
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
void IdeDrive::read(const int offset, char *buffer, const int count)
{
    const int blockBegin = offset / 512;
    const int blockEnd = (offset + count) / 512;

    const int blockNbr = blockEnd - blockBegin + 1;

    auto data = read(blockBegin, blockNbr);

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

    uint16_t tmp;

    sendCommand(ATA_Command::ATA_WRITE);

    for(int idx = 0; idx < 256 * count; idx++)
    {
        tmp = (data[idx * 2 + 1] << 8) | data[idx * 2];
        outw(_regPorts + ATA_DATA, tmp);
    }
}

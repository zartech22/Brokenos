#include "ide.h"
#include "pci.h"
#include "io.h"
#include "strLib.h"
#include "kmalloc.h"
#include "lib.h"

#include "Ext2FS.h"

IdeCtrl::IdeCtrl(u8 bus, u8 device, u8 function)
	: _bus(bus), _device(device), _function(function)
{
	_primaryPorts[0] = 0x1F0;
	_primaryPorts[1] = 0x3F6;

	_secundaryPorts[0] = 0x170;
	_secundaryPorts[1] = 0x376;

	checkPorts();

	_drives[0] = IdeDrive(_primaryPorts[0], _primaryPorts[1], Master);
	_drives[1] = IdeDrive(_primaryPorts[0], _primaryPorts[1], Slave);

	_drives[2] = IdeDrive(_secundaryPorts[0], _secundaryPorts[1], Master);
	_drives[3] = IdeDrive(_secundaryPorts[0], _secundaryPorts[1], Slave);


	for(int i = 0; i < 4; ++i)
		_connetedDevice[i] = _drives[i].isConnected();
}

IdeDrive& IdeCtrl::getDrive(BusRole bus, DriveRole drive)
{
	unsigned int index = (bus == SecundaryBus); // 0 if primary, else 1
	index *= 2;
	index += (drive == Slave);

	return _drives[index];
}

void IdeCtrl::displayModelNames()
{
	for(int i = 0; i < 4; i++)
		if(_connetedDevice[i])
			Screen::getScreen().printk("IDE Device : %s\n", _drives[i].getModelName());
}

void IdeCtrl::displayTree()
{
	Screen &s = Screen::getScreen();

	s.putcar(0xB3);
	s.putcar(0x0A);

	s.putcar(0xC3);
	s.printk(" Primary master : %s\n", _drives[0].getModelName());

	s.putcar(0xC3);
	s.printk(" Primary slave : %s\n", _drives[1].getModelName());

	s.putcar(0xC3);
	s.printk(" Secundary master : %s\n", _drives[2].getModelName());

	s.putcar(0xC0);
	s.printk(" Secundary slave : %s\n", _drives[3].getModelName());
}

void IdeCtrl::checkPorts()
{
	u8 progIf = pciConfigReadByte(_bus, _device, _function, 0x09);

	if(!(progIf & 0x1))
		return;
	else
	{
		_primaryPorts[0] = 0xFFFC & pciConfigReadWord(_bus, _device, _function, 0x10);
		_primaryPorts[1] = 0xFFFC & pciConfigReadByte(_bus, _device, _function, 0x14);

		_secundaryPorts[0] = 0xFFFC & pciConfigReadWord(_bus, _device, _function, 0x18);
		_secundaryPorts[1] = 0xFFFC & pciConfigReadByte(_bus, _device, _function, 0x1C);
	}
}

/***********************************************************************
 * IdeDrive
 * *********************************************************************
*/


IdeDrive::IdeDrive(u16 regPorts, u16 controlPort, enum DriveRole pos)
: _regPorts(regPorts), _controlPort(controlPort), _role(pos), _isConnected(false)
{
	memset(_modelName, '\0', 41);
	initDevice();

	if(!_isConnected)
		strcpy(_modelName, "Device not connected\0");
	/*else
	{
		_part = new Partitions(this);
		_part->fillPartitions();
	}*/
}

IdeDrive::IdeDrive(const IdeDrive &o) : IdeDrive(o._regPorts, o._controlPort, o._role)
{

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
}

void IdeDrive::displayPartitions()
{
	struct Partition p;
	for(int i = 0; i < 4; ++i)
	{
		p = _part->getPartition(i);
		u32 size =  p.size * 512 / 1024 / 1024 / 1024;
		size += p.size % 1024;

		char *data = read(p.s_lba + 2, 2);

		const char* isExt2 = (Ext2FS::isExt2FS(data)) ? "Ext2 Part" : "Unk Part";
		Screen::getScreen().printDebug("Partition %d - start : %u, size %u Go, %s, SysId : %x, Bootable : %s", i + 1, p.s_lba, size, isExt2, p.sys_id, (p.bootable == 0x80) ? "True" : "False");

		if(strcmp(isExt2, "Ext2 Part") == 0)
		{
            FileSystem *fs = new Ext2FS(p, *this);
            struct file * f = fs->getDirEntries(fs->getRoot());
            struct file *tmp = f;

			bool cont = true;

			while(cont)
			{
				if(tmp)
				{
                    if(fs->isDirectory(tmp))
						Screen().printDebug("Dir %s", tmp->name);
					else
						Screen().printDebug("File %s", tmp->name);


					if(strcmp(tmp->name, "taMere.txt") == 0)
					{
						Screen().print("Data TaMere : ");
                        struct file *file = fs->getFile(tmp->name);
                        struct filePrivateData *data = (filePrivateData*)file->privateData;
                        char *c = fs->readFile(data->inode);

                        for(int i = 0; i < data->inode->size; ++i, ++c)
							Screen().print("%c", *c);

						Screen().print("\n");
					}

					if(tmp->next == f || !tmp->next)
						cont = false;
					else
						tmp = tmp->next;
				}
			}
		}

		kfree(data);
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

	for(u16 i = ATA_SECT_COUNT; i <= ATA_LBA_HIGH; i++) // Send 0 to port 2-5
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

	//Drive indicator, magic bits and highest 4 bits of th block address
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
	}

	return buffer;
}

void IdeDrive::read(int offset, char *buffer, int count)
{
	int blockBegin, blockEnd, blockNbr;

	blockBegin = offset / 512;
	blockEnd = (offset + count) / 512;

	blockNbr = blockEnd - blockBegin + 1;

	char *data = read(blockBegin, blockNbr);

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

/***********************************************************************
 * Partitions
 * ********************************************************************/

void Partitions::fillPartition(unsigned int i)
{
	struct Partition &p = _partitions[i];

	//Screen::getScreen().printError("Offset : %x", (0x01BE + (i * 0x10)));
	_drive->read(0x01BE + (i * 0x10), (char*)(&p), sizeof(struct Partition));
}

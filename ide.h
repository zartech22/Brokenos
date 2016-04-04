#ifndef __IDE__
#define __IDE__

#include "types.h"
#include "strLib.h"
#include "lib.h"

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

class IdeCtrl;
class Partitions;

class IdeDrive
{
public:
	IdeDrive() : _regPorts(0), _controlPort(0), _part(0)
	{
		memset(_modelName, '\0', 41);
		strcpy(_modelName, "Uninitialized IDE Device");
	}
	
	~IdeDrive() {}
	
	IdeDrive(u16 regPorts, u16 controlPort, enum DriveRole pos);
	bool isConnected() const { return _isConnected; }
	const char* getModelName() const { return _modelName; }
	
	void displayPartitions() const;
	
	char* read(int numblock, int count);
	void read(int offset, char *buffer, int count);
	void write(int numblock, int count, const char * const data);
	
private:
	friend class IdeCtrl;
	
	Partitions *_part;
	
	bool _isConnected;
	
	char _modelName[41];
	u16 _regPorts, _controlPort;
	enum DriveRole _role;
	
	void diskSelect(int block, int n);
	void initDevice();
	
	void getIdentifyData();
		
	IdeDrive(const IdeDrive &o);
	const IdeDrive& operator=(const IdeDrive &o);
};

class IdeCtrl
{
public:
	IdeCtrl() {}
	IdeCtrl(u8 bus, u8 device, u8 function);
	
	IdeDrive& getDrive(BusRole bus, DriveRole drive);
	
	void displayModelNames();
	void displayTree();
	
private:
	u8 _bus, _device, _function;
	
	u16 _primaryPorts[2], _secundaryPorts[2];
	
	bool _connetedDevice[4];
	
	IdeDrive _drives[4];
	
	void checkPorts();
};

class Partitions
{
public:
	Partitions(IdeDrive *drive) : _drive(drive) { memset((char*)_partitions, 0, 4 * sizeof(struct Partition)); }
	
	void fillPartitions()
	{ for(unsigned int i = 0; i < 4; ++i) fillPartition(i); }
	
	struct Partition getPartition(unsigned int i)
	{ Screen::getScreen().printDebug("Start LBA : %u, Size : %u", _partitions[i].s_lba, _partitions[i].size); return _partitions[i]; }
	
private:
	IdeDrive *_drive;
	struct Partition _partitions[4];
	
	void fillPartition(unsigned int i);
};

#endif

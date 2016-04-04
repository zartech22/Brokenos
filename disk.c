#include "types.h"
#include "io.h"
#include "strLib.h"

int bl_common(int drive, int numblock, int count)
{
	// LBA 28bits
	while(inb(0x1F7) >> 7);
		
	outb(0x1F1, 0x00); // NULL byte to port 0x1F1
	outb(0x1F2, count); //Sector count
	outb(0x1F3, (uchar) numblock); //Low 8 bits of the block address
	outb(0x1F4, (uchar) (numblock >> 8)); //Next 8 bits
	outb(0x1F5, (uchar) (numblock >> 16)); //Next
	
	//Drive indicator, magic bits and highest 4 bits of th block address
	outb(0x1F6, 0xE0 | (drive << 4) | ((numblock >> 24) & 0x0F));
	
	return 0;
}

int bl_read(int drive, int numblock, int count, char *buf)
{
	u16 tmp;
			
	bl_common(drive, numblock, count);
	outb(0x1F7, 0x20);
			
	while(!(inb(0x1F7) & 0x08));
		
	for(int idx = 0; idx < 256 * count; idx++)
	{
		tmp = inw(0x1F0);
		buf[idx * 2] = (uchar) tmp;
		buf[idx * 2 + 1] = (uchar) (tmp >> 8);
	}
		
	return count;
}

int bl_write(int drive, int numblock, int count, const char *buf)
{
	u16 tmp;
		
	bl_common(drive, numblock, count);
	outb(0x1F7, 0x30);
	
	while(!(inb(0x1F7) & 0x08));
	
	for(int idx = 0; idx < 256 * count; idx++)
	{
		tmp = (buf[idx * 2 + 1] << 8) | buf[idx * 2];
		outw(0x1F0, tmp);
	}
	
	return count;
}

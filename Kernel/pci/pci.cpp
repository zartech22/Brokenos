#define __PCI__

#include <pci/pci.h>
#include <core/io.h>
#include <utils/lib.h>
#include <video/Screen.h>
#include <memory/kmalloc.h>
#include <memory/mm.h>
#include <pci/pciIds.h>
#include <disk/IDE/IdeCtrl.h>


u32 pciConfigReadDWord(u8 bus, u8 slot, u8 function, u8 offset)
{
	u32 address;
	u32 lbus = (u32) bus;
	u32 lslot = (u32) slot;
	u32 lfunc = (u32) function;
		
	address = (u32)((lbus << 16) | (lslot << 11) | (lfunc << 8)
	| (offset & 0xFC) | ((u32) 0x80000000));
	
	outl(0xCF8, address);
	
	return inl(0xCFC);
}

u16 pciConfigReadWord(u8 bus, u8 slot, u8 function, u8 offset)
{
	u32 address;
	u32 lbus = (u32) bus;
	u32 lslot = (u32) slot;
	u32 lfunc = (u32) function;
	
	u16 tmp;
	
	address = (u32)((lbus << 16) | (lslot << 11) | (lfunc << 8)
	| (offset & 0xFC) | ((u32) 0x80000000));
	
	outl(0xCF8, address);
	
	tmp = (u16)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
	
	return tmp;
}

u8 pciConfigReadByte(u8 bus, u8 slot, u8 function, u8 offset)
{
	u32 address;
	u32 lbus = (u32) bus;
	u32 lslot = (u32) slot;
	u32 lfunc = (u32) function;
	
	u8 tmp;
	
	address = (u32)((lbus << 16) | (lslot << 11) | (lfunc << 8)
	| (offset & 0xFC) | ((u32) 0x80000000));
	
	outl(0xCF8, address);
	
	tmp = (u8)((inl(0xCFC) >> ((offset & 3) * 8)) & 0xFF);
	
	return tmp;
}

void pciConfigWrite(u8 bus, u8 slot, u8 function, u8 offset, u32 data)
{
    u32 address;
    u32 lbus = (u32)bus;
    u32 lslot = (u32)slot;
    u32 lfunc = (u32)function;

    address = (u32)((lbus << 16) | (lslot << 11) | (lfunc << 8)
                    | (offset & 0xFC) | ((u32) 0x80000000));

    outl(0xCFC, data);
}

inline u16 pciCheckVendor(u8 bus, u8 slot)
{	
	return pciConfigReadWord(bus, slot, 0, 0);
}

const char* pciGetVendorName(u16 vendor)
{
	char *name = new char[10];
	memset(name, '\0', 10);
	
	switch(vendor)
	{
		case Intel:
			strcpy(name, "Intel");
			break;
		case Realtec:
			strcpy(name, "Realtek");
			break;
		case NVidia:
			strcpy(name, "NVidia");
			break;
		case Oracle:
			strcpy(name, "Oracle");
			break;
		case AMD:
			strcpy(name, "AMD");
			break;
		case APPLE:
			strcpy(name, "Apple");
            break;
        case VmWare:
            strcpy(name, "VmWare");
            break;
        case Ensoniq:
            strcpy(name, "Ensoniq");
            break;
        case Mylex:
            strcpy(name, "Mylex");
            break;
		default:
			itoa(name, vendor, 16);
			break;
	}
	
	return name;
}

const char* pciGetClassCodeName(u8 code)
{
    char *name = new char[35];
	memset(name, '\0', 35);
	
	switch(code)
	{
		case MassStorageCtrl:
			strcpy(name, "Mass Storage Controller");
			break;
		case NetworkCtrl:
			strcpy(name, "Network Controller");
			break;
		case DisplayCtrl:
			strcpy(name, "Display Controller");
			break;
		case MultimediaCtrl:
			strcpy(name, "Multimedia Controller");
			break;
		case MemoryCtrl:
			strcpy(name, "Memory Controller");
			break;
		case BridgeCtrl:
			strcpy(name, "Bridge Controller");
			break;
		case SimpleCommCtrl:
			strcpy(name, "Simple Communication Controler");
			break;
		case BaseSystemPeriph:
			strcpy(name, "Base System Peripherals");
			break;
		case InputDevices:
			strcpy(name, "Input Devices");
			break;
		case DockingStations:
			strcpy(name, "Docking Stations");
			break;
		case Processors:
			strcpy(name, "Processor");
			break;
		case SerialBusCtrl:
			strcpy(name, "Serial Bus Controller");
			break;
		case WirelessCtrl:
			strcpy(name, "Wireless Controller");
			break;
		case IntelligentIOCtrl:
			strcpy(name, "Intelligent I/O Controller");
			break;
		case SatelliteCommCtrl:
			strcpy(name, "Satellite Communication Controller");
			break;
		case EncDecCtrl:
			strcpy(name, "Encryption/Decryption Controller");
			break;
		case DataAcqSignalProces:
			strcpy(name, "Data Acquisition/Signal process");
			break;
		default:
			itoa(name, code, 16);
			break;
	}
	
	return name;
}

void displayDevice(u16 vendor, u8 classCode, u8 subClassCode, u16 devId)
{
	const char *vendorName = pciGetVendorName(vendor);
	const char *className = pciGetClassCodeName(classCode);
	
	sScreen.printk("\t%s, Class : %s, SubClass : %x, DevId : %x\n", vendorName,
				className, subClassCode, devId);
	
	delete[] vendorName;
	delete[] className;
}

void checkFunction(u8 bus, u8 device, u8 function)
{
	u8 classCode = pciConfigReadByte(bus, device, function, 0x0B);
	u8 subClass = pciConfigReadByte(bus, device, function, 0x0A);
	
	u16 vendor = pciConfigReadWord(bus, device, function, 0x00);
	u16 devId = pciConfigReadWord(bus, device, function, 0x02);
	
	if((classCode == BridgeCtrl) && (subClass == 0x04))
	{
		u8 secBus = pciConfigReadByte(bus, device, function, 0x19);
		checkBus(secBus);
	}
	
    displayDevice(vendor, classCode, subClass, devId);


    if(classCode == SerialBusCtrl && subClass == 0x03)
    {
        u8 progIf = pciConfigReadByte(bus, device, function, 0x9);

        if(progIf == 0x0)
            sScreen.println("UHCI Controller");
        else if(progIf == 0x10)
        {
            u32 bar0 = pciConfigReadDWord(bus, device, function, 0x10);
            u8 type = bar0 & 0x6;
            char *p = reinterpret_cast<char*>(0xFFFFFFF0 & bar0);

            pciConfigWrite(bus, device, function, 0x10, (u32)~0);

            u32 res = pciConfigReadDWord(bus, device, function, 0x10);
            res &= 0xFFFFFFF0;
            res = ~res;
            res++;

            pciConfigWrite(bus, device, function, 0x10, bar0);

            sScreen.println("OHCI Controller. type = %b, Memmory map %p - %p", type, p, p + res);

            struct page *pages = get_page_from_heap(p, p + res);
            u32 *mem = (u32*)pages->v_addr;

            u8 rev = (*mem & 0xFF);

            sScreen.printError("OHCI Version : %d.%d", rev >> 4, rev & 0xF);

            u32 hcControl = mem[1];

            if(!(hcControl & 0x100) && !(hcControl & 0xC0))
            {
                sScreen.printError("No active driver");

                mem[1] = mem[1] | 0x40;
            }
        }
        else
            sScreen.println("Unknown USB Controller");
    }

    // TODO: gérer le cas où il y a plusieurs contrôleur IDE
	if(classCode == MassStorageCtrl && subClass == 0x01)
	{
		ctrl = IdeCtrl(bus, device, function);
		ctrl.displayTree();
	}
}

void checkDevice(u8 bus, u8 device)
{
	u16 vendor = pciConfigReadWord(bus, device, 0, 0);
	
	if(vendor == 0xFFFF) return;
	
	checkFunction(bus, device, 0);
	
	u8 headerType = pciConfigReadByte(bus, device, 0, 0x0E);
	
	for(u8 function = 1; function < 8; function++)
		if(pciConfigReadWord(bus, device, function, 0x00) != 0xFFFF)
			checkFunction(bus, device, function);
}

void checkBus(u8 bus)
{
	for(u8 device = 0; device < 32; device++)
		checkDevice(bus, device);
}

void pciGetVendors()
{
	u8 headerType = pciConfigReadByte(0, 0, 0, 0x0E);
	
	if((headerType & 0x80) == 0)
		checkBus(0);
	else
	{
		for(u8 function = 0; function < 8; function++)
		{
			if(pciConfigReadWord(0, 0, function, 0x00) != 0xFFFF) break;
			
			checkBus(function);
		}
	}
}

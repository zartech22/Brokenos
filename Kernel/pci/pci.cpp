#include <pci/pci.h>
#include <core/io.h>
#include <utils/lib.h>
#include <video/Screen.h>
#include <memory/mm.h>
#include <pci/pciIds.h>
#include <disk/IDE/IdeCtrl.h>
#include <utils/String.h>


u32 pciConfigReadDWord(u8 bus, u8 slot, u8 function, u8 offset)
{
	u32 address;
	u32 lbus = bus;
	u32 lslot = slot;
	u32 lfunc = function;
		
	address = lbus << 16 | lslot << 11 | lfunc << 8
	          | offset & 0xFC | 0x80000000;
	
	outl(0xCF8, address);
	
	return inl(0xCFC);
}

u16 pciConfigReadWord(u8 bus, u8 slot, u8 function, u8 offset)
{
	u32 address;
	u32 lbus = bus;
	u32 lslot = slot;
	u32 lfunc = function;
	
	u16 tmp;
	
	address = lbus << 16 | lslot << 11 | lfunc << 8
	          | offset & 0xFC | 0x80000000;
	
	outl(0xCF8, address);
	
	tmp = static_cast<u16>((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
	
	return tmp;
}

u8 pciConfigReadByte(u8 bus, u8 slot, u8 function, u8 offset)
{
	u32 address;
	u32 lbus = bus;
	u32 lslot = slot;
	u32 lfunc = function;
	
	u8 tmp;
	
	address = lbus << 16 | lslot << 11 | lfunc << 8
	          | offset & 0xFC | 0x80000000;
	
	outl(0xCF8, address);
	
	tmp = static_cast<u8>((inl(0xCFC) >> ((offset & 3) * 8)) & 0xFF);
	
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

inline u16 pciCheckVendor(const u8 bus, const u8 slot)
{	
	return pciConfigReadWord(bus, slot, 0, 0);
}

String* pciGetVendorName(const u16 vendor)
{
	switch(vendor)
	{
		case Intel:
			return new String("Intel");
		case Realtec:
			return new String("Realtek");
		case NVidia:
			return new String("NVidia");
		case Oracle:
			return new String("Oracle");
		case AMD:
			return new String("AMD");
		case APPLE:
			return new String("Apple");
        case VmWare:
            return new String("VmWare");
        case Ensoniq:
            return new String("Ensoniq");
        case Mylex:
            return new String("Mylex");
		default:
			return stoa(vendor, 16);
	}
}

String* pciGetClassCodeName(u8 code)
{
	switch(code)
	{
		case MassStorageCtrl:
			return new String("Mass Storage Controller");
		case NetworkCtrl:
			return new String("Network Controller");
		case DisplayCtrl:
			return new String("Display Controller");
		case MultimediaCtrl:
			return new String("Multimedia Controller");
		case MemoryCtrl:
			return new String("Memory Controller");
		case BridgeCtrl:
			return new String("Bridge Controller");
		case SimpleCommCtrl:
			return new String("Simple Communication Controler");
		case BaseSystemPeriph:
			return new String("Base System Peripherals");
		case InputDevices:
			return new String("Input Devices");
		case DockingStations:
			return new String("Docking Stations");
		case Processors:
			return new String("Processor");
		case SerialBusCtrl:
			return new String("Serial Bus Controller");
		case WirelessCtrl:
			return new String("Wireless Controller");
		case IntelligentIOCtrl:
			return new String("Intelligent I/O Controller");
		case SatelliteCommCtrl:
			return new String("Satellite Communication Controller");
		case EncDecCtrl:
			return new String("Encryption/Decryption Controller");
		case DataAcqSignalProces:
			return new String("Data Acquisition/Signal process");
		default:
			return stoa(code, 16);
	}
}

void displayDevice(const u16 vendor, const u8 classCode, const u8 subClassCode, const u16 devId)
{
	String *vendorName = pciGetVendorName(vendor);
	String *className = pciGetClassCodeName(classCode);
	
	sScreen.printk("\t%S, Class : %S, SubClass : %x, DevId : %x\n", vendorName,
				className, subClassCode, devId);
	
	delete vendorName;
	delete className;
}

void checkFunction(const u8 bus, const u8 device, const u8 function)
{
	const u8 classCode = pciConfigReadByte(bus, device, function, 0x0B);
	const u8 subClass = pciConfigReadByte(bus, device, function, 0x0A);

	const u16 vendor = pciConfigReadWord(bus, device, function, 0x00);
	const u16 devId = pciConfigReadWord(bus, device, function, 0x02);
	
	if((classCode == BridgeCtrl) && (subClass == 0x04))
	{
		const u8 secBus = pciConfigReadByte(bus, device, function, 0x19);
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
	        const u32 bar0 = pciConfigReadDWord(bus, device, function, 0x10);
	        const u8 type = bar0 & 0x6;
	        auto p = reinterpret_cast<char*>(0xFFFFFFF0 & bar0);

            pciConfigWrite(bus, device, function, 0x10, static_cast<u32>(~0));

            u32 res = pciConfigReadDWord(bus, device, function, 0x10);
            res &= 0xFFFFFFF0;
            res = ~res;
            res++;

            pciConfigWrite(bus, device, function, 0x10, bar0);

            sScreen.println("OHCI Controller. type = %b, Memmory map %p - %p", type, p, p + res);

	        const page *pages = get_page_from_heap(p, p + res);
	        const auto mem = reinterpret_cast<u32 *>(pages->v_addr);

	        const u8 rev = (*mem & 0xFF);

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
	    IdeCtrl::addController(bus, device, function);
}

void checkDevice(const u8 bus, const u8 device)
{
	u16 vendor = pciConfigReadWord(bus, device, 0, 0);
	
	if(vendor == 0xFFFF) return;
	
	checkFunction(bus, device, 0);
	
	u8 headerType = pciConfigReadByte(bus, device, 0, 0x0E);

	if((headerType & 0x80) != 0x00) {
		for(u8 function = 1; function < 8; function++)
			if(pciConfigReadWord(bus, device, function, 0x00) != 0xFFFF)
				checkFunction(bus, device, function);
	}
}

void checkBus(const u8 bus)
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

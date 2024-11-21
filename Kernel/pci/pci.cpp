#include <pci/pci.h>
#include <core/io.h>
#include <utils/lib.h>
#include <video/Screen.h>
#include <memory/mm.h>
#include <pci/pciIds.h>
#include <disk/IDE/IdeCtrl.h>
#include <utils/String.h>


uint32_t pciConfigReadDWord(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset)
{
	uint32_t address;
	uint32_t lbus = bus;
	uint32_t lslot = slot;
	uint32_t lfunc = function;
		
	address = lbus << 16 | lslot << 11 | lfunc << 8
	          | offset & 0xFC | 0x80000000;
	
	outl(0xCF8, address);
	
	return inl(0xCFC);
}

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset)
{
	uint32_t address;
	uint32_t lbus = bus;
	uint32_t lslot = slot;
	uint32_t lfunc = function;
	
	uint16_t tmp;
	
	address = lbus << 16 | lslot << 11 | lfunc << 8
	          | offset & 0xFC | 0x80000000;
	
	outl(0xCF8, address);
	
	tmp = static_cast<uint16_t>((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
	
	return tmp;
}

uint8_t pciConfigReadByte(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset)
{
	uint32_t address;
	uint32_t lbus = bus;
	uint32_t lslot = slot;
	uint32_t lfunc = function;
	
	uint8_t tmp;
	
	address = lbus << 16 | lslot << 11 | lfunc << 8
	          | offset & 0xFC | 0x80000000;
	
	outl(0xCF8, address);
	
	tmp = static_cast<uint8_t>((inl(0xCFC) >> ((offset & 3) * 8)) & 0xFF);
	
	return tmp;
}

void pciConfigWrite(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t data)
{
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)function;

    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8)
                    | (offset & 0xFC) | ((uint32_t) 0x80000000));

    outl(0xCFC, data);
}

inline uint16_t pciCheckVendor(const uint8_t bus, const uint8_t slot)
{	
	return pciConfigReadWord(bus, slot, 0, 0);
}

String* pciGetVendorName(const uint16_t vendor)
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

String* pciGetClassCodeName(uint8_t code)
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

void displayDevice(const uint16_t vendor, const uint8_t classCode, const uint8_t subClassCode, const uint16_t devId)
{
	String *vendorName = pciGetVendorName(vendor);
	String *className = pciGetClassCodeName(classCode);
	
	sScreen.printk("\t%S, Class : %S, SubClass : %x, DevId : %x\n", vendorName,
				className, subClassCode, devId);
	
	delete vendorName;
	delete className;
}

void checkFunction(const uint8_t bus, const uint8_t device, const uint8_t function)
{
	const uint8_t classCode = pciConfigReadByte(bus, device, function, 0x0B);
	const uint8_t subClass = pciConfigReadByte(bus, device, function, 0x0A);

	const uint16_t vendor = pciConfigReadWord(bus, device, function, 0x00);
	const uint16_t devId = pciConfigReadWord(bus, device, function, 0x02);
	
	if((classCode == BridgeCtrl) && (subClass == 0x04))
	{
		const uint8_t secBus = pciConfigReadByte(bus, device, function, 0x19);
		checkBus(secBus);
	}
	
    displayDevice(vendor, classCode, subClass, devId);


    if(classCode == SerialBusCtrl && subClass == 0x03)
    {
        uint8_t progIf = pciConfigReadByte(bus, device, function, 0x9);

        if(progIf == 0x0)
            sScreen.println("UHCI Controller");
        else if(progIf == 0x10)
        {
	        const uint32_t bar0 = pciConfigReadDWord(bus, device, function, 0x10);
	        const uint8_t type = bar0 & 0x6;
	        auto p = reinterpret_cast<char*>(0xFFFFFFF0 & bar0);

            pciConfigWrite(bus, device, function, 0x10, static_cast<uint32_t>(~0));

            uint32_t res = pciConfigReadDWord(bus, device, function, 0x10);
            res &= 0xFFFFFFF0;
            res = ~res;
            res++;

            pciConfigWrite(bus, device, function, 0x10, bar0);

            sScreen.println("OHCI Controller. type = %b, Memmory map %p - %p", type, p, p + res);

	        const page *pages = get_page_from_heap(p, p + res);
	        const auto mem = reinterpret_cast<uint32_t *>(pages->v_addr);

	        const uint8_t rev = (*mem & 0xFF);

            sScreen.printError("OHCI Version : %d.%d", rev >> 4, rev & 0xF);

            uint32_t hcControl = mem[1];

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

void checkDevice(const uint8_t bus, const uint8_t device)
{
	uint16_t vendor = pciConfigReadWord(bus, device, 0, 0);
	
	if(vendor == 0xFFFF) return;
	
	checkFunction(bus, device, 0);
	
	uint8_t headerType = pciConfigReadByte(bus, device, 0, 0x0E);

	if((headerType & 0x80) != 0x00) {
		for(uint8_t function = 1; function < 8; function++)
			if(pciConfigReadWord(bus, device, function, 0x00) != 0xFFFF)
				checkFunction(bus, device, function);
	}
}

void checkBus(const uint8_t bus)
{
	for(uint8_t device = 0; device < 32; device++)
		checkDevice(bus, device);
}

void pciGetVendors()
{
	uint8_t headerType = pciConfigReadByte(0, 0, 0, 0x0E);
	
	if((headerType & 0x80) == 0)
		checkBus(0);
	else
	{
		for(uint8_t function = 0; function < 8; function++)
		{
			if(pciConfigReadWord(0, 0, function, 0x00) != 0xFFFF) break;
			
			checkBus(function);
		}
	}
}

#include <disk/IDE/IdeCtrl.h>
#include <video/Screen.h>
#include <pci/pci.h>

#include <disk/IDE/IdeDrive.h>

Vector<IdeCtrl*> *IdeCtrl::IdeList = nullptr;

IdeCtrl::IdeCtrl(uint8_t bus, uint8_t device, uint8_t function)
    : _bus(bus), _device(device), _function(function)
{
    _primaryPorts[0] = 0x1F0;
    _primaryPorts[1] = 0x3F6;

    _secondaryPorts[0] = 0x170;
    _secondaryPorts[1] = 0x376;

    checkPorts();

    _drives[0] = new IdeDrive(_primaryPorts[0], _primaryPorts[1], Master);
    _drives[1] = new IdeDrive(_primaryPorts[0], _primaryPorts[1], Slave);

    _drives[2] = new IdeDrive(_secondaryPorts[0], _secondaryPorts[1], Master);
    _drives[3] = new IdeDrive(_secondaryPorts[0], _secondaryPorts[1], Slave);


    for(int i = 0; i < 4; ++i)
        _connectedDevice[i] = _drives[i]->isConnected();
}

IdeDrive& IdeCtrl::getDrive(BusRole bus, DriveRole drive)
{
    unsigned int index = (bus == SecundaryBus) ? 1 : 0; // 0 if primary, else 1
    index *= 2;
    index += (drive == Slave);

    return *_drives[index];
}

void IdeCtrl::displayModelNames()
{
    for(int i = 0; i < 4; i++)
        if(_connectedDevice[i])
            sScreen.printk("IDE Device : %s\n", _drives[i]->getModelName());
}

void IdeCtrl::displayTree()
{
    sScreen.println("Ide controller: PCI(%u, %u, %u)", _bus, _device, _function);
    sScreen.putcar(0xC8);
    sScreen.putcar('\n');

    sScreen.putcar(0xC6);
    sScreen.printk(" Primary master : %s\n", _drives[0]->getModelName());

    sScreen.putcar(0xC6);
    sScreen.printk(" Primary slave : %s\n", _drives[1]->getModelName());

    sScreen.putcar(0xC6);
    sScreen.printk(" Secundary master : %s\n", _drives[2]->getModelName());

    sScreen.putcar(0xC4);
    sScreen.printk(" Secundary slave : %s\n", _drives[3]->getModelName());
}

void IdeCtrl::checkPorts()
{
    uint8_t progIf = pciConfigReadByte(_bus, _device, _function, 0x09);

    sScreen.println("Checking ports...");
    sScreen.println("ProfIF is: %b", progIf);

    if(!(progIf & 0x1))
    {
        sScreen.printDebug("Dans le return...");
    }
    else
    {
        sScreen.printDebug("Custom port found!");
        _primaryPorts[0] = 0xFFFC & pciConfigReadWord(_bus, _device, _function, 0x10);
        _primaryPorts[1] = 0xFFFC & pciConfigReadByte(_bus, _device, _function, 0x14);

        _secondaryPorts[0] = 0xFFFC & pciConfigReadWord(_bus, _device, _function, 0x18);
        _secondaryPorts[1] = 0xFFFC & pciConfigReadByte(_bus, _device, _function, 0x1C);
        sScreen.printDebug("Primary ports are: %x %x", _primaryPorts[0], _primaryPorts[1]);
        sScreen.printDebug("Secondary ports are: %x %x", _secondaryPorts[0], _secondaryPorts[1]);
    }
}
#ifndef IDECTRL_H
#define IDECTRL_H

#include <utils/types.h>
#include <disk/IDE/IdeTypes.h>

class IdeDrive;

class IdeCtrl
{
public:
    IdeCtrl() : _bus(0xFF), _device(0xFF), _function(0xFF) {}
    IdeCtrl(u8 bus, u8 device, u8 function);
    ~IdeCtrl() {}

    IdeDrive &getDrive(BusRole bus, DriveRole drive);

    void displayModelNames();
    void displayTree();

    bool isNull() const { return (_bus == 0xFF && _device == 0xFF && _function == 0xFF); }

private:
    u8 _bus, _device, _function;

    u16 _primaryPorts[2], _secundaryPorts[2];

    bool _connetedDevice[4];

    IdeDrive* _drives[4];

    void checkPorts();
};

#endif // IDECTRL_H

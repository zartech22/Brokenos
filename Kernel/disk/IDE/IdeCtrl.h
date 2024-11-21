#ifndef IDECTRL_H
#define IDECTRL_H

#include <utils/types.h>
#include <utils/Vector.h>
#include <disk/IDE/IdeTypes.h>
#include <memory/kmalloc.h>

class IdeDrive;

class IdeCtrl
{
public:
    static IdeCtrl& addController(uint8_t bus, uint8_t device, uint8_t function)
    {
        auto *ctrl = new IdeCtrl(bus, device, function);
        addControllerToList(ctrl);

        return *ctrl;
    }

    static Vector<IdeCtrl*>& getControllerList()
    {
        return *IdeList;
    }

    IdeCtrl() : _bus(0xFF), _device(0xFF), _function(0xFF), _primaryPorts{0xFF, 0xFF}, _secondaryPorts{0xFF, 0xFF},
                _connectedDevice{false, false, false, false}, _drives{nullptr, nullptr, nullptr, nullptr}
    {
    }

    IdeCtrl(uint8_t bus, uint8_t device, uint8_t function);
    ~IdeCtrl() = default;

    IdeDrive &getDrive(BusRole bus, DriveRole drive);

    void displayModelNames();
    void displayTree();

    bool isNull() const { return (_bus == 0xFF && _device == 0xFF && _function == 0xFF); }

private:
    static Vector<IdeCtrl*>* IdeList;

    static void addControllerToList(IdeCtrl * const ctrl)
    {
        if(!IdeList)
            IdeList = new Vector<IdeCtrl*>();

        IdeList->push_back(ctrl);
    }

    uint8_t _bus, _device, _function;

    uint16_t _primaryPorts[2], _secondaryPorts[2];

    bool _connectedDevice[4];

    IdeDrive* _drives[4];

    void checkPorts();
};

#endif // IDECTRL_H

#pragma once

#include <utils/lib.h>
#include <video/Screen.h>
#include <disk/IDE/IdeTypes.h>

class Partitions;

class IdeDrive
{
public:
    IdeDrive() : _isConnected(false), _modelName{}, _regPorts(0), _controlPort(0), _role(Master)
    {
        memset(_modelName, '\0', 41);
        strcpy(_modelName, "Uninitialized IDE Device");
    }

    ~IdeDrive() { Screen::getScreen().printError("IDE_DRIVE DELETED !");}

    IdeDrive(u16 regPorts, u16 controlPort, DriveRole pos);
    [[nodiscard]] bool isConnected() const { return _isConnected; }
    [[nodiscard]] const char* getModelName() const { return _modelName; }

    void displayPartitions();

    char* read(int numblock, int count);
    void read(int offset, char *buffer, int count);
    void write(int numblock, int count, const char * data);

private:
    friend class IdeCtrl;

    Partitions *_part{};

    bool _isConnected;

    char _modelName[41];
    u16 _regPorts, _controlPort;
    DriveRole _role;

    void sendCommand(ATA_Command command) const;
    bool waitStatus() const;
    ATA_Status readStatus() const;

    void diskSelect(int block, int n) const;
    void diskSelect() const;
    void initDevice();

    void getIdentifyData();

    IdeDrive(const IdeDrive &o);
    const IdeDrive& operator=(const IdeDrive &o);
};

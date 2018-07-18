#ifndef IDEDRIVE_H
#define IDEDRIVE_H

#include <utils/lib.h>
#include <video/Screen.h>
#include <disk/IDE/IdeTypes.h>

class Partitions;
//class IdeCtrl;

class IdeDrive
{
public:
    IdeDrive() : _regPorts(0), _controlPort(0), _part(0)
    {
        memset(_modelName, '\0', 41);
        strcpy(_modelName, "Uninitialized IDE Device");
    }

    ~IdeDrive() { Screen::getScreen().printError("IDE_DRIVE DELETED !");}

    IdeDrive(u16 regPorts, u16 controlPort, enum DriveRole pos);
    bool isConnected() const { return _isConnected; }
    const char* getModelName() const { return _modelName; }

    void displayPartitions();

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

#endif // IDEDRIVE_H

#ifndef PARTITIONS_H
#define PARTITIONS_H

#include <utils/types.h>
#include <disk/IDE/IdeTypes.h>
#include <utils/Vector.h>

#include <disk/IDE/IdeDrive.h>

class IdeDrive;

class Partitions
{
public:
    explicit Partitions(IdeDrive *drive) : _drive(drive) {}

    u8 getPartitionsNumber() const { return _partitions.size(); }

    void fillPartitions()
    {
        struct Partition p;

        for(u8 i = 0; i < 4; ++i)
        {
            _drive->read(0x01BE + (i * 0x10), (char*)(&p), sizeof(struct Partition));

            if(p.size == 0 || p.s_lba == 0)
                break;

            struct Partition *part = new struct Partition;
            *part = p;
            _partitions.push_back(part);
            checkFilesystem(*part);
        }
    }

    struct Partition getPartition(unsigned int i)
    { return *(_partitions[i]); }

private:
    IdeDrive *_drive;
    //struct Partition _partitions[4];
    Vector<struct Partition*, false> _partitions;

    void checkFilesystem(struct Partition &partition);
};

#endif // PARTITIONS_H

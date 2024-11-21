#pragma once

#include <utils/types.h>
#include <disk/IDE/IdeTypes.h>
#include <utils/Vector.h>

#include <disk/IDE/IdeDrive.h>

class IdeDrive;

class Partitions
{
public:
    explicit Partitions(IdeDrive *drive) : _drive(drive) {}

    [[nodiscard]] uint8_t getPartitionsNumber() const { return _partitions.size(); }

    void fillPartitions()
    {
        for(uint8_t i = 0; i < 4; ++i)
        {
            auto *part = new struct Partition;
            _drive->read(0x01BE + (i * 0x10), reinterpret_cast<char *>(part), sizeof(Partition));

            if(part->size == 0 || part->s_lba == 0) {
                delete part;
                break;
            }

            _partitions.push_back(part);
            checkFilesystem(*part);
        }
    }

    Partition getPartition(const unsigned int i)
    { return *(_partitions[i]); }

private:
    IdeDrive *_drive;
    Vector<Partition*, false> _partitions;

    void checkFilesystem(Partition &partition) const;
};

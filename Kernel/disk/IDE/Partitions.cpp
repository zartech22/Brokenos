#include <disk/IDE/Partitions.h>
#include <disk/FileSystems/Ext2/Ext2FS.h>

void Partitions::checkFilesystem(struct Partition &partition)
{
    char *data = _drive->read(partition.s_lba + 2, 2);

    bool isExt2 = (partition.size != 0 && Ext2FS::isExt2FS(data));

    if(isExt2)
        Ext2FS::initializeFS(partition, *_drive);
}

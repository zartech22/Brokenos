#include <disk/IDE/Partitions.h>
#include <disk/FileSystems/Ext2/Ext2FS.h>

void Partitions::checkFilesystem(Partition &partition) const {
    char *data = _drive->read(partition.s_lba + 2, 2);

    if(partition.size != 0 && Ext2FS::isExt2FS(data))
        Ext2FS::initializeFS(partition, *_drive);
}

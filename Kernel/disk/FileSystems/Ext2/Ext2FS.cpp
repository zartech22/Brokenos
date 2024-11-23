#include <disk/FileSystems/Ext2/Ext2FS.h>
#include <utils/lib.h>
#include <memory/kmalloc.h>
#include <video/Screen.h>

namespace kernel::disk::fileSystems {
    bool Ext2FS::isExt2FS(const char *data) {
        if (!data) {
            sScreen.printError("Data NULL !");
            return false;
        }

        return (reinterpret_cast<const ext2fs::super_block *>(data)->ext2_magic == EXT2_MAGIC_NUMBER);
    }

    Ext2FS::Ext2FS(const Partition &part, IdeDrive &drive) : FileSystem(part, drive) {
        initFsRoot();
    }

    void Ext2FS::initFsRoot() {
        readSuperBlock();

        _blockSize = 1024 << _sb->log_block_size;

        const uint16_t i = (_sb->block_count / _sb->blocks_per_group) +
                      ((_sb->block_count % _sb->blocks_per_group) ? 1 : 0);

        const uint16_t j = (_sb->inodes_count / _sb->inodes_per_group) +
                      ((_sb->inodes_count % _sb->inodes_per_group) ? 1 : 0);

        _groupNumber = (i > j) ? i : j;

        readGroupBlock();

        File *rootFile = getRoot();

        auto *data = new ext2fs::filePrivateData;

        rootFile->name = new char[strlen("/") + 1];
        strcpy(rootFile->name, "/");

        data->inum = EXT2_INUM_ROOT;
        data->node = readInode(EXT2_INUM_ROOT);

        rootFile->content = nullptr;
        rootFile->privateData = static_cast<void *>(data);
        rootFile->parent = rootFile;
        rootFile->leaf = getDirEntries(rootFile);
        rootFile->next = nullptr;
        rootFile->prev = nullptr;
    }

    void Ext2FS::readSuperBlock() {
        _sb = reinterpret_cast<ext2fs::super_block *>(readFromDisk(1024, sizeof(ext2fs::super_block)));
    }

    void Ext2FS::readGroupBlock() {
        const int offset = (_blockSize == 1024) ? 2048 : _blockSize;

        const int gd_size = _groupNumber * sizeof(struct ext2fs::group_desc);

        _groups = reinterpret_cast<ext2fs::group_desc *>(readFromDisk(offset, gd_size));
    }

    ext2fs::inode *Ext2FS::readInode(const int num) const {
        const int gr_num = (num - 1) / _sb->inodes_per_group;

        const int index = (num - 1) % _sb->inodes_per_group;

        const int offset = _groups[gr_num].inode_table * _blockSize + index * _sb->inode_size;

        auto *inode = reinterpret_cast<ext2fs::inode *>(readFromDisk(offset, _sb->inode_size));

        return inode;
    }

    char *Ext2FS::readFile(const char *path) {
        File *f = getFile(path);

        if (f) {
            const auto *data = static_cast<struct ext2fs::filePrivateData *>(f->privateData);
            if (!data->node)
                readInode(data->inum);
            f->size = data->node->size;
            return readFile(f);
        }

        return nullptr;
    }

    char *Ext2FS::readFile(File *file) {
        char *mmap_base;
        char *buf;

        int *p;
        int *pp;
        int n;

        const auto *data = static_cast<ext2fs::filePrivateData *>(file->privateData);

        if (!data)
            return nullptr;

        const ext2fs::inode *inode = data->node;

        unsigned int size = inode->size; // Taille totale du fichier
        file->size = inode->size;
        char *mmap_head = mmap_base = new char[size];

        // Direct block number
        for (int i = 0; i < 12 && inode->block[i]; ++i) {
            buf = readFromDisk(inode->block[i] * _blockSize, _blockSize);

            n = ((size > _blockSize) ? _blockSize : size);

            memcpy(mmap_head, buf, n);
            mmap_head += n;
            size -= n;

            kfree(buf);
        }

        // Indirect block number
        if (inode->block[12]) {
            p = reinterpret_cast<int *>(readFromDisk(inode->block[12] * _blockSize, _blockSize));

            for (int i = 0; i < _blockSize / 4 && p[i]; ++i) {
                buf = readFromDisk(p[i] * _blockSize, _blockSize);

                n = ((size > _blockSize) ? _blockSize : size);

                memcpy(mmap_head, buf, n);
                mmap_head += n;
                size -= n;

                kfree(buf);
            }

            delete p;
        }

        // Bi-indirect block number
        if (inode->block[13]) {
            p = reinterpret_cast<int *>(readFromDisk(inode->block[13] * _blockSize, _blockSize));

            for (int i = 0; i < _blockSize / 4 && p[i]; ++i) {
                pp = reinterpret_cast<int *>(readFromDisk(p[i] * _blockSize, _blockSize));

                for (int j = 0; j < _blockSize / 4 && pp[j]; ++j) {
                    buf = readFromDisk(pp[j] * _blockSize, _blockSize);

                    n = ((size > _blockSize) ? _blockSize : size);

                    memcpy(mmap_head, buf, n);
                    mmap_head += n;
                    size -= n;

                    delete buf;
                }

                delete pp;
            }

            delete p;
        }

        // Tri-indirect block number
        if (inode->block[14]) {
            p = reinterpret_cast<int *>(readFromDisk(inode->block[14] * _blockSize, _blockSize));

            for (int i = 0; i < _blockSize / 4 && p[i]; ++i) {
                pp = reinterpret_cast<int *>(readFromDisk(p[i] * _blockSize, _blockSize));

                for (int j = 0; j < _blockSize / 4 && pp[j]; ++j) {
                    int *ppp = reinterpret_cast<int *>(readFromDisk(pp[j] * _blockSize, _blockSize));

                    for (int k = 0; k < _blockSize / 4 && ppp[k]; ++k) {
                        buf = readFromDisk(ppp[k] * _blockSize, _blockSize);

                        n = ((size > _blockSize) ? _blockSize : size);

                        memcpy(mmap_head, buf, n);
                        mmap_head += n;
                        size -= n;

                        delete buf;
                    }

                    delete ppp;
                }

                delete pp;
            }

            delete p;
        }

        return mmap_base;
    }

    bool Ext2FS::isDirectory(File *f) {
        auto *data = static_cast<struct ext2fs::filePrivateData *>(f->privateData);

        if (!data->node)
            data->node = readInode(data->inum);

        return (data->node->mode & EXT2_DIR);
    }

    File *Ext2FS::isCachedLeaf(const File *dir, const char *filename) {
        File *leaf = dir->leaf;

        while (leaf) {
            if (strcmp(filename, leaf->name) == 0)
                return leaf;

            leaf = leaf->next;
        }

        return nullptr;
    }

    File *Ext2FS::getDirEntries(File *dir) {
        auto *data = static_cast<struct ext2fs::filePrivateData *>(dir->privateData);
        File *leaf;
        File *prevLeaf;

        bool fileToClose;

        if (!data->node)
            data->node = readInode(data->inum);

        if (!isDirectory(dir)) {
            sScreen.printError("%s isn't a directory !", dir->name);
            return nullptr;
        }

        if (!dir->content) {
            dir->content = readFile(dir);
            fileToClose = true;
        } else
            fileToClose = false;

        uint32_t dsize = data->node->size;
        auto *dentry = reinterpret_cast<struct ext2fs::directory_entry *>(dir->content);

        File *firstLeaf = prevLeaf = dir->leaf;

        while (dentry->inode && dsize) {
            const auto filename = new char[dentry->name_len + 1];
            memcpy(filename, &(dentry->name), dentry->name_len);
            filename[dentry->name_len] = 0;

            if (strcmp(".", filename) != 0 && strcmp("..", filename) != 0) {
                if (!((leaf = isCachedLeaf(dir, filename)))) {
                    auto *privData = new ext2fs::filePrivateData;

                    leaf = new File;
                    leaf->name = static_cast<char *>(kmalloc(dentry->name_len + 1));
                    strcpy(leaf->name, filename);

                    privData->inum = dentry->inode;
                    privData->node = readInode(dentry->inode);

                    leaf->size = privData->node->size;
                    leaf->content = nullptr;
                    leaf->parent = dir;
                    leaf->leaf = nullptr;
                    leaf->privateData = privData;

                    if (prevLeaf) {
                        leaf->next = prevLeaf->next;

                        if (prevLeaf->next)
                            prevLeaf->next->prev = leaf;

                        prevLeaf->next = leaf;
                        leaf->prev = prevLeaf;
                    } else {
                        leaf->next = nullptr;
                        leaf->prev = nullptr;
                        firstLeaf = leaf;
                    }
                }

                prevLeaf = leaf;
            }


            delete filename;

            dsize -= dentry->record_entry;
            dentry = reinterpret_cast<ext2fs::directory_entry *>(
                reinterpret_cast<char *>(dentry) + dentry->record_entry);
        }

        dir->leaf = firstLeaf;

        if (fileToClose) {
            delete dir->content;
            dir->content = nullptr;
        }

        return firstLeaf;
    }

    File *Ext2FS::getFile(const char *filename) {
        // TODO: Faire le reste !
        /*if(path[O] != '/')
            fp = current->pwd;
        else*/
        struct File *file = getRoot();

        const char *beg_p = filename;

        while (*beg_p == '/')
            beg_p++;
        const char *end_p = beg_p + 1;

        while (*beg_p != 0) {
            auto *data = static_cast<ext2fs::filePrivateData *>(file->privateData);

            if (!data->node)
                data->node = readInode(data->inum);

            if (!isDirectory(file))
                return nullptr;

            while (*end_p != 0 && *end_p != '/')
                end_p++;

            const auto name = new char[end_p - beg_p + 1];
            memcpy(name, beg_p, end_p - beg_p);
            name[end_p - beg_p] = 0;

            if (strcmp("..", name) == 0)
                file = file->parent;
            else if (strcmp(".", name) == 0) {
            } else {
                getDirEntries(file);

                if (!((file = isCachedLeaf(file, name)))) {
                    delete name;
                    return nullptr;
                }
            }

            beg_p = end_p;

            while (*beg_p == '/')
                beg_p++;

            end_p = beg_p + 1;

            delete name;
        }

        return file;
    }
}

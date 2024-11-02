#include <disk/FileSystems/Ext2/Ext2FS.h>
#include <utils/lib.h>
#include <memory/kmalloc.h>
#include <video/Screen.h>

bool Ext2FS::isExt2FS(char *data)
{
	if(!data)
	{
        Screen::getScreen().printError("Data NULL !");
		return false;
	}

	return (reinterpret_cast<ext2_super_block *>(data)->ext2_magic == EXT2_MAGIC_NUMBER);
}

Ext2FS::Ext2FS(const Partition &part, IdeDrive &drive) : FileSystem(part, drive)
{
    initFsRoot();
}

void Ext2FS::initFsRoot()
{
    readSuperBlock();

    _blockSize = 1024 << _sb->log_block_size;

    const int i = (_sb->block_count / _sb->blocks_per_group) +
                  ((_sb->block_count % _sb->blocks_per_group) ? 1 : 0);

    const int j = (_sb->inodes_count / _sb->inodes_per_group) +
                  ((_sb->inodes_count % _sb->inodes_per_group) ? 1 : 0);

    _groupNumber = (i > j) ? i : j;

    readGroupBlock();

    file *rootFile = getRoot();

    auto *data = static_cast<filePrivateData *>(kmalloc(sizeof(filePrivateData)));

    rootFile->name = static_cast<char *>(kmalloc(strlen("/") + 1));
    strcpy(rootFile->name, "/");

    data->inum = EXT2_INUM_ROOT;
    data->inode = readInode(EXT2_INUM_ROOT);

    rootFile->content = nullptr;
    rootFile->privateData = static_cast<void *>(data);
    rootFile->parent = rootFile;
    rootFile->leaf = getDirEntries(rootFile);
    rootFile->next = nullptr;
    rootFile->prev = nullptr;
}

void Ext2FS::readSuperBlock()
{
    _sb = reinterpret_cast<ext2_super_block *>(readFromDisk(1024, sizeof(ext2_super_block)));
}

void Ext2FS::readGroupBlock()
{
	const int offset = (_blockSize == 1024) ? 2048 : _blockSize;

	const int gd_size = _groupNumber * sizeof(struct ext2_group_desc);

    _groups = reinterpret_cast<ext2_group_desc *>(readFromDisk(offset, gd_size));
}

ext2_inode* Ext2FS::readInode(const int num) const {
	//struct ext2_inode *inode = (struct ext2_inode*)kmalloc(sizeof(struct ext2_inode));

	const int gr_num = (num - 1) / _sb->inodes_per_group;

	const int index = (num - 1) % _sb->inodes_per_group;

	const int offset = _groups[gr_num].inode_table * _blockSize + index * _sb->inode_size;

    auto *inode = reinterpret_cast<ext2_inode *>(readFromDisk(offset, _sb->inode_size));

	return inode;
}

char* Ext2FS::readFile(const char *path)
{
	file *f = nullptr;

    if((f = getFile(path)))
	{
		const auto *data = static_cast<struct filePrivateData *>(f->privateData);
        if(!data->inode)
            readInode(data->inum);
        f->size = data->inode->size;
        return readFile(f);
	}
	else
		return nullptr;
}

char* Ext2FS::readFile(file *file)
{
	char *mmap_base, *buf;

	int *p,*pp;
    int n;

	const auto *data = static_cast<filePrivateData *>(file->privateData);

    if(!data)
        return nullptr;

	const ext2_inode *inode = data->inode;

    //Screen::getScreen().printInfo("\tData inode : %p, num %u, size %u", data->inode, data->inum, inode->size);

    //buf = (char*)kmalloc(_blockSize);
    //p = (int*)kmalloc(_blockSize);
    //pp = (int*)kmalloc(_blockSize);
    //ppp = (int*)kmalloc(_blockSize);

	unsigned int size = inode->size; // Taille totale du fichier
    file->size = inode->size;
	char *mmap_head = mmap_base = static_cast<char *>(kmalloc(size));

    //Screen::getScreen().printDebug("BlockSize : %d", _blockSize);

	// Direct block number
	for(int i = 0; i < 12 && inode->block[i]; ++i)
	{
        buf = readFromDisk(inode->block[i] * _blockSize, _blockSize);

		n = ((size > _blockSize) ? _blockSize : size);

		memcpy(mmap_head, buf, n);
		mmap_head += n;
		size -= n;

        kfree(buf);
	}

	// Indirect block number
	if(inode->block[12])
	{
         p = reinterpret_cast<int *>(readFromDisk(inode->block[12] * _blockSize, _blockSize));

		for(int i = 0; i < _blockSize / 4 && p[i]; ++i)
        {
            buf = readFromDisk(p[i] * _blockSize, _blockSize);

			n = ((size > _blockSize) ? _blockSize : size);

			memcpy(mmap_head, buf, n);
			mmap_head += n;
			size -= n;

            kfree(buf);
		}

        kfree(p);
	}

	// Bi-indirect block number
	if(inode->block[13])
    {
         p = reinterpret_cast<int *>(readFromDisk(inode->block[13] * _blockSize, _blockSize));

		for(int i = 0; i < _blockSize / 4 && p[i]; ++i)
		{
             pp = reinterpret_cast<int *>(readFromDisk(p[i] * _blockSize, _blockSize));

			for(int j = 0; j < _blockSize / 4 && pp[j]; ++j)
            {
                buf = readFromDisk(pp[j] * _blockSize, _blockSize);

				n = ((size > _blockSize) ? _blockSize : size);

				memcpy(mmap_head, buf, n);
				mmap_head += n;
				size -= n;

                kfree(buf);
			}

            kfree(pp);
		}

        kfree(p);
	}

	// Tri-indirect block number
	if(inode->block[14])
    {
	    p = reinterpret_cast<int *>(readFromDisk(inode->block[14] * _blockSize, _blockSize));

		for(int i = 0; i < _blockSize / 4 && p[i]; ++i)
		{
             pp = reinterpret_cast<int *>(readFromDisk(p[i] * _blockSize, _blockSize));

			for(int j = 0; j < _blockSize / 4 && pp[j]; ++j)
            {
                int *ppp = reinterpret_cast<int *>(readFromDisk(pp[j] * _blockSize, _blockSize));

				for(int k = 0; k < _blockSize / 4 && ppp[k]; ++k)
				{
                     buf = readFromDisk(ppp[k] * _blockSize, _blockSize);

					n = ((size > _blockSize) ? _blockSize : size);

					memcpy(mmap_head, buf, n);
					mmap_head += n;
					size -= n;

                    kfree(buf);
				}

                kfree(ppp);
			}

            kfree(pp);
		}

        kfree(p);
	}

	return mmap_base;
}

bool Ext2FS::isDirectory(file *f)
{
    auto *data = static_cast<struct filePrivateData *>(f->privateData);

    if(!data->inode)
        data->inode = readInode(data->inum);

    return (data->inode->mode & EXT2_DIR);
}

file* Ext2FS::isCachedLeaf(const file *dir, const char *filename)
{
	file *leaf = dir->leaf;

    //Screen::getScreen().printDebug("CacheLeaf leaf : %p", leaf);

	while(leaf)
	{
        //Screen::getScreen().printDebug("CacheLeaf : seek %s, on %s", filename, leaf->name);
		if(strcmp(filename, leaf->name) == 0)
			return leaf;

		leaf = leaf->next;
	}

	return nullptr;
}

file* Ext2FS::getDirEntries(file *dir)
{
	auto *data = static_cast<struct filePrivateData *>(dir->privateData);
    file *leaf, *prevLeaf;

	bool fileToClose;

    if(!data->inode)
        data->inode = readInode(data->inum);


    //Screen::getScreen().printError("DirData : %d, %d", data->inode->size, data->inum);

	if(!isDirectory(dir))
	{
        Screen::getScreen().printError("%s isn't a directory !", dir->name);
		return nullptr;
	}

	if (!dir->content) {
		dir->content = readFile(dir);
		fileToClose = true;
	} else
		fileToClose = false;

    u32 dsize = data->inode->size;
    auto *dentry = reinterpret_cast<struct directory_entry *>(dir->content);

	file *firstLeaf = prevLeaf = dir->leaf;

	while(dentry->inode && dsize)
	{
		const auto filename = static_cast<char *>(kmalloc(dentry->name_len + 1));
        memcpy(filename, &(dentry->name), dentry->name_len);
        filename[dentry->name_len] = 0;

		if(strcmp(".", filename) != 0 && strcmp("..", filename) != 0)
		{
			if(!((leaf = isCachedLeaf(dir, filename))))
			{
                auto *privData = static_cast<filePrivateData *>(kmalloc(sizeof(filePrivateData)));

				leaf = static_cast<file *>(kmalloc(sizeof(file)));
				leaf->name = static_cast<char *>(kmalloc(dentry->name_len + 1));
                strcpy(leaf->name, filename);

                //Screen::getScreen().printDebug("Name : %s", filename);
                //Screen::getScreen().printDebug("Dentry : %x, %u, %c, %u, %u", dentry->file_type, dentry->inode, 'c', dentry->name_len, dentry->record_entry);

                privData->inum = dentry->inode;
                privData->inode = readInode(dentry->inode);

                leaf->size = privData->inode->size;
                leaf->content = nullptr;
				leaf->parent = dir;
				leaf->leaf = nullptr;
                leaf->privateData = privData;

				if(prevLeaf)
				{
					leaf->next = prevLeaf->next;

					if(prevLeaf->next)
						prevLeaf->next->prev = leaf;

					prevLeaf->next = leaf;
					leaf->prev = prevLeaf;
				}
				else
				{
					leaf->next = nullptr;
					leaf->prev = nullptr;
					firstLeaf = leaf;
				}
			}

			prevLeaf = leaf;
		}


        kfree(filename);

		dsize -= dentry->record_entry;
		dentry = reinterpret_cast<directory_entry *>(reinterpret_cast<char *>(dentry) + dentry->record_entry);
	}

	dir->leaf = firstLeaf;

	if(fileToClose)
	{
        kfree(dir->content);
        dir->content = nullptr;
	}

	return firstLeaf;
}

file* Ext2FS::getFile(const char *filename)
{
	// TODO: Faire le reste !
	/*if(path[O] != '/')
		fp = current->pwd;
	else*/
        struct file *file = getRoot();

	const char *beg_p = filename;

	while(*beg_p == '/')
		beg_p++;
    const char *end_p = beg_p + 1;

	while(*beg_p != 0)
	{
        auto *data = static_cast<filePrivateData *>(file->privateData);

        if(!data->inode)
            data->inode = readInode(data->inum);

		if(!isDirectory(file))
			return nullptr;

		while(*end_p != 0 && *end_p != '/')
			end_p++;

        const auto name = static_cast<char *>(kmalloc(end_p - beg_p + 1));
		memcpy(name, beg_p, end_p - beg_p);
		name[end_p - beg_p] = 0;

		if(strcmp("..", name) == 0)
			file = file->parent;
		else if(strcmp(".", name) == 0)
			{}
		else
        {
            //Screen::getScreen().printError("Filename : %s, dir : %s, inum %u", name, file->name, data->inum);
            //Screen::getScreen().printError("Data inode size : %u", data->inode->size);
            getDirEntries(file);

            //Screen::getScreen().printError("Filename : %s, dir : %s", name, file->name);

			if(!((file = isCachedLeaf(file, name))))
            {
                kfree(name);
				return nullptr;
            }
		}

		beg_p = end_p;

		while(*beg_p == '/')
			beg_p++;

        end_p = beg_p + 1;

        kfree(name);
	}

	return file;
}

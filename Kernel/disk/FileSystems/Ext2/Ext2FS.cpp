#include <disk/FileSystems/Ext2/Ext2FS.h>
#include <utils/lib.h>
#include <memory/kmalloc.h>
#include <video/Screen.h>

bool Ext2FS::isExt2FS(char *data)
{
	struct ext2_super_block *sb = (struct ext2_super_block*)data;

	if(!data)
	{
        Screen::getScreen().printError("Data NULL !");
		return false;
	}

	return (((struct ext2_super_block*)data)->ext2_magic == EXT2_MAGIC_NUMBER);
}

Ext2FS::Ext2FS(struct Partition &part, IdeDrive &drive) : FileSystem(part, drive)
{
    initFsRoot();
}

void Ext2FS::initFsRoot()
{
    readSuperBlock();

    _blockSize = 1024 << _sb->log_block_size;

    int i = (_sb->block_count / _sb->blocks_per_group) +
            ((_sb->block_count % _sb->blocks_per_group) ? 1 : 0);

    int j = (_sb->inodes_count / _sb->inodes_per_group) +
            ((_sb->inodes_count % _sb->inodes_per_group) ? 1 : 0);

    _groupNumber = (i > j) ? i : j;

    readGroupBlock();

    struct file *rootFile = getRoot();

    struct filePrivateData *data = (struct filePrivateData*)kmalloc(sizeof(struct filePrivateData));

    rootFile->name = (char*)kmalloc(strlen("/") + 1);
    strcpy(rootFile->name, "/");

    data->inum = EXT2_INUM_ROOT;
    data->inode = readInode(EXT2_INUM_ROOT);

    rootFile->content = 0;
    rootFile->privateData = (void*)data;
    rootFile->parent = rootFile;
    rootFile->leaf = getDirEntries(rootFile);
    rootFile->next = 0;
    rootFile->prev = 0;
}

void Ext2FS::readSuperBlock()
{
    _sb = (struct ext2_super_block*)readFromDisk(1024, sizeof(struct ext2_super_block));
}

void Ext2FS::readGroupBlock()
{
    int offset, gd_size;

    offset = (_blockSize == 1024) ? 2048 : _blockSize;

    gd_size = _groupNumber * sizeof(struct ext2_group_desc);

    _groups = (struct ext2_group_desc*)readFromDisk(offset, gd_size);
}

struct ext2_inode* Ext2FS::readInode(int num)
{
	int gr_num, index, offset;

    //struct ext2_inode *inode = (struct ext2_inode*)kmalloc(sizeof(struct ext2_inode));
    struct ext2_inode *inode;

    gr_num = (num - 1) / _sb->inodes_per_group;

    index = (num - 1) % _sb->inodes_per_group;

    offset = _groups[gr_num].inode_table * _blockSize + index * _sb->inode_size;

    inode = (ext2_inode*)readFromDisk(offset, _sb->inode_size);

	return inode;
}

char* Ext2FS::readFile(const char *path)
{
	struct file *f = 0;

    if((f = getFile(path)))
	{
        struct filePrivateData *data = (struct filePrivateData*)f->privateData;
        if(!data->inode)
            readInode(data->inum);
        f->size = data->inode->size;
        return readFile(f);
	}
	else
		return 0;
}

char* Ext2FS::readFile(struct file *file)
{
	char *mmap_base, *mmap_head, *buf;

	int *p,*pp, *ppp;
    int n;
    unsigned int size;

    struct filePrivateData *data = (struct filePrivateData*)file->privateData;

    if(!data)
        return 0;

    struct ext2_inode *inode = data->inode;

    //Screen::getScreen().printInfo("\tData inode : %p, num %u, size %u", data->inode, data->inum, inode->size);

    //buf = (char*)kmalloc(_blockSize);
    //p = (int*)kmalloc(_blockSize);
    //pp = (int*)kmalloc(_blockSize);
    //ppp = (int*)kmalloc(_blockSize);

	size = inode->size; // Taille totale du fichier
    file->size = inode->size;
	mmap_head = mmap_base = (char*)kmalloc(size);

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
         p = (int*)readFromDisk(inode->block[12] * _blockSize, _blockSize);

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
         p = (int*)readFromDisk(inode->block[13] * _blockSize, _blockSize);

		for(int i = 0; i < _blockSize / 4 && p[i]; ++i)
		{
             pp = (int*)readFromDisk(p[i] * _blockSize, _blockSize);

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
         p = (int*)readFromDisk(inode->block[14] * _blockSize, _blockSize);

		for(int i = 0; i < _blockSize / 4 && p[i]; ++i)
		{
             pp = (int*)readFromDisk(p[i] * _blockSize, _blockSize);

			for(int j = 0; j < _blockSize / 4 && pp[j]; ++j)
            {
                ppp = (int*)readFromDisk(pp[j] * _blockSize, _blockSize);

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

bool Ext2FS::isDirectory(struct file *f)
{
    struct filePrivateData *data = (struct filePrivateData*)f->privateData;

    if(!data->inode)
        data->inode = readInode(data->inum);

    return (data->inode->mode & EXT2_DIR);
}

struct file* Ext2FS::isCachedLeaf(struct file *dir, char *filename)
{
    struct file *leaf;

	leaf = dir->leaf;

    //Screen::getScreen().printDebug("CacheLeaf leaf : %p", leaf);

	while(leaf)
	{
        //Screen::getScreen().printDebug("CacheLeaf : seek %s, on %s", filename, leaf->name);
		if(strcmp(filename, leaf->name) == 0)
			return leaf;

		leaf = leaf->next;
	}

	return 0;
}

struct file* Ext2FS::getDirEntries(struct file *dir)
{
    struct directory_entry *dentry;
    struct filePrivateData *data = (struct filePrivateData*)dir->privateData;
    struct file *firstLeaf, *leaf, *prevLeaf;

	u32 dsize;

    char *filename;

	bool fileToClose;

    if(!data->inode)
        data->inode = readInode(data->inum);


    //Screen::getScreen().printError("DirData : %d, %d", data->inode->size, data->inum);

	if(!isDirectory(dir))
	{
        Screen::getScreen().printError("%s isn't a directory !", dir->name);
		return 0;
	}

    if(!dir->content)
	{
        dir->content = readFile(dir);
		fileToClose = true;
	}
	else
		fileToClose = false;

    dsize = data->inode->size;
    dentry = (struct directory_entry*) dir->content;

	firstLeaf = prevLeaf = dir->leaf;

	while(dentry->inode && dsize)
	{
		filename = (char*)kmalloc(dentry->name_len + 1);
        memcpy(filename, &(dentry->name), dentry->name_len);
        filename[dentry->name_len] = 0;

		if(strcmp(".", filename) && strcmp("..", filename))
		{
			if(!(leaf = isCachedLeaf(dir, filename)))
			{
                struct filePrivateData *privData = (struct filePrivateData*)kmalloc(sizeof(struct filePrivateData));

				leaf = (struct file*)kmalloc(sizeof(struct file));
				leaf->name = (char*)kmalloc(dentry->name_len + 1);
                strcpy(leaf->name, filename);

                //Screen::getScreen().printDebug("Name : %s", filename);
                //Screen::getScreen().printDebug("Dentry : %x, %u, %c, %u, %u", dentry->file_type, dentry->inode, 'c', dentry->name_len, dentry->record_entry);

                privData->inum = dentry->inode;
                privData->inode = readInode(dentry->inode);

                leaf->size = privData->inode->size;
                leaf->content = 0;
				leaf->parent = dir;
				leaf->leaf = 0;
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
					leaf->next = 0;
					leaf->prev = 0;
					firstLeaf = leaf;
				}
			}

			prevLeaf = leaf;
		}


        kfree(filename);

		dsize -= dentry->record_entry;
		dentry = (struct directory_entry*) ((char*) dentry + dentry->record_entry);
	}

	dir->leaf = firstLeaf;

	if(fileToClose)
	{
        kfree(dir->content);
        dir->content = 0;
	}

	return firstLeaf;
}

struct file* Ext2FS::getFile(const char *filename)
{
    char *name;
    const char *beg_p, *end_p;
    struct file *file;
    struct filePrivateData *data;

	// TODO: Faire le reste !
	/*if(path[O] != '/')
		fp = current->pwd;
	else*/
        file = getRoot();

	beg_p = filename;

	while(*beg_p == '/')
		beg_p++;
    end_p = beg_p + 1;

	while(*beg_p != 0)
	{
        data = (struct filePrivateData*)file->privateData;

        if(!data->inode)
            data->inode = readInode(data->inum);

		if(!isDirectory(file))
			return 0;

		while(*end_p != 0 && *end_p != '/')
			end_p++;

		name = (char*)kmalloc(end_p - beg_p + 1);
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

			if(!(file = isCachedLeaf(file, name)))
            {
                kfree(name);
				return 0;
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

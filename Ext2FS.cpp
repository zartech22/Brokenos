#include "Ext2FS.h"
#include "lib.h"
#include "kmalloc.h"
#include "strLib.h"

bool Ext2FS::isExt2FS(char *data)
{
	struct ext2_super_block *sb = (struct ext2_super_block*)data;

	Screen s;

	if(!data)
	{
		s.printError("Data NULL !");
		return false;
	}

	s.printError("Magic number %x, volume Name : %s, Os Creator %x", sb->ext2_magic, sb->volume_name, sb->inodes_count);

	return (((struct ext2_super_block*)data)->ext2_magic == EXT2_MAGIC_NUMBER);
}

Ext2FS::Ext2FS(struct Partition part, IdeDrive &drive) : _part(part), _drive(drive)
{
	readSuperBlock();

	_blockSize = 1024 << _sb.blocks_per_group;

	int i = (_sb.block_count / _sb.blocks_per_group) +
			((_sb.block_count % _sb.blocks_per_group) ? 1 : 0);

	int j = (_sb.inodes_count / _sb.inodes_per_group) +
			((_sb.inodes_count % _sb.inodes_per_group) ? 1 : 0);

	_groupNumber = (i > j) ? i : j;

	readGroupBlock();

	initRoot();
	Screen().printDebug("Root - inum : %d, name : %s", _rootFile->inum, _rootFile->name);
}

void Ext2FS::readSuperBlock()
{
    _drive.read(_part.s_lba * 512 + 1024, (char*)(&_sb), sizeof(struct ext2_super_block));
}

void Ext2FS::readGroupBlock()
{
    int offset, gd_size;

    offset = (_blockSize == 1024) ? 2048 : _blockSize;

    gd_size = _groupNumber * sizeof(struct ext2_group_desc);

    _drive.read(_part.s_lba * 512 + offset, (char*)_groups, gd_size);
}

struct ext2_inode* Ext2FS::readInode(int num)
{
	int gr_num, index, offset;

	struct ext2_inode *inode = (struct ext2_inode*)kmalloc(sizeof(struct ext2_inode));

	gr_num = (num - 1) / _sb.inodes_per_group;

	index = (num - 1) % _sb.inodes_per_group;

	offset = _groups[gr_num].inode_table * _blockSize + index * _sb.inode_size;

	_drive.read(_part.s_lba * 512 + offset, (char*)inode, _sb.inode_size);

	return inode;
}

char* Ext2FS::readFile(char *filename)
{
	struct file *f = 0;

	if((f = getFile(filename)))
	{
		if(!f->inode)
			readInode(f->inum);
		return readFile(f->inode);
	}

	else
		return 0;
}

char* Ext2FS::readFile(struct ext2_inode *inode)
{
	char *mmap_base, *mmap_head, *buf;

	int *p,*pp, *ppp;
	int n, size;

	buf = (char*)kmalloc(_blockSize);
	p = (int*)kmalloc(_blockSize);
	pp = (int*)kmalloc(_blockSize);
	ppp = (int*)kmalloc(_blockSize);

	size = inode->size; // Taille totale du fichier
	mmap_head = mmap_base = (char*)kmalloc(size);

	// Direct block number
	for(int i = 0; i < 12 && inode->block[i]; ++i)
	{
		_drive.read(_part.s_lba * 512 + inode->block[i] * _blockSize, buf, _blockSize);

		n = ((size > _blockSize) ? _blockSize : size);

		memcpy(mmap_head, buf, n);
		mmap_head += n;
		size -= n;
	}

	// Indirect block number
	if(inode->block[12])
	{
		_drive.read(_part.s_lba * 512 + inode->block[12] * _blockSize, (char*)p, _blockSize);

		for(int i = 0; i < _blockSize / 4 && p[i]; ++i)
		{
			_drive.read(_part.s_lba * 512 + p[i] * _blockSize, buf, _blockSize);

			n = ((size > _blockSize) ? _blockSize : size);

			memcpy(mmap_head, buf, n);
			mmap_head += n;
			size -= n;
		}
	}

	// Bi-indirect block number
	if(inode->block[13])
	{
		_drive.read(_part.s_lba * 512 + inode->block[13] * _blockSize, (char*)p, _blockSize);

		for(int i = 0; i < _blockSize / 4 && p[i]; ++i)
		{
			_drive.read(_part.s_lba * 512 + p[i] * _blockSize, (char*) pp, _blockSize);

			for(int j = 0; j < _blockSize / 4 && pp[j]; ++j)
			{
				_drive.read(_part.s_lba * 512 + pp[j] * _blockSize, buf, _blockSize);

				n = ((size > _blockSize) ? _blockSize : size);

				memcpy(mmap_head, buf, n);
				mmap_head += n;
				size -= n;
			}
		}
	}

	// Tri-indirect block number
	if(inode->block[14])
	{
		_drive.read(_part.s_lba * 512 + inode->block[14] * _blockSize, (char*)p, _blockSize);

		for(int i = 0; i < _blockSize / 4 && p[i]; ++i)
		{
			_drive.read(_part.s_lba * 512 + p[i] * _blockSize, (char*) pp, _blockSize);

			for(int j = 0; j < _blockSize / 4 && pp[j]; ++j)
			{
				_drive.read(_part.s_lba * 512 + pp[j] * _blockSize, (char*) ppp, _blockSize);

				for(int k = 0; k < _blockSize / 4 && ppp[k]; ++k)
				{
					_drive.read(_part.s_lba * 512 + ppp[k] * _blockSize, buf, _blockSize);

					n = ((size > _blockSize) ? _blockSize : size);

					memcpy(mmap_head, buf, n);
					mmap_head += n;
					size -= n;
				}
			}
		}
	}

	kfree(buf);
	kfree(p);
	kfree(pp);
	kfree(ppp);

	return mmap_base;
}

void Ext2FS::initRoot()
{
	_rootFile = (struct file*)kmalloc(sizeof(struct file));

	_rootFile->name = (char*)kmalloc(strlen("/"));
	strcpy(_rootFile->name, "/");
	_rootFile->name[strlen("/")] = 0;

	_rootFile->inum = EXT2_INUM_ROOT;
	_rootFile->inode = readInode(EXT2_INUM_ROOT);
	_rootFile->mmap = 0;
	_rootFile->parent = _rootFile;
	_rootFile->leaf = getDirEntries(_rootFile);
	_rootFile->next = 0;
	_rootFile->prev = 0;
}

bool Ext2FS::isDirectory(struct file *f)
{
	if(!f->inode)
		f->inode = readInode(f->inum);

	return (f->inode->mode & EXT2_DIR);
}

struct file* Ext2FS::isCachedLeaf(struct file *dir, char *filename)
{
	struct file *leaf;

	leaf = dir->leaf;

	while(leaf)
	{
		if(strcmp(filename, leaf->name) == 0)
			return leaf;

		leaf = leaf->next;
	}

	return 0;
}

struct file* Ext2FS::getDirEntries(struct file *dir)
{
	struct directory_entry *dentry;
	struct file *firstLeaf, *leaf, *prevLeaf;

	u32 dsize;

	char *filename;

	bool fileToClose;

	if(!dir->inode)
		dir->inode = readInode(dir->inum);

	if(!isDirectory(dir))
	{
		Screen().printError("%s isn't a directory !", dir->name);
		return 0;
	}

	if(!dir->mmap)
	{
		dir->mmap = readFile(dir->inode);
		fileToClose = true;
	}
	else
		fileToClose = false;

	dsize = dir->inode->size;
	dentry = (struct directory_entry*) dir->mmap;

	firstLeaf = prevLeaf = dir->leaf;

	while(dentry->inode && dsize)
	{
		filename = (char*)kmalloc(dentry->name_len + 1);
		memcpy(filename, &dentry->name, dentry->name_len);
		filename[dentry->name_len] = 0;

		if(strcmp(".", filename) && strcmp("..", filename))
		{
			if(!(leaf = isCachedLeaf(dir, filename)))
			{
				leaf = (struct file*)kmalloc(sizeof(struct file));
				leaf->name = (char*)kmalloc(dentry->name_len + 1);
				strcpy(leaf->name, filename);

				leaf->inum = dentry->inode;
				leaf->inode = 0;
				leaf->mmap = 0;
				leaf->parent = dir;
				leaf->leaf = 0;

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
		kfree(dir->mmap);
		dir->mmap = 0;
	}

	return firstLeaf;
}

struct file* Ext2FS::getFile(char *filename)
{
	char *name, *beg_p, *end_p;
	struct file *file;

	// TODO: Faire le reste !
	/*if(path[O] != '/')
		fp = current->pwd;
	else*/
		file = _rootFile;

	beg_p = filename;

	while(*beg_p == '/')
		beg_p++;
	end_p = beg_p + 1;

	while(*beg_p != 0)
	{
		if(!file->inode)
			file->inode = readInode(file->inum);

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
			getDirEntries(file);

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
	}

	return file;
}

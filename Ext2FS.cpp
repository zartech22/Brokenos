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
}

void Ext2FS::readSuperBlock()
{
    _drive.read(1024, (char*)(&_sb), sizeof(struct ext2_super_block));
}

void Ext2FS::readGroupBlock()
{
    int offset, gd_size;

    offset = (_blockSize == 1024) ? 2048 : _blockSize;

    gd_size = _groupNumber * sizeof(struct ext2_group_desc);

    _drive.read(offset, (char*)_groups, gd_size);
}

struct ext2_inode* Ext2FS::readInode(int num)
{
	int gr_num, index, offset;

	struct ext2_inode *inode = (struct ext2_inode*)kmalloc(sizeof(struct ext2_inode));

	gr_num = (num - 1) / _sb.inodes_per_group;

	index = (num - 1) % _sb.inodes_per_group;

	offset = _groups[gr_num].inode_table * _blockSize + index * _sb.inode_size;

	_drive.read(offset, (char*)inode, _sb.inode_size);

	return inode;
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
		_drive.read(inode->block[i] * _blockSize, buf, _blockSize);

		n = ((size > _blockSize) ? _blockSize : size);

		memcpy(mmap_head, buf, n);
		mmap_head += n;
		size -= n;
	}

	// Indirect block number
	if(inode->block[12])
	{
		_drive.read(inode->block[12] * _blockSize, (char*)p, _blockSize);

		for(int i = 0; i < _blockSize / 4 && p[i]; ++i)
		{
			_drive.read(p[i] * _blockSize, buf, _blockSize);

			n = ((size > _blockSize) ? _blockSize : size);

			memcpy(mmap_head, buf, n);
			mmap_head += n;
			size -= n;
		}
	}

	// Bi-indirect block number
	if(inode->block[13])
	{
		_drive.read(inode->block[13] * _blockSize, (char*)p, _blockSize);

		for(int i = 0; i < _blockSize / 4 && p[i]; ++i)
		{
			_drive.read(p[i] * _blockSize, (char*) pp, _blockSize);

			for(int j = 0; j < _blockSize / 4 && pp[j]; ++j)
			{
				_drive.read(pp[j] * _blockSize, buf, _blockSize);

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
		_drive.read(inode->block[14] * _blockSize, (char*)p, _blockSize);

		for(int i = 0; i < _blockSize / 4 && p[i]; ++i)
		{
			_drive.read(p[i] * _blockSize, (char*) pp, _blockSize);

			for(int j = 0; j < _blockSize / 4 && pp[j]; ++j)
			{
				_drive.read(pp[j] * _blockSize, (char*) ppp, _blockSize);

				for(int k = 0; k < _blockSize / 4 && ppp[k]; ++k)
				{
					_drive.read(ppp[k] * _blockSize, buf, _blockSize);

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

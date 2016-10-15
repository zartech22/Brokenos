#ifndef EXT2FS_H
#define EXT2FS_H

#include <disk/ide.h>
#include <utils/types.h>
#include <disk/FileSystem.h>

/* ERROR TYPES */
#define	EXT2_ERRORS_CONTINUE	1
#define	EXT2_ERRORS_RO			2
#define	EXT2_ERRORS_PANIC		3
#define	EXT2_ERRORS_DEFAULT		1

/* INODE MODES */
#define	EXT2_FORMAT		0xF000
#define	EXT2_SOCKET		0xC000
#define	EXT2_LINK		0xA000
#define	EXT2_REGULAR	0x8000
#define	EXT2_BLOCK		0x6000
#define	EXT2_DIR		0x4000
#define	EXT2_CHAR		0x2000
#define	EXT2_FIFO		0x1000

#define	EXT2_SUID		0x0800
#define	EXT2_SGIF		0x0400
#define	EXT2_STICKY		0x0200
#define	EXT2_RWX_U		0x01C0
#define	EXT2_R_U		0x0100
#define	EXT2_W_U		0x0080
#define	EXT2_X_U		0x0040
#define	EXT2_RWX_G		0x0038
#define	EXT2_R_G		0x0020
#define	EXT2_W_G		0x0010
#define	EXT2_X_G		0x0008
#define	EXT2_RWX_O		0x0007
#define	EXT2_R_O		0x0004
#define	EXT2_W_O		0x0002
#define	EXT2_X_O		0x0001

#define	EXT2_MAGIC_NUMBER	0xEF53
#define	EXT2_INUM_ROOT		2

struct ext2_super_block
{
    u32     inodes_count;
    u32     block_count;
    u32     reserved_block_count;
    u32     free_block_count;
    u32     free_inodes_count;
    u32     first_data_block;

    u32     log_block_size;
    u32     log_fragment_size;
    u32     blocks_per_group;
    u32     frags_per_group;
    u32     inodes_per_group;
    u32     mount_time; // Last time has been mounted
    u32     write_time; // Last time has been written

    u16     mount_count; // How many mount since last full check
    u16     max_mount_count;
    u16     ext2_magic;
    u16		state;
    u16		errors_behaviour;
    u16		minor_rev_level;

    u32		last_check;
    u32		checkInterval;
    u32		creator_os;
    u32		rev_level;

    u16		def_reserved_uid;
    u16		de_reserved_gid;

    u32		first_inode;

    u16		inode_size;
    u16		block_group_nr;

    u32		feature_compat;
    u32		feature_incompat;
    u32		feature_ro_compat;

	u8		uuid[16];
	char	volume_name[16];
	char	last_mounted[64];
	u32		algo_bitmap;
	u8		padding[820];
} __attribute__((packed));

struct ext2_group_desc
{
	u32		block_bitmap;
	u32		inode_bitmap;
	u32		inode_table;

	u16		free_blocks_count;
	u16		free_inodes_count;
	u16		used_dirs_count;
	u16		padding;

	u32		reserved[3];
} __attribute__((packed));

struct ext2_inode
{
	u16		mode;
	u16		uid;

	u32		size;
	u32		atime;
	u32		ctime;
	u32		mtime;
	u32		dtime;

	u16		gid;
	u16		links_count;

	u32		blocks;
	u32		flags;
	u32		osd1;

	u32		block[15];
	u32		generation;
	u32		file_acl;
	u32		dir_acl;
	u32		faddr;

	u8		osd2[12];
} __attribute__((packed));

struct directory_entry
{
    u32		inode;
    u16		record_entry;
    u8		name_len;
    u8		file_type;

    char	name;
} __attribute__((packed));

struct filePrivateData
{
	u32					inum;
	struct ext2_inode*	inode;
};

struct open_file
{
    struct file*		file;
	u32					ptr;
	struct open_file*	next;
};


class Ext2FS : public FileSystem
{
    public:
        Ext2FS(struct Partition&, IdeDrive&);
        ~Ext2FS() { Screen::getScreen().printError("Fin Ext2FS"); }

        Ext2FS(const Ext2FS &o) : FileSystem(*this) { Screen::getScreen().printError("Copie"); }

        virtual char* readFile(const char *path) override;
        virtual char* readFile(struct file*) override;

        virtual struct file* getFile(const char *name) override;

        struct ext2_inode* readInode(int num);

        virtual bool isDirectory(const char *path) override { return isDirectory(getFile(path)); }
        virtual bool isDirectory(struct file *f) override;

        virtual struct file* getDirEntries(struct file*) override;
        virtual struct file* getDirEntries(const char *path) override { return getDirEntries(getFile(path)); }

        static bool isExt2FS(char *data);

    private:
		u32 _blockSize;
		u16	_groupNumber;

        struct ext2_super_block *_sb;
		struct ext2_group_desc *_groups;

        inline void readSuperBlock();
		void readGroupBlock();

        virtual void initFsRoot() override;

        struct file* isCachedLeaf(struct file*, char*);

};

#endif // EXT2FS_H

#pragma once

#include <utils/types.h>
#include <disk/FileSystems/FileSystem.h>

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
    uint32_t     inodes_count;
    uint32_t     block_count;
    uint32_t     reserved_block_count;
    uint32_t     free_block_count;
    uint32_t     free_inodes_count;
    uint32_t     first_data_block;

    uint32_t     log_block_size;
    uint32_t     log_fragment_size;
    uint32_t     blocks_per_group;
    uint32_t     frags_per_group;
    uint32_t     inodes_per_group;
    uint32_t     mount_time; // Last time has been mounted
    uint32_t     write_time; // Last time has been written

    uint16_t     mount_count; // How many mount since last full check
    uint16_t     max_mount_count;
    uint16_t     ext2_magic;
    uint16_t		state;
    uint16_t		errors_behaviour;
    uint16_t		minor_rev_level;

    uint32_t		last_check;
    uint32_t		checkInterval;
    uint32_t		creator_os;
    uint32_t		rev_level;

    uint16_t		def_reserved_uid;
    uint16_t		de_reserved_gid;

    uint32_t		first_inode;

    uint16_t		inode_size;
    uint16_t		block_group_nr;

    uint32_t		feature_compat;
    uint32_t		feature_incompat;
    uint32_t		feature_ro_compat;

	uint8_t		uuid[16];
	char	volume_name[16];
	char	last_mounted[64];
	uint32_t		algo_bitmap;
	uint8_t		padding[820];
} __attribute__((packed));

struct ext2_group_desc
{
	uint32_t		block_bitmap;
	uint32_t		inode_bitmap;
	uint32_t		inode_table;

	uint16_t		free_blocks_count;
	uint16_t		free_inodes_count;
	uint16_t		used_dirs_count;
	uint16_t		padding;

	uint32_t		reserved[3];
} __attribute__((packed));

struct ext2_inode
{
	uint16_t		mode;
	uint16_t		uid;

	uint32_t		size;
	uint32_t		atime;
	uint32_t		ctime;
	uint32_t		mtime;
	uint32_t		dtime;

	uint16_t		gid;
	uint16_t		links_count;

	uint32_t		blocks;
	uint32_t		flags;
	uint32_t		osd1;

	uint32_t		block[15];
	uint32_t		generation;
	uint32_t		file_acl;
	uint32_t		dir_acl;
	uint32_t		faddr;

	uint8_t		osd2[12];
} __attribute__((packed));

struct directory_entry
{
    uint32_t		inode;
    uint16_t		record_entry;
    uint8_t		name_len;
    uint8_t		file_type;

    char	name;
} __attribute__((packed));

struct filePrivateData
{
	uint32_t					inum;
	ext2_inode*	inode;
};

struct open_file
{
    struct file*		file;
	uint32_t					ptr;
	open_file*	next;
};


class Ext2FS final : public FileSystem
{
    public:
        Ext2FS(const Partition&, IdeDrive&);
        ~Ext2FS() override { Screen::getScreen().printError("Fin Ext2FS"); }

        explicit Ext2FS(const Ext2FS &o) : FileSystem(*this) { Screen::getScreen().printError("Copie"); }

        char* readFile(const char *path) override;
        char* readFile(file*) override;

        file* getFile(const char *name) override;

        ext2_inode* readInode(int num) const;

        bool isDirectory(const char *path) override { return isDirectory(getFile(path)); }
        bool isDirectory(file *f) override;

        file* getDirEntries(file*) override;
        file* getDirEntries(const char *path) override { return getDirEntries(getFile(path)); }

        static bool isExt2FS(char *data);

        static Ext2FS* initializeFS(Partition &part, IdeDrive &drive)
        {
            return new Ext2FS(part, drive);
        }

    private:
		uint32_t _blockSize;
		uint16_t	_groupNumber;

        ext2_super_block *_sb;
		ext2_group_desc *_groups;

        inline void readSuperBlock();
		void readGroupBlock();

        void initFsRoot() override;

        static file* isCachedLeaf(const file*, const char*);

};

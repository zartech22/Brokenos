#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "kmalloc.h"
#include "types.h"
#include "ide.h"
#include "lib.h"

/*namespace FileSystem
{*/
    struct file
    {
        char*				name;
        char*				content;
        bool				isOpen;
        void*               privateData;
        struct file*		parent;
        struct file*		leaf;
        struct file*		next;
        struct file*		prev;
    };

    class FileSystem
    {
    public:
        FileSystem(struct Partition &part, IdeDrive &drive) : _part(part), _drive(drive), _fsRoot(new file) { memset((char*)_fsRoot, 0, sizeof(struct file)); }
        virtual ~FileSystem()
        {
            if(_fsRoot->content != 0)
                kfree(_fsRoot->content);
            if(_fsRoot->name != 0)
                kfree(_fsRoot->name);
            if(_fsRoot->privateData != 0)
                kfree(_fsRoot->privateData);

            delete _fsRoot;
        }

        virtual void initFsRoot() = 0;

        struct Partition& getPartition() const { return _part; }
        IdeDrive& getDrive() const { return _drive; }
        struct file* getRoot() const { return _fsRoot; }

        virtual char* readFile(const char *path) = 0;
        virtual struct file* getFile(const char *path) = 0;

        virtual bool isDirectory(struct file *file) = 0;
        virtual bool isDirectory(const char *path) = 0;

        virtual struct file* getDirEntries(const char *path) = 0;
        virtual struct file* getDirEntries(struct file*) = 0;
    protected:
        char* readFromDisk(int offset, int bytes)
        {
            char *data = (char*)kmalloc(bytes);
            _drive.read(_part.s_lba * 512 + offset, data, bytes);
            return data;
        }
    private:
        struct Partition &_part;
        IdeDrive &_drive;
        struct file *_fsRoot;
    };

//}

#endif // FILESYSTEM_H

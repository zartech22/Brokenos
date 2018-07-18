#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <memory/kmalloc.h>
#include <utils/types.h>
#include <disk/IDE/IdeDrive.h>
#include <utils/lib.h>
#include <utils/Vector.h>

/*namespace FileSystem
{*/
    struct file
    {
        char*				name;
        char*				content;
        bool				isOpen;
        u32                 size;
        void*               privateData;
        struct file*		parent;
        struct file*		leaf;
        struct file*		next;
        struct file*		prev;
    };

    class FileSystem
    {
    public:
        FileSystem(const struct Partition &part, IdeDrive &drive) : _part(part), _drive(drive)
        {
            _fsRoot = new struct file;
            memset((char*)_fsRoot, 0, sizeof(struct file));

            if(!_fsList)
                _fsList = new Vector<FileSystem*>;

            _fsList->push_back(this);
        }

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

        static const Vector<FileSystem*>& getFsList() { return *_fsList; }

        virtual void initFsRoot() = 0;

        const struct Partition& getPartition() const { return _part; }
        IdeDrive& getDrive() const { return _drive; }
        struct file* getRoot() const { return _fsRoot; }

        virtual char* readFile(const char *path) = 0;
        virtual char* readFile(struct file*) = 0;
        virtual struct file* getFile(const char *path) = 0;

        virtual bool isDirectory(struct file *file) = 0;
        virtual bool isDirectory(const char *path) = 0;

        virtual struct file* getDirEntries(const char *path) = 0;
        virtual struct file* getDirEntries(struct file*) = 0;
    protected:
        char* readFromDisk(int offset, int bytes)
        {
            char *data = (char*)kmalloc(bytes);
            memset(data, 0, bytes);
            _drive.read(_part.s_lba * 512 + offset, data, bytes);
            return data;
        }
    private:
        const struct Partition _part;
        IdeDrive &_drive;
        struct file *_fsRoot;

        static Vector<FileSystem*> *_fsList;

    };
//}

#endif // FILESYSTEM_H

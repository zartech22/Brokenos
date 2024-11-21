#pragma once

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
        uint32_t                 size;
        void*               privateData;
        file*		parent;
        file*		leaf;
        file*		next;
        file*		prev;
    };

    class FileSystem
    {
    public:
        FileSystem(const Partition &part, IdeDrive &drive) : _part(part), _drive(drive)
        {
            _fsRoot = new file;
            memset(reinterpret_cast<char *>(_fsRoot), 0, sizeof(file));

            if(!_fsList)
                _fsList = new Vector<FileSystem*>;

            _fsList->push_back(this);
        }

        virtual ~FileSystem()
        {
            if(_fsRoot->content != nullptr)
                kfree(_fsRoot->content);
            if(_fsRoot->name != nullptr)
                kfree(_fsRoot->name);
            if(_fsRoot->privateData != nullptr)
                kfree(_fsRoot->privateData);

            delete _fsRoot;
        }

        static const Vector<FileSystem*>& getFsList() { return *_fsList; }

        virtual void initFsRoot() = 0;

        [[nodiscard]] const Partition& getPartition() const { return _part; }
        [[nodiscard]] IdeDrive& getDrive() const { return _drive; }
        [[nodiscard]] file* getRoot() const { return _fsRoot; }

        virtual char* readFile(const char *path) = 0;
        virtual char* readFile(file*) = 0;
        virtual file* getFile(const char *path) = 0;

        virtual bool isDirectory(struct file *file) = 0;
        virtual bool isDirectory(const char *path) = 0;

        virtual file* getDirEntries(const char *path) = 0;
        virtual file* getDirEntries(file*) = 0;
    protected:
        [[nodiscard]] char* readFromDisk(const int offset, const int bytes) const {
            const auto data = new char[bytes];
            memset(data, 0, bytes);
            _drive.read(_part.s_lba * 512 + offset, data, bytes);
            return data;
        }
    private:
        const Partition _part;
        IdeDrive &_drive;
        file *_fsRoot;

        static Vector<FileSystem*> *_fsList;

    };
//}

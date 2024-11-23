#pragma once

#include <disk/IDE/IdeDrive.h>
#include <utils/lib.h>
#include <utils/Vector.h>

namespace kernel::disk::fileSystems
{
    struct File
    {
        char*       name;
        char*       content;
        bool        isOpen;
        uint32_t    size;
        void*       privateData;
        File*       parent;
        File*       leaf;
        File*       next;
        File*       prev;
    };

    class FileSystem
    {
    public:
        FileSystem(const Partition &part, IdeDrive &drive) : _part(part), _drive(drive)
        {
            _fsRoot = new File;
            memset(_fsRoot, 0, sizeof(File));

            if(!_fsList)
                _fsList = new Vector<FileSystem*>;

            _fsList->push_back(this);
        }

        virtual ~FileSystem()
        {
            delete _fsRoot->content;
            delete _fsRoot->name;

            if(_fsRoot->privateData != nullptr)
                delete _fsRoot->privateData;

            delete _fsRoot;
        }

        static const Vector<FileSystem*>& getFsList() { return *_fsList; }

        virtual void initFsRoot() = 0;

        [[nodiscard]] const Partition& getPartition() const { return _part; }
        [[nodiscard]] IdeDrive& getDrive() const { return _drive; }
        [[nodiscard]] File* getRoot() const { return _fsRoot; }

        virtual char* readFile(const char *path) = 0;
        virtual char* readFile(File*) = 0;
        virtual File* getFile(const char *path) = 0;

        virtual bool isDirectory(File *file) = 0;
        virtual bool isDirectory(const char *path) = 0;

        virtual File* getDirEntries(const char *path) = 0;
        virtual File* getDirEntries(File*) = 0;
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
        File *_fsRoot;

        static Vector<FileSystem*> *_fsList;

    };
}

#include <disk/FileSystems/FileSystem.h>

namespace kernel::disk::fileSystems {
    Vector<FileSystem*>* FileSystem::_fsList = nullptr;
}

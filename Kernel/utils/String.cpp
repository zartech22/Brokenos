#include <utils/String.h>

#include "memory/kmalloc.h"

String& String::operator =(const String &o)
{
    if(&o != this) {
        _str = static_cast<char *>(krealloc(_str, o.size() + 1));
        strcpy(_str, o._str);
    }

    return *this;
}

String& String::operator +=(const String &o)
{
    _str = static_cast<char *>(krealloc(_str, size() + o.size() + 1));
    memcpy((_str + size()), o._str, o.size() + 1);

    return *this;
}

String& String::append(const String &o)
{
    return (*this) += o;
}

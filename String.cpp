#include "String.h"

String& String::operator =(const String &o)
{
    delete[] _str;
    _str = new char[o.size() + 1];
    strcpy(_str, o._str);

    return *this;
}

String& String::operator +=(const String &o)
{
    char *tmp = new char[size() + o.size() + 1];
    memcpy(tmp, _str, size());
    memcpy(tmp + size(), o._str, o.size() + 1);

    delete[] _str;
    _str = tmp;

    return *this;
}

String& String::append(const String &o)
{
    return (*this) += o;
}

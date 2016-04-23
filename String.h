#ifndef STRING_H
#define STRING_H

#include "types.h"
#include "Screen.h"
#include "kmalloc.h"
#include "lib.h"

class String
{
public:
    String(const char *str) : _str(new char[strlen(str)]) { strcpy(_str, str); }
    String(const char c) : _str(new char[2]) { _str[0] = c; _str[1] = 0;  }
    String(const String &o) : String(o._str) {}
    virtual ~String() { delete[] _str; }

    size_t size() const { return strlen(_str); }

    void clear() { _str[0] = 0; }
    bool empty() const { return (strlen(_str) == 0); }

    String& append(const String &o);

    String& operator=(const String &o);

    String& operator+=(const String &o);
    char& operator[](size_t pos) { return _str[pos]; }
    const char& operator[](size_t pos) const { return _str[pos]; }

    char& at(size_t pos) { return _str[pos]; }
    const char& at(size_t pos) const { return _str[pos]; }

    const char* c_str() const { return _str; }

private:
    char *_str;
};

#endif // STRING_H

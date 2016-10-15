#ifndef STRING_H
#define STRING_H

#include <utils/types.h>
#include <video/Screen.h>
#include <memory/kmalloc.h>
#include <utils/lib.h>

class String
{
public:
    String(const char *str) { _str = new char[strlen(str) + 1]; strcpy(_str, str); }
    String(const char c) : _str(new char[2]) { _str[0] = c; _str[1] = 0;  }
    String(const String &o) : String(o._str) {}
    virtual ~String() { delete _str; }

    inline size_t size() const { return strlen(_str); }

    inline void clear() { _str[0] = 0; }
    inline bool empty() const { return (strlen(_str) == 0); }

    String& append(const String &o);

    String& operator=(const String &o);

    String& operator+=(const String &o);
    inline char& operator[](size_t pos) { return _str[pos]; }
    inline const char& operator[](size_t pos) const { return _str[pos]; }

    inline char& at(size_t pos) { return _str[pos]; }
    inline const char& at(size_t pos) const { return _str[pos]; }

    inline const char* c_str() const { return _str; }

private:
    char *_str;
};

#endif // STRING_H

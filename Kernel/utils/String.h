#pragma once

#include <utils/lib.h>
#include <cstddef>

class String final {
public:
    explicit String(const char * const str) { _str = new char[strlen(str) + 1]; strcpy(_str, str); }
    explicit String(const char c) : _str(new char[2]) { _str[0] = c; _str[1] = 0;  }
    String(const String &o) : String(o._str) {}
    ~String() { delete _str; }

    [[nodiscard]] size_t size() const { return strlen(_str); }

    void clear() const { _str[0] = 0; }
    [[nodiscard]] bool empty() const { return (strlen(_str) == 0); }

    String& append(const String &o);

    String& operator=(const String &o);

    String& operator+=(const String &o);
    char& operator[](const size_t pos) { return _str[pos]; }
    const char& operator[](const size_t pos) const { return _str[pos]; }

    char& at(const size_t pos) { return _str[pos]; }
    [[nodiscard]] const char& at(const size_t pos) const { return _str[pos]; }

    [[nodiscard]] const char* c_str() const { return _str; }

private:
    char *_str;
};

#ifndef VECTOR_H
#define VECTOR_H

#include <memory/kmalloc.h>

#include <video/Screen.h>

template<typename T>
struct is_pointer { static const bool value = false; };

template<typename T>
struct is_pointer<T*> { static const bool value = true; };

template<typename T, bool destroyPtr = true>
class Vector
{
public:
    Vector() : _size(0), _cap(10), _tab(new T[10]) {}
    Vector(unsigned int n, T val = T()) : _size(0), _cap(n), _tab(new T[_cap]) { for(u8 i = 0; i < n; ++i) push_back(val); }
    Vector(const Vector &x) {}

    ~Vector() { clear(); }

    unsigned int size() const { return _size; }
    unsigned int capacity() const { return _cap; }

    bool empty() const { return (size == 0); }

    T& at(unsigned int n) { if(n >= _cap) Screen::getScreen().printError("Erreur %s : %u >= %u", __func__, n, _cap); return _tab[n]; }
    const T& at(unsigned int n) const { if(n >= _cap) Screen::getScreen().printError("Erreur %s : %u >= %u", __func__, n, _cap); return _tab[n]; }

    T& operator[](unsigned int n) { if(n >= _cap) Screen::getScreen().printError("Erreur Cap"); return _tab[n]; }
    const T& operator[](unsigned int n) const { if(n >= _cap) Screen::getScreen().printError("Erreur Cap"); return _tab[n]; }

    T& front() { if(empty()) asm("hlt"); return _tab[0]; }
    const T& front() const { if(empty()) asm("hlt"); return _tab[0]; }

    T& back() { if(empty()) asm("hlt"); return _tab[size]; }
    const T& back() const { if(empty()) asm("hlt"); return _tab[size]; }

    T* data() { return _tab; }
    const T* data() const { return _tab; }

    void push_back(const T &val)
    {
        if((_size + 1) == _cap)
        {
            _cap *= 2;
            _tab = (T*)krealloc(_tab, sizeof(T) * _cap);
        }

        _tab[_size] = val;
        _size++;
    }

    void pop_back() { if(is_pointer<T>::value && destroyPtr) delete _tab[size() - 1]; _size--; }
    void clear() { if(is_pointer<T>::value && destroyPtr) for(unsigned int i = 0; i < size(); ++i) delete _tab[i]; _size = 0; }

private:
    T *_tab;
    unsigned int _cap;
    unsigned int _size;
};

#endif // VECTOR_H

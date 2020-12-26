#ifndef VECTOR_H
#define VECTOR_H

#include <memory/kmalloc.h>

#include <video/Screen.h>
#include <utils/types.h>

template<typename T>
struct is_pointer { static const bool value = false; };

template<typename T>
struct is_pointer<T*> { static const bool value = true; };

template<typename T, bool destroyPtr = true>
class Vector
{
public:
    Vector() : _size(0), _cap(10), _tab(new T[10]) {}

    explicit Vector(unsigned int n, T val = T()) : _size(0)
    {
        _cap = (n != 0) ? n : 1;
        _tab = new T[_cap];

        for(unsigned int i = 0; i < n; ++i)
            push_back(val);
    }

    //TODO: Finish copy construcor...
    explicit Vector(const Vector &x) {}

    ~Vector() { clear(); delete[] _tab; }

    unsigned int size() const { return _size; }
    unsigned int capacity() const { return _cap; }

    bool empty() const { return (_size == 0); }

    T& at(unsigned int n)
    {
        if(n >= _cap)
        {
            sScreen.printError("Erreur %s : %u >= %u", __func__, n, _cap);
            asm("hlt");
        }

        return _tab[n];
    }
    const T& at(unsigned int n) const
    {
        if(n >= _cap)
        {
            sScreen.printError("Erreur %s : %u >= %u", __func__, n, _cap);
            asm("hlt");
        }

        return _tab[n];
    }

    T& operator[](unsigned int n)
    {
        if(n >= _cap)
        {
            sScreen.printError("Erreur %s : %u >= %u", __func__, n, _cap);
            asm("hlt");
        }

        return _tab[n];
    }
    const T& operator[](unsigned int n) const
    {
        if(n >= _cap)
        {
            sScreen.printError("Erreur %s : %u >= %u", __func__, n, _cap);
            asm("hlt");
        }

        return _tab[n];
    }

    T& front() { if(empty()) asm("hlt"); return _tab[0]; }
    const T& front() const { if(empty()) asm("hlt"); return _tab[0]; }

    T& back() { if(empty()) asm("hlt"); return _tab[_size - 1]; }
    const T& back() const { if(empty()) asm("hlt"); return _tab[_size - 1]; }

    T* data() { return _tab; }
    const T* data() const { return _tab; }

    void push_back(const T &val)
    {
        if((_size + 1) > _cap)
            increaseSize();

        _tab[_size] = val;
        _size++;
    }

    void pop_back()
    {
        if(destroyPtr && is_pointer<T>::value)
            delete _tab[_size - 1];

        _size--;
    }

    void clear()
    {
        if(destroyPtr && is_pointer<T>::value)
            for(unsigned int i = 0; i < _size; ++i)
                delete _tab[i];

        _size = 0;
    }

private:
    T *_tab;
    unsigned int _cap;
    unsigned int _size;

    void increaseSize()
    {
        _cap *= 2;
        _tab = (T*)krealloc(_tab, sizeof(T) * _cap);
    }
};

#endif // VECTOR_H

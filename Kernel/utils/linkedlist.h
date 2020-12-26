#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <utils/types.h>
#include <memory/kmalloc.h>

template<typename T>
class LinkedList
{
public:
    LinkedList() : _head(nullptr), _tail(nullptr), _size(0), _default() {}
    //TODO: handle copy
    LinkedList(const LinkedList<T> &o) : _size(0) {}

    ~LinkedList() { clear(); }

    void push_front(T element)
    {
        if(empty())
            insertFirstElement(element);
        else
        {
            auto nextHead = new struct Element;
            nextHead->elem = element;
            nextHead->prev = nullptr;
            nextHead->next = _head;

            _head->prev = nextHead;

            _head = nextHead;
        }

        ++_size;
    }

    void push_back(T element)
    {
        if(empty())
            insertFirstElement(element);
        else
        {
            auto nextTail = new struct Element;
            nextTail->elem = element;
            nextTail->prev = _tail;
            nextTail->next = nullptr;

            _tail->next = nextTail;

            _tail = nextTail;
        }

        ++_size;
    }

    void insert(T element, size_t n)
    {
        if(n < _size)
        {
            auto current = _head;

            for(size_t i = 0; i != n; ++i)
                current = current->next;

            auto inserted = new struct Element;

            inserted->elem = element;
            inserted->prev = current->prev;
            inserted->next = current;

            if(current->prev != nullptr)
                current->prev->next = inserted;

            ++_size;
        }
    }

    void pop_front()
    {
        if(!empty())
        {
            auto front = _head;

            if(front->next != nullptr)
            {
                _head = front->next;
                _head->prev = nullptr;
            }
            else
                _head = _tail = nullptr;

            delete front;
            --_size;
        }
    }

    void pop_back()
    {
        if(!empty())
        {
            auto back = _tail;

            if(back->prev != nullptr)
            {
                _tail = back->prev;
                _tail->next = nullptr;
            }
            else
                _head = _tail = nullptr;

            delete back;
            --_size;
        }
    }

    bool erase(size_t n)
    {
        if(n < _size)
        {
            auto toDelete = _head;

            for(size_t i = 0; i != n; ++i)
                toDelete = toDelete->next;

            if(toDelete->prev != nullptr)
                toDelete->prev->next = toDelete->next;

            if(toDelete->next != nullptr)
                toDelete->next->prev = toDelete->prev;

            delete toDelete;
            --_size;

            if(empty())
                _head = _tail = nullptr;

            return true;
        }
        else
            return false;
    }

    void clear()
    {
        if(!empty())
        {
            auto current = _head;

            while(current->next != nullptr)
                if(current->prev != nullptr)
                    delete current->prev;

            delete current;

            _size = 0;
        }
    }

    T& at(size_t n)
    {
        if(n < _size)
        {
            auto current = _head;

            for(size_t i = 0; i != n; ++i)
                current = current->next;

            return current->elem;
        }
        else
            return _default;
    }

    const T& at(size_t n) const
    {
        if(n < _size)
        {
            auto current = _head;

            for(size_t i = 0; i != n; ++i)
                current = current->next;

            return current->elem;
        }
        else
            return _default;
    }

    T& front()
    {
        return (!empty()) ? _head->elem : T();
    }

    const T& front() const
    {
        return (!empty()) ? _head->elem : T();
    }

    T& back()
    {
        return (!empty()) ? _tail->elem : T();
    }

    const T& back() const
    {
        return (!empty()) ? _tail->elem : T();
    }


    size_t size() const { return _size; }
    bool empty() const { return _size == 0; }


    //TODO: handle copy
    LinkedList<T>& operator=(const LinkedList<T>& other) {}
    T& operator[](size_t n) { return at(n); }
    const T& operator[](size_t n) const { return at(n); }

protected:
    void insertFirstElement(T element)
    {
        auto elem = new struct Element;

        elem->next = nullptr;
        elem->prev = nullptr;

        elem->elem = element;

        _head = _tail = elem;
    }

private:
    struct Element
    {
        T elem;
        struct Element *prev;
        struct Element *next;
    };

    struct Element *_head;
    struct Element *_tail;

    size_t _size;
    T _default;

};

#endif // LINKEDLIST_H

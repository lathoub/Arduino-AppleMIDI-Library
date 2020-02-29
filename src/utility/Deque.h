#pragma once

template<typename T, size_t Size>
class Deque {
//    class iterator;

private:
    int _head, _tail;
    T _data[Size];
    
public:
    Deque()
    {
        clear();
    };

    size_t free();
    const size_t size() const;
    const size_t max_size() const;
    T & front();
    const T & front() const;
    T & back();
    const T & back() const;
    void push_front(const T &);
    void push_back(const T &);
    T pop_front();
    T pop_back();
    
    T& operator[](size_t);
    const T& operator[](size_t) const;
    T& at(size_t);
    const T& at(size_t) const;

    void clear();
        
//    iterator begin();
//    iterator end();

    void erase(size_t);
    void erase(size_t, size_t);

    bool empty() const {
        return size() == 0;
    }
    bool full() const {
        return (size() == Size);
    }
};

template<typename T, size_t Size>
size_t Deque<T, Size>::free()
{
    return Size - size();
}

template<typename T, size_t Size>
const size_t Deque<T, Size>::size() const
{
  if (_tail < 0)
      return 0; // empty
  else if (_head > _tail)
      return _head - _tail;
  else
      return Size - _tail + _head;
}

template<typename T, size_t Size>
const size_t Deque<T, Size>::max_size() const
{
  return Size;
}

template<typename T, size_t Size>
T & Deque<T, Size>::front()
{
  return _data[_tail];
}

template<typename T, size_t Size>
const T & Deque<T, Size>::front() const
{
  return _data[_tail];
}

template<typename T, size_t Size>
T & Deque<T, Size>::back()
{
    int idx = _head - 1;
    if (idx < 0) idx = Size - 1;
    return _data[idx];
}

template<typename T, size_t Size>
const T & Deque<T, Size>::back() const
{
    int idx = _head - 1;
    if (idx < 0) idx = Size - 1;
    return _data[idx];
}

template<typename T, size_t Size>
void Deque<T, Size>::push_front(const T &value)
{
    //if container is full, do nothing.
    if (free()){
        if (--_tail < 0)
            _tail = Size - 1;
        _data[_tail] = value;
    }
}

template<typename T, size_t Size>
void Deque<T, Size>::push_back(const T &value)
{
    //if container is full, do nothing.
    if (free()){
        _data[_head] = value;
        if (empty())
            _tail = _head;
        if (++_head >= Size)
            _head %= Size;
    }
}

template<typename T, size_t Size>
T Deque<T, Size>::pop_front() {
    if (empty()) // if empty, do nothing.
        return T();
    auto item = front();
    if (++_tail >= Size)
        _tail %= Size;
    if (_tail == _head)
        clear();
    return item;
}

template<typename T, size_t Size>
T Deque<T, Size>::pop_back() {
    if (empty()) // if empty, do nothing.
        return T();
    auto item = front();
    if (--_head < 0)
        _head = Size - 1;
    if (_head == _tail) //now buffer is empty
        clear();
    return item;
}

template<typename T, size_t Size>
void Deque<T, Size>::erase(size_t position) {
    if (position >= size()) // out-of-range!
        return; // do nothing.
    for (size_t i = position; i < size() - 1; i++){
        at(i) = at(i + 1);
    }
    pop_back();
}

template<typename T, size_t Size>
void Deque<T, Size>::erase(size_t first, size_t last) {
    if (first > last // invalid arguments
    || first >= size()) // out-of-range
        return; //do nothing.
    
    size_t tgt = first;
    for (size_t i = last + 1; i < size(); i++){
        at(tgt++) = at(i);
    }
    for (size_t i = first; i <= last; i++){
        pop_back();
    }
}

template<typename T, size_t Size>
T& Deque<T, Size>::operator[](size_t index)
{
    auto i = _tail + index;
    if (i >= Size)
        i %= Size;
    return _data[i];
}

template<typename T, size_t Size>
const T& Deque<T, Size>::operator[](size_t index) const
{
    auto i = _tail + index;
    if (i >= Size)
        i %= Size;
    return _data[i];
}

template<typename T, size_t Size>
T& Deque<T, Size>::at(size_t index)
{
    auto i = _tail + index;
    if (i >= Size)
        i %= Size;
    return _data[i];
}

template<typename T, size_t Size>
const T& Deque<T, Size>::at(size_t index) const
{
    auto i = _tail + index;
    if (i >= Size)
        i %= Size;
    return _data[i];
}

template<typename T, size_t Size>
void Deque<T, Size>::clear()
{
  _tail = -1;
  _head = 0;
}

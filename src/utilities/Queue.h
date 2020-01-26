#pragma once

template <typename DataType, size_t Size>
class Queue {
  public:
    Queue();
        
    inline size_t count();
    void push(const DataType &item);
    void push(const DataType *inData, size_t inSize);
    DataType peek();
    DataType pop(size_t inNumberOfItems = 1);
    void pop(DataType *outData, size_t inSize);
    void clear();

  private:
    size_t _index;
    DataType _data[Size];
};

#include "Queue.hpp"
